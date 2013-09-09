/*
===========================================================================
g_players.c

Player's commands and features.

Author: Nate 'L0
Created: 8.Sept/13
Updated:
===========================================================================
*/
#include "g_local.h"

/*
================
Throw knives

Originally BOTW (tho took it from s4ndmod as it's faster to implement..).
================
*/
// Knife "think" function                                                                                                                                                                                                                                                                                               ////////////////////////////////////////////
void Touch_Knife( gentity_t *ent, gentity_t *other, trace_t *trace ) {
	qboolean hurt = qfalse;
	ent->active = qfalse;  

	if ( !other->client ) {
		return;
	}
	if ( other->health < 1 ) {
		return;  
	}

	if ( VectorLength( ent->s.pos.trDelta ) != 0 ) { 		
		if ( ( g_friendlyFire.integer ) || ( !OnSameTeam( other, ent->parent ) ) ) {
			int i;
			int sound;
			int damage = 20;     
			damage -= rand() % 10;

			if ( damage <= 0 ) {
				damage = 1;
			}

			// pick a random sound to play
			i = rand() % 3;
			if ( i == 0 ) {
				sound = G_SoundIndex( "/sound/weapons/knife/knife_hit1.wav" );
			} else if ( i == 1 ) {
				sound = G_SoundIndex( "/sound/weapons/knife/knife_hit2.wav" );
			} else {
				sound = G_SoundIndex( "/sound/weapons/knife/knife_hit3.wav" );
			}

			G_Sound( other, sound );  
			G_Damage( other, ent->parent, ent->parent, NULL, trace->endpos, damage, 0, MOD_THROWKNIFE ); 
			hurt = qtrue;
		}
	}

	if ( hurt == qfalse ) { 
		int makenoise = EV_ITEM_PICKUP;

		if ( g_throwKnives.integer > 0 ) {
			other->client->pers.throwingKnives++; 
		}		
		if ( g_throwKnives.integer != 0 ) { 
			if ( other->client->pers.throwingKnives > (g_throwKnives.integer + 5) ) {
				other->client->pers.throwingKnives = (g_throwKnives.integer + 5);
			}
		}		
		if ( ent->noise_index ) {
			makenoise = EV_ITEM_PICKUP_QUIET;
			G_AddEvent( other, EV_GENERAL_SOUND, ent->noise_index );
		}		
		if ( other->client->pers.predictItemPickup ) {
			G_AddPredictableEvent( other, makenoise, ent->s.modelindex );
		} else {
			G_AddEvent( other, makenoise, ent->s.modelindex );
		}
	}

	ent->freeAfterEvent = qtrue; 
	ent->flags |= FL_NODRAW;
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->r.contents = 0;
	ent->nextthink = 0;
	ent->think = 0;
}
// Actual command
void cmd_throwKnives( gentity_t *ent ) {
	vec3_t velocity, angles, offset, org, mins, maxs;
	trace_t tr;
	gentity_t *ent2;
	gitem_t *item = BG_FindItemForWeapon( WP_KNIFE );

	if ( g_throwKnives.integer == 0 ) {
		return;
	}

	if ( level.time < ( ent->thrownKnifeTime + 800 ) ) {   
		return;
	}

	// If out or -1/unlimited
	if ( ( ent->client->pers.throwingKnives == 0 ) && 
		 ( g_throwKnives.integer != -1 ) ) {
	return;
	}
	
	AngleVectors( ent->client->ps.viewangles, velocity, NULL, NULL );
	VectorScale( velocity, 64, offset );
	offset[2] += ent->client->ps.viewheight / 2;
	VectorScale( velocity, 800, velocity );      
	velocity[2] += 50 + random() * 35;
	VectorAdd( ent->client->ps.origin, offset, org );
	VectorSet( mins, -ITEM_RADIUS, -ITEM_RADIUS, 0 );
	VectorSet( maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS );  
	trap_Trace( &tr, ent->client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID );
	VectorCopy( tr.endpos, org );

	G_Sound( ent, G_SoundIndex( "sound/weapons/knife/knife_slash1.wav" ) );  	
	ent2 = LaunchItem( item, org, velocity, ent->client->ps.clientNum ); 
	VectorCopy( ent->client->ps.viewangles, angles );
	angles[1] += 90;
	G_SetAngle( ent2, angles );
	ent2->touch = Touch_Knife;  
	ent2->parent = ent; 

	if ( g_throwKnives.integer > 0 ) {
		ent->client->pers.throwingKnives--; 
	}

	//only show the message if throwing knives are enabled
	if ( g_throwKnives.integer > 0 ) {		
		CP(va( "chat \"^3Knives left:^7 %d\" %i", ent->client->pers.throwingKnives, qfalse ));
	}

	ent->thrownKnifeTime = level.time;  
}

