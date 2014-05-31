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
			int damage = g_knifeDamage.integer;
			int randomize = g_knifeDamageRand.integer;

			// Randomize damage a little if enabled..
			if (randomize) {
				if (randomize > 100)
					randomize = 100;
				else if (randomize < 1)
					randomize = 1;

				damage -= rand() % randomize;
			}

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

From BOTW/S4NDMoD 
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

	shoveAmount = 512 * g_shoveAmount.value;
	VectorMA(target->client->ps.velocity, shoveAmount, forward, target->client->ps.velocity);

	APRS(target, "sound/multiplayer/vo_revive.wav");
	ent->lastPushTime = level.time;	
}

/*
=================
Drop objective

Port from NQ
=================
*/
void Cmd_DropObj(gentity_t *self)
{
	gitem_t *item= NULL;

	// drop flag regardless
	if (self->client->ps.powerups[PW_REDFLAG]) {
		item = BG_FindItem("Red Flag");
		if (!item)
			item = BG_FindItem("Objective");

		self->client->ps.powerups[PW_REDFLAG] = 0;
	}
	if (self->client->ps.powerups[PW_BLUEFLAG]) {
		item = BG_FindItem("Blue Flag");
		if (!item)
			item = BG_FindItem("Objective");

		self->client->ps.powerups[PW_BLUEFLAG] = 0;
	}

	if (item) {
		vec3_t launchvel = { 0, 0, 0 };
		vec3_t forward;
		vec3_t origin;
		vec3_t angles;
		gentity_t *flag;

		VectorCopy(self->client->ps.origin, origin);
		// tjw: if the player hasn't died, then assume he's
		//      throwing objective per g_dropObj
		if(self->health > 0) {
			VectorCopy(self->client->ps.viewangles, angles);
			if(angles[PITCH] > 0)
				angles[PITCH] = 0;
			AngleVectors(angles, forward, NULL, NULL);
			VectorMA(self->client->ps.velocity,
				96, forward, launchvel);
			VectorMA(origin, 36, forward, origin);
			origin[2] += self->client->ps.viewheight;
		}

		flag = LaunchItem(item, origin, launchvel, self->s.number);

		flag->s.modelindex2 = self->s.otherEntityNum2;// JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
		flag->message = self->message;	// DHM - Nerve :: also restore item name
		// Clear out player's temp copies
		self->s.otherEntityNum2 = 0;
		self->message = NULL;
		self->droppedObj = qtrue;
	} 
	else
	{
		Cmd_ThrowKnives( self );
	}
}

/*
============
Cmd_Spy_f

Core idea is from ET but heavily relies on S4NDMoD's version
to avoid any (infamous) skin errors ..
============
*/
void limbo( gentity_t *ent, qboolean makeCorpse );

void Cmd_Spy( gentity_t *ent ) {
	int charge;
	char *label;
	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);	
	start[2] += ent->client->ps.viewheight;
	VectorMA (start, 96, forward, end);	
	
	trap_Trace (&tr, start, NULL, NULL, end, ent->s.number, CONTENTS_CORPSE);

	if (tr.entityNum >= MAX_CLIENTS)
		return;

	target = &(g_entities[tr.entityNum]);
	if ((!target->inuse) || (!target->client))
		return;

	if (target->client->ps.stats[STAT_HEALTH] > 0)	
		return;

	if (OnSameTeam(ent,target))
		return;

	if(ent->client->ps.stats[STAT_HEALTH] < 1)
		return;

	if(g_gamestate.integer != GS_PLAYING)
		return;

	if(ent->client->ps.powerups[PW_REDFLAG] || ent->client->ps.powerups[PW_BLUEFLAG])
		return;

    if(ent->client->ps.eFlags & EF_ZOOMING)
        return;
   
    if(ent->client->ps.weapon == WP_SNIPERRIFLE)
        return;

	if (ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC)
		charge = g_medicChargeTime.integer * 0.75;
	else if (ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT)
	    charge = g_LTChargeTime.integer * 0.75;
	else if (ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_SOLDIER)
		charge = g_soldierChargeTime.integer * 0.75;
	else
	   charge = g_engineerChargeTime.integer * 0.75;

	if ( level.time - ent->client->ps.classWeaponTime < charge )
	{
		CP("cp \"You must have at least 3/4 of a full charge^1!\n\"1");
		return;
	}

	label = (target->client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC ? "Medic" : 
		(target->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER ? "Engineer" :
			(target->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT ? "Lieutenant" : "Soldier") ) );

	CP(va("cp \"You are now disguised as a %s^3!\n\"2", label));

	ent->client->ps.classWeaponTime = level.time;
	ent->client->ps.isSpy = qtrue;
	ent->client->ps.spyType = target->client->ps.stats[STAT_PLAYER_CLASS];
	ClientUserinfoChanged(ent->client->ps.clientNum);

	limbo(target, qtrue);
}

