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
void Cmd_ThrowKnives( gentity_t *ent ) {
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
void Cmd_Pmsg( gentity_t *ent )
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

/*
================
Shows time
================
*/
extern int trap_RealTime ( qtime_t * qtime );
const char *aMonths[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void Cmd_Time_f( gentity_t *ent ) {
	qtime_t ct;
	trap_RealTime(&ct);

	CP(va("chat \"%s^7 current time is: %02d:%02d:%02d ^3(^7%02d %s %d^3) \n\"", 
		ent->client->pers.netname , ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, aMonths[ct.tm_mon], 1900+ct.tm_year ) );
	CPS(ent, "sound/multiplayer/dynamite_01.wav");
}

/*
===================
Drag players 

Came from BOTW/S4NDMoD 
===================
*/
void Cmd_Drag( gentity_t *ent) {
	gentity_t *target;
	vec3_t start,dir,end;
	trace_t tr;
	target = NULL;

	if (!g_dragBodies.integer)
		return;

	if (level.time < (ent->lastDragTime + 20))		
		return;

	if (ent->client->ps.stats[STAT_HEALTH] <= 0)	
		return;

	AngleVectors(ent->client->ps.viewangles, dir, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);	
	start[2] += ent->client->ps.viewheight;
	VectorMA (start, 100, dir, end);


	trap_Trace (&tr, start, NULL, NULL, end, ent->s.number, CONTENTS_CORPSE);

	if (tr.entityNum >= MAX_CLIENTS)
		return;

	target = &(g_entities[tr.entityNum]);

       	if ((!target->inuse) || (!target->client))
		return;		

		VectorCopy(target->r.currentOrigin, start); 
		VectorCopy(ent->r.currentOrigin, end); 
		VectorSubtract(end, start, dir); 
		VectorNormalize(dir); 
		VectorScale(dir,100, target->client->ps.velocity);
		VectorCopy(dir, target->movedir); 
       
		ent->lastDragTime = level.time;		
}

/*
=================
L0 - Shove players away

Ported from BOTW source.
=================
*/
void Cmd_Push(gentity_t* ent)
{
	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;
	float shoveAmount;

	if (!g_shove.integer)	
		return;

	if (ent->client->ps.stats[STAT_HEALTH] <= 0)	
		return;

	if (level.time < (ent->lastPushTime + 600))		
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);
	start[2] += ent->client->ps.viewheight;
	VectorMA (start, 128, forward, end);	

	trap_Trace (&tr, start, NULL, NULL, end, ent->s.number, CONTENTS_BODY);

	if (tr.entityNum >= MAX_CLIENTS)
		return;

	target = &(g_entities[tr.entityNum]);

	if ((!target->inuse) || (!target->client))	
		return;

	if (target->client->ps.stats[STAT_HEALTH] <= 0)	
		return;

	shoveAmount = 512 * .8;
	VectorMA(target->client->ps.velocity, shoveAmount, forward, target->client->ps.velocity);

	APRS(target, "sound/multiplayer/vo_revive.wav");

	ent->lastPushTime = level.time;	
}

/*
=================
Stats command

TODO: Unlazy my self and add targeted stats (victim, killer, <client number>)
=================
*/
void Cmd_Stats_f(gentity_t *ent) {
	gclient_t *client = ent->client;
	qtime_t ct;
	int eff;
	int deaths = client->pers.deaths;	
	float killRatio = client->pers.kills;
	int shots = client->pers.acc_shots;
	float acc = 0.0f;

	if (deaths > 0)
		killRatio = (float)client->pers.kills / (float)deaths;
	
	if (shots > 0)
		acc = ((float)client->pers.acc_hits / (float)shots) * 100.0f;

	eff = ( client->pers.deaths + client->pers.kills == 0 ) ? 0 : 100 * client->pers.kills / ( client->pers.deaths + client->pers.kills );
	if ( eff < 0 ) {
		eff = 0;
	}	

	trap_RealTime(&ct);

	CP(va("print \"\n"
			"^7Mod: %s \n"
			"^7Server: %s \n"
			"^7Time: ^7%02d:%02d:%02d ^3(^7%02d %s %d^3) \n\n"
			"^7Stats for %s ^7this round: \n"
			"^7-------------------------------------------\n"			
			"^3Kills: ^7%d | ^3TKs: ^7%d | ^3Poisoned: ^7%d\n"			
			"^3Deaths: ^7%d | ^3Suicides: ^7%d\n"			
			"^3Headshots: ^7%d | ^3Gibs: ^7%d | ^3Revived: ^7%d\n"			
			"^3Packs Given: ^7%d ^3Health ^7| %d ^3Ammo \n"
			"^3Accuracy: ^7%2.2f (%d/%d)\n"
			"^3DmgGiv: ^7%d ^| ^3DmgRec: ^7%d | ^3dmgTeam: ^7%d \n"			
			"^3Kill Ratio: ^7%2.2f | ^3Efficency: ^7%d\n"
			"^3Peak: ^7%d ^3Life Kills ^7| %d ^3Death Spree \n"
			"^7-------------------------------------------\n"
			"\n\"",
		GAMEVERSION,
		sv_hostname.string,
		ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, aMonths[ct.tm_mon], 1900+ct.tm_year,
		client->pers.netname,
		client->pers.kills, client->pers.teamKills,	client->pers.poison,
		deaths,	client->pers.suicides,
		client->pers.headshots,	client->pers.gibs, client->pers.revives,
		client->pers.medPacks, client->pers.ammoPacks,
		acc, client->pers.acc_hits, shots,
		client->pers.dmgGiven, client->pers.dmgReceived, client->pers.dmgTeam,
		killRatio, eff,
		client->pers.lifeKillsPeak, client->pers.lifeDeathsPeak
	));
}