/*
================
Smoke
================
*/
void Cmd_Smoke_f( gentity_t *ent )
{
	gclient_t *client = ent->client;
	char message[64] = "Smoke grenade ";

	if (!g_smokeGrenades.integer)	
	{
		trap_SendServerCommand(ent-g_entities, va("print \"Smoke grenades are disabled on this server.\n\""));
		return;
	}

	if ((g_smokeGrenadesLmt.integer) && (ent->thrownSmoke >= g_smokeGrenadesLmt.integer))
	{
		trap_SendServerCommand(ent-g_entities, va("cp \"You have used all the Smoke supplies^3!\n\" 1"));
		return;
	}

	client->ps.selectedSmoke = !client->ps.selectedSmoke;
	strcat(message, va("%s", (client->ps.selectedSmoke ? "^2enabled" : "^1disabled")));

	trap_SendServerCommand(ent-g_entities, va("cp \"%s^7!\n\" 1", message));
}

// think function
extern void G_ExplodeMissile(gentity_t *ent);	
void weapon_smokeGrenade(gentity_t *ent)
{
	ent->s.eFlags |= EF_SMOKINGBLACK;		
	ent->s.loopSound = G_SoundIndex("sound/world/steam.wav");
	ent->nextthink = level.time + ((g_smokeGrenades.integer - 1) * 1000);	
	ent->think = G_ExplodeMissile;	
}

/*
================
Private messages
================
*/
void cmd_pmsg( gentity_t *ent )
{
	char cmd[MAX_TOKEN_CHARS];	
	char name[MAX_STRING_CHARS];
	char nameList[MAX_STRING_CHARS];
	char *msg;
	int matchList[MAX_CLIENTS];
	int count, i;

	if (!g_allowPMs.integer)	{	
		CP("print \"Private messages are Disabled^1!\n\"");
		return;
	}

	if (trap_Argc() < 3) {
		trap_Argv(0, cmd, sizeof(cmd));			
		CP(va("print \"^3Usage:^7  %s <match> <message>\n\"", cmd));
	return;
	}

	if (ent->client->sess.ignored) {
		if (ent->client->sess.ignored == 1)
			CP( "cp \"You are ignored^1!\n\"2" );
		else
			CP( "print \"You are ^3permanently ^7ignored^1!\n\"" );
	return;
	}

	trap_Argv(1, name, sizeof(name));
	if (strlen(name) < 2) {		
		CP("print \"You must match at least ^32 ^7characters of the name^3!\n\"");
	return;
	}

	count = ClientNumberFromNameMatch(name, matchList);
	if (count == 0) {		
		CP("print \"No matching clients found^3!\n\"");
	return;
	}

	msg = ConcatArgs(2);	   
    if( strlen(msg) >= 700 ){
		G_LogPrintf( "NUKER(pmsg >= 700): %s IP: %i.%i.%i.%i\n", ent->client->pers.netname, ent->client->sess.ip[0], ent->client->sess.ip[1], ent->client->sess.ip[2], ent->client->sess.ip[3] );
	    trap_DropClient( ent-g_entities, "^7Player Kicked: ^3Nuking" );
	return;
    }

	Q_strncpyz ( nameList, "", sizeof( nameList ) );
	for (i = 0; i < count; i++)	{
		strcat(nameList, g_entities[matchList[i]].client->pers.netname);
		if (i != (count-1))
			strcat(nameList, "^7, ");	
			
		// Notify user in chat 
		CPx(matchList[i], va("chat \"^3Private Message from ^7%s^7!\n\"", ent->client->pers.netname));

		// Print full message in console..
		CPx(matchList[i], va("print \"^3Private Message from ^7%s^7: \n^3Message: ^7%.99s\n\"", ent->client->pers.netname, msg));		

		// Send sound to them  as well (keep an eye on this!)
		CPS( g_entities-matchList[i], "xmod/sound/client/pm.wav");
	}

	//let the sender know his message went to
	CP(va("print \"^3Message was sent to: ^7%s \n^3Message: ^7%.99s\n\"", nameList, msg));
}