/*
=========
Report spies
=========
*/
void checkSpies( gentity_t *ent ) {
	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;

	if (ent->client->ps.stats[STAT_HEALTH] <= 0)
		return;

	if(g_gamestate.integer != GS_PLAYING)
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);
	start[2] += ent->client->ps.viewheight;
	VectorMA (start, 512, forward, end);	

	trap_Trace (&tr, start, NULL, NULL, end, ent->s.number, (CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE|CONTENTS_TRIGGER));

	if ( tr.surfaceFlags & SURF_NOIMPACT )
		return;

	if ( tr.entityNum == ENTITYNUM_WORLD )
		return;

	if (tr.entityNum >= MAX_CLIENTS)
		return;

	target = &g_entities[ tr.entityNum ];

	if ((!target->inuse) || (!target->client))
		return;

	if (target->client->ps.stats[STAT_HEALTH] <= 0)	
		return;

	if ( !target->client->ps.isSpy )
		return;

	ent->client->infoTime = level.time;

	if (OnSameTeam(target, ent)) 	
		CP("cp \"^3YOUR TEAMMATE!\n\"1");
	else
		CP("cp \"^3ENEMY SPY!\n\"1");
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

/*
=================
Hitsounds

Do it like in shrub just permanently 
(A hack tied to color so one doesn't need to type it all the time..)
=================
*/
void Cmd_hitsounds(gentity_t *ent) {
	char *action = (ent->client->sess.clientFlags & CFLAGS_HITSOUNDS ? "^3Disable^7" : "^3Enable^7");
	int	flag = (ent->client->sess.clientFlags & CFLAGS_HITSOUNDS ? 0 : 1);

	CP(va("print \"Bit flag to %s Hitsounds is /color %d \nType ^3/commands bitflags^7 for explanation.\n\"", action, flag));
	return;	
}

/*
=================
GiveAmmo

Borrowed this from S4NDMoD o:)
=================
*/
void Cmd_GiveAmmo(gentity_t* ent) {
	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;
	char arg1[MAX_STRING_TOKENS];
	int givenAmmo;
	int weapon, targWeap;
	int maxGive; //the max ammount of ammo you can give this player(just in case a player gives too much ammo they will get it back)

	if (!g_allowGiveAmmo.integer) {
		CPx(ent - g_entities, va("cp \"^3/giveammo^7 is not enabled on this server!\"1"));
		return;
	}

	trap_Argv(1, arg1, sizeof(arg1));
	givenAmmo = atoi(arg1);

	if (ent->client->ps.stats[STAT_HEALTH] <= 0) {
		CPx(ent - g_entities, va("cp \"You have to be alive to do that!\"1"));
		return;
	}

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);   //set 'start' to the player's position (plus the viewheight)
	start[2] += ent->client->ps.viewheight;
	VectorMA(start, 128, forward, end);    //put 'end' 128 units forward of 'start'

	//see if we hit anything between 'start' and 'end'
	trap_Trace(&tr, start, NULL, NULL, end, ent->s.number, CONTENTS_BODY);

	//if we didn't hit a player, return
	if (tr.entityNum >= MAX_CLIENTS) {
		return;
	}

	target = &(g_entities[tr.entityNum]);

	if ((!target->inuse) || (!target->client)) { //if the player is lagged/disconnected/etc
		return;
	}
	if (target->client->sess.sessionTeam != ent->client->sess.sessionTeam) { //not on same team
		CPx(ent - g_entities, va("cp \"What are you a spy or something!\"1"));
		return;
	}

	if (target->client->ps.stats[STAT_HEALTH] <= 0) {   //if they're dead
		CPx(ent - g_entities, va("cp \"You can not give ammo to the dead!\"1"));
		return;
	}

	//if given ammo is 0 or unset make it the size of a clip
	if (givenAmmo <= 0) {
		givenAmmo = ammoTable[ent->client->ps.weapon].maxclip;
	}

	weapon = ent->client->ps.weapon;

	switch (weapon) {
	case WP_LUGER:
	case WP_COLT:

		if (ent->client->ps.ammo[BG_FindClipForWeapon(ent->client->ps.weapon)] < givenAmmo) {
			CPx(ent - g_entities, va("cp \"You do not have enough ammo to give ^7%i^3!\"2", givenAmmo));
			return;
		}

		if (!COM_BitCheck(target->client->ps.weapons, WP_LUGER) && !COM_BitCheck(target->client->ps.weapons, WP_COLT)) {
			CPx(ent - g_entities, va("cp \"%s ^7does not have a Pistol!\"2", target->client->pers.netname));
			return;
		}

		if (COM_BitCheck(target->client->ps.weapons, WP_LUGER)) {
			targWeap = WP_LUGER;
		}
		else {
			targWeap = WP_COLT;
		}

		maxGive = (ammoTable[targWeap].maxclip * 4) - target->client->ps.ammo[BG_FindAmmoForWeapon(targWeap)];
		target->client->ps.ammo[BG_FindAmmoForWeapon(targWeap)] += givenAmmo;
		if (target->client->ps.ammo[BG_FindAmmoForWeapon(targWeap)] > ammoTable[targWeap].maxclip * 4) {
			target->client->ps.ammo[BG_FindAmmoForWeapon(targWeap)] = ammoTable[targWeap].maxclip * 4;
		}

		break;

	case WP_MP40:
	case WP_THOMPSON:
	case WP_STEN:

		if (ent->client->ps.ammo[BG_FindClipForWeapon(ent->client->ps.weapon)] < givenAmmo) {
			CPx(ent - g_entities, va("cp \"You do not have enough ammo to give ^7%i^7!\"2", givenAmmo));
			return;
		}

		if (!COM_BitCheck(target->client->ps.weapons, WP_MP40) && !COM_BitCheck(target->client->ps.weapons, WP_THOMPSON) && !COM_BitCheck(target->client->ps.weapons, WP_STEN)) {
			CPx(ent - g_entities, va("cp \"%s ^7does not have a SMG!\"2", target->client->pers.netname));
			return;
		}

		if (COM_BitCheck(target->client->ps.weapons, WP_MP40)) {
			targWeap = WP_MP40;
		}
		else if (COM_BitCheck(target->client->ps.weapons, WP_THOMPSON)) {
			targWeap = WP_THOMPSON;
		}
		else {
			targWeap = WP_STEN;
		}

		maxGive = (ammoTable[targWeap].maxclip * 3) - target->client->ps.ammo[BG_FindAmmoForWeapon(targWeap)];
		target->client->ps.ammo[BG_FindAmmoForWeapon(targWeap)] += givenAmmo;
		if (target->client->ps.ammo[BG_FindAmmoForWeapon(targWeap)] > ammoTable[targWeap].maxclip * 3) {
			target->client->ps.ammo[BG_FindAmmoForWeapon(targWeap)] = ammoTable[targWeap].maxclip * 3;
		}

		break;

	case WP_MAUSER:
	case WP_SNIPERRIFLE:

		if (ent->client->ps.ammo[BG_FindClipForWeapon(ent->client->ps.weapon)] < givenAmmo) {
			CPx(ent - g_entities, va("cp \"You do not have enough ammo to give ^7%i^7!\"2", givenAmmo));
			return;
		}

		if (!COM_BitCheck(target->client->ps.weapons, WP_SNIPERRIFLE) && !COM_BitCheck(target->client->ps.weapons, WP_MAUSER)) {
			CPx(ent - g_entities, va("cp \"%s ^7does not have a Mauser!\"2", target->client->pers.netname));
			return;
		}
		
		maxGive = (ammoTable[WP_SNIPERRIFLE].maxclip * 3) - target->client->ps.ammo[BG_FindAmmoForWeapon(WP_SNIPERRIFLE)];
		target->client->ps.ammo[BG_FindAmmoForWeapon(WP_SNIPERRIFLE)] += givenAmmo;
		if (target->client->ps.ammo[BG_FindAmmoForWeapon(WP_SNIPERRIFLE)] > ammoTable[WP_SNIPERRIFLE].maxclip * 3) {
			target->client->ps.ammo[BG_FindAmmoForWeapon(WP_SNIPERRIFLE)] = ammoTable[WP_SNIPERRIFLE].maxclip * 3;
		}

		maxGive = (ammoTable[WP_MAUSER].maxclip * 3) - target->client->ps.ammo[BG_FindAmmoForWeapon(WP_MAUSER)];
		target->client->ps.ammo[BG_FindAmmoForWeapon(WP_MAUSER)] += givenAmmo;
		if (target->client->ps.ammo[BG_FindAmmoForWeapon(WP_MAUSER)] > ammoTable[WP_MAUSER].maxclip * 3) {
			target->client->ps.ammo[BG_FindAmmoForWeapon(WP_MAUSER)] = ammoTable[WP_MAUSER].maxclip * 3;
		}
		break;


	case WP_PANZERFAUST:

		if (ent->client->ps.ammo[BG_FindClipForWeapon(ent->client->ps.weapon)] < givenAmmo) {
			CPx(ent - g_entities, va("cp \"You do not have enough ammo to give ^7%i^7!\"2", givenAmmo));
			return;
		}

		if (!COM_BitCheck(target->client->ps.weapons, WP_PANZERFAUST)) {
			CPx(ent - g_entities, va("cp \"%s ^7does not have a Panzerfaust!\"2", target->client->pers.netname));
			return;
		}
		maxGive = (ammoTable[WP_PANZERFAUST].maxclip * 3) - target->client->ps.ammo[BG_FindAmmoForWeapon(WP_PANZERFAUST)];
		target->client->ps.ammo[BG_FindAmmoForWeapon(WP_PANZERFAUST)] += givenAmmo;
		if (target->client->ps.ammo[BG_FindAmmoForWeapon(WP_PANZERFAUST)] > ammoTable[WP_PANZERFAUST].maxclip * 3) {
			target->client->ps.ammo[BG_FindAmmoForWeapon(WP_PANZERFAUST)] = ammoTable[WP_PANZERFAUST].maxclip * 3;
		}

		break;


	case WP_VENOM:

		if (ent->client->ps.ammo[BG_FindClipForWeapon(ent->client->ps.weapon)] < givenAmmo) {
			CPx(ent - g_entities, va("cp \"You do not have enough ammo to give ^7%i^7!\"2", givenAmmo));
			return;
		}

		if (!COM_BitCheck(target->client->ps.weapons, WP_VENOM)) {
			CPx(ent - g_entities, va("cp \"^7%s ^7does not have a Venom!\"2", target->client->pers.netname));
			return;
		}
		maxGive = (ammoTable[WP_VENOM].maxclip * 3) - target->client->ps.ammo[BG_FindAmmoForWeapon(WP_VENOM)];
		target->client->ps.ammo[BG_FindAmmoForWeapon(WP_VENOM)] += givenAmmo;
		if (target->client->ps.ammo[BG_FindAmmoForWeapon(WP_VENOM)] > ammoTable[WP_VENOM].maxclip * 3) {
			target->client->ps.ammo[BG_FindAmmoForWeapon(WP_VENOM)] = ammoTable[WP_VENOM].maxclip * 3;
		}
		break;

	default:
		CPx(ent - g_entities, va("cp \"You can not use ^3/giveammo^7 with this weapon!\"2"));
		return;
	}

	if (maxGive <= 0) {
		CPx(ent - g_entities, va("cp \"%s ^7does not need any ammo!\"3", target->client->pers.netname));
		return;
	}

	if (givenAmmo > maxGive) {
		CPx(ent - g_entities, va("cp \"You gave ^3%s^7 ammo, ^3%i ^7was all he could carry!\"3", target->client->pers.netname, maxGive));
		CPx(target - g_entities, va("cp \"%s^7 gave you ^3%i ^7ammo!\"3", ent->client->pers.netname, maxGive));
		ent->client->ps.ammo[BG_FindClipForWeapon(weapon)] -= maxGive;

	}
	else {

		CPx(ent - g_entities, va("cp \"You gave ^3%i^7 ammo to %s^7!\"3", givenAmmo, target->client->pers.netname));
		CPx(target - g_entities, va("cp \"%s^7 gave you ^3%i ^7ammo!\"3", ent->client->pers.netname, givenAmmo));
		ent->client->ps.ammo[BG_FindClipForWeapon(weapon)] -= givenAmmo;
	}
}

/*
=================
User help..

Spam fest^^
=================
*/
void Cmd_help(gentity_t *ent) {
	char		about[MAX_STRING_TOKENS];	

	trap_Argv(1, about, sizeof(about));

	// About
	if (!Q_stricmp(about, "about")) {
		CP("print \"^7\n\"");
		CP("print \"^3About:\n\"");
		CP("print \"^7-------------------------------------------\n\"");
		CP(va("print \"^3Mod:           ^7%s\n\"", GAMEVERSION));
		CP("print \"^3Developer:     ^7Nate 'L0\n\"");
		CP("print \"^3Forums:        ^7http://rtcwx.com\n\"");
		CP("print \"^3Contact:       ^7nate.afk@gmail.com\n\"");
		CP("print \"^3Source code:   ^7Available per request\n\"");
		CP("print \"^3Bugs/Ideas:    ^7Use info provided above.\n\"");
		CP("print \"^3Show Support:  ^7Donate on forums..\n\"");
		CP("print \"^7-------------------------------------------\n\n\"");
	}
	// Bit flags
	else if (!Q_stricmp(about, "bitflags")) {
		CP("print \"^7\n\"");
		CP("print \"^3Bit Flags:\n\"");		
		CP("print \"^7-----------------------------------------\n\"");
		CP("print \"^7Hitsounds ^3>> 1\n\"");
		CP("print \"^7MP40      ^3>> 2\n\"");
		CP("print \"^7Thompson  ^3>> 8\n\"");
		CP("print \"^7Sten      ^3>> 16\n\"");
		CP("print \"^7\n\"");
		CP("print \"^3Example\n\"");
		CP("print \"^7/color 3 = Hitsounds On + Spawn with mp40\n\"");
		CP("print \"^7-----------------------------------------\n\"");
		CP("print \"^3Usage^7: /color <num>\n\n\"");
	}
	// Commands
	else if (!Q_stricmp(about, "list")) {
		CP("print \"^7\n\"");
		CP("print \"^3Available Commands:\n\"");
		CP("print \"^7-----------------------------------------------\n\"");
		CP("print \"^7/getstatus         ^3>> Player's list\n\"");
		CP("print \"^7/hitsound          ^3>> Hitsounds on/off\n\"");
		CP("print \"^7/login <password>  ^3>> Logs you as Admin\n\"");
		CP("print \"^7/mp40              ^3>> Spawn with MP40\n\"");
		CP("print \"^7/pm <target> <msg> ^3>> Sends private message\n\"");
		CP("print \"^7/private <msg>     ^3>> Private Admin chat\n\"");
		CP("print \"^7/smoke             ^3>> Throw smoke (LT only)\n\"");
		CP("print \"^7/stats             ^3>> Prints your current stats\n\"");
		CP("print \"^7/sten              ^3>> Spawn with Sten\n\"");
		CP("print \"^7/sui               ^3>> Suicide without a gib\n\"");
		CP("print \"^7/thompson          ^3>> Spawn with Thompson\n\"");
		CP("print \"^7/time              ^3>> Current server time\n\"");
		CP("print \"^7-----------------------------------------------\n\n\"");
		CP("print \"^3Usage^7: /<command>\n\"");
	}
	// Nothing specific..list all categories
	else {
		CP("print \"^7\n\"");
		CP("print \"^3Available Categories:\n\"");
		CP("print \"^7--------------------\n\"");
		CP("print \"^3- about\n\"");
		CP("print \"^3- bitflags\n\"");
		CP("print \"^3- list\n\"");
		CP("print \"^7--------------------\n\"");
		CP("print \"^3Usage^7: /commands <category>\n\n\"");
	}

}


