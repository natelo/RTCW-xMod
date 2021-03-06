
#include "g_local.h"

#include "ai_cast_fight.h"	// need these for avoidance


extern void G_CheckForCursorHints(gentity_t *ent);



/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
	gclient_t	*client;
	float	count;
	vec3_t	angles;

	client = player->client;
	if ( client->ps.pm_type == PM_DEAD ) {
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) {
		return;		// didn't take any damage
	}

	if ( count > 127 ) {
		count = 127;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) {
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	} else {
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an apropriate pain sound
	if ( (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && !(player->r.svFlags & SVF_CASTAI) && !(player->s.powerups & PW_INVULNERABLE) ) {	//----(SA)	
		player->pain_debounce_time = level.time + 700;
		G_AddEvent( player, EV_PAIN, player->health );
	}

	client->ps.damageEvent++;	// Ridah, always increment this since we do multiple view damage anims

	client->ps.damageCount = count;

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}


#define MIN_BURN_INTERVAL 399 // JPW NERVE set burn timeinterval so we can do more precise damage (was 199 old model)

/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
	qboolean	envirosuit;
	int			waterlevel;
	// TTimo: unused
//	static int	lastflameburntime = 0; // JPW NERVE for slowing flamethrower burn intervals and doing more damage per interval

	if ( ent->client->noclip ) {
		ent->client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;

	envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;

//	G_Printf("breathe: %d   invuln: %d   nofatigue: %d\n", ent->client->ps.powerups[PW_BREATHER], level.time - ent->client->ps.powerups[PW_INVULNERABLE], ent->client->ps.powerups[PW_NOFATIGUE]);

	//
	// check for drowning
	//
	if ( waterlevel == 3 ) {
		// envirosuit give air
		if ( envirosuit ) {
			ent->client->airOutTime = level.time + 10000;
		}

		//----(SA)	both these will end up being by virtue of having the 'breather' powerup
		if(ent->client->ps.aiChar == AICHAR_FROGMAN) {	// let frogmen breathe forever
			ent->client->airOutTime = level.time + 10000;
		}

		if(ent->client->ps.aiChar == AICHAR_SEALOPER) {	// ditto
			ent->client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if ( ent->client->airOutTime < level.time) {

			if(ent->client->ps.powerups[PW_BREATHER]) {	// take air from the breather now that we need it
				ent->client->ps.powerups[PW_BREATHER] -= (level.time - ent->client->airOutTime);
				ent->client->airOutTime = level.time + (level.time - ent->client->airOutTime);
			}
			else {


				// drown!
				ent->client->airOutTime += 1000;
				if ( ent->health > 0 ) {
					// take more damage the longer underwater
					ent->damage += 2;
					if (ent->damage > 15)
						ent->damage = 15;

					// play a gurp sound instead of a normal pain sound
					if (ent->health <= ent->damage) {
						G_Sound(ent, G_SoundIndex("*drown.wav"));
					} else if (rand()&1) {
						G_Sound(ent, G_SoundIndex("sound/player/gurp1.wav"));
					} else {
						G_Sound(ent, G_SoundIndex("sound/player/gurp2.wav"));
					}

					// don't play a normal pain sound
					ent->pain_debounce_time = level.time + 200;

					G_Damage (ent, NULL, NULL, NULL, NULL, 
						ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
				}
			}
		}
	} else {
		ent->client->airOutTime = level.time + 12000;
		ent->damage = 2;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if (waterlevel && (ent->watertype&CONTENTS_LAVA) ) {
		if (ent->health > 0	&& ent->pain_debounce_time <= level.time ) {

			if ( envirosuit ) {
				G_AddEvent( ent, EV_POWERUP_BATTLESUIT, 0 );
			} else {
				if (ent->watertype & CONTENTS_LAVA) {
					G_Damage (ent, NULL, NULL, NULL, NULL, 
						30*waterlevel, 0, MOD_LAVA);
				}
			}

		}
	}

	//
	// check for burning from flamethrower
	//
	// JPW NERVE MP way
	// L0 - Flamer bug fix
	if (ent->s.onFireEnd && ent->client) {	
//Martin 6/28/08 REDONE
		if ((level.time - ent->client->lastBurnTime >= 1000) && (ent->health > 0)) {
			ent->client->lastBurnTime = level.time;
			if ((ent->s.onFireEnd > level.time) && (ent->health > 0)) {
				gentity_t *attacker;
   				attacker = g_entities + ent->flameBurnEnt;
				G_Damage (ent, attacker->parent, attacker->parent, NULL, NULL, 5, DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER); // JPW NERVE was 7
			}
		// G_burnMeGood isnt called if they are dead so do the work here.
		} else if((level.time - ent->client->lastBurnTime >= 10) && (ent->health <= 0)) {
			ent->client->lastBurnTime = level.time;
			if ((ent->s.onFireEnd > level.time) && (ent->health <= 0)) {
				gentity_t *attacker;
   				attacker = g_entities + ent->flameBurnEnt;
				G_Damage (ent, attacker->parent, attacker->parent, NULL, NULL, 5, DAMAGE_NO_KNOCKBACK, MOD_FLAMETHROWER);
			}
		}
	}	// Flamer end
	
	// L0 - poison
	if (ent->client->ps.eFlags & EF_POISONED && ent->client && level.match_pause == PAUSE_NONE)
	{
		// Check if person is under spawn protection before any damange can be done
		if (ent->client->ps.powerups[PW_INVULNERABLE]) {
			// if he's under protecion reset poison or it kicks in once protection expires =X
			ent->client->ps.eFlags &= ~EF_POISONED;
			return;
		}

		//if the guy isn't dead and it's been a second since the last time we did this
		if ((level.time >= (ent->lastPoisonTime + 1000)) && (ent->health > 0))
		{
			int n = rand() % 3;	//will be 0, 1 or 2
			gentity_t *attacker = g_entities + ent->poisonEnt;	//the person who poisoned him			

			//play a random pain sound to near by players
			if (n == 0)
				G_Sound(ent, G_SoundIndex("sound/beast/gasp1.wav"));
			else if (n == 1)
				G_Sound(ent, G_SoundIndex("sound/player/bj2/death2.wav"));
			else if (n == 2)
				G_Sound(ent, G_SoundIndex("sound/player/bj2/death3.wav"));

			G_Damage(ent, attacker, attacker, NULL, NULL, 20, 0, MOD_POISONED);	//hurt him!
			ent->lastPoisonTime = level.time; //keep track of this so we can time it
		}
	} // L0 - end
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
	if (ent->aiCharacter)
		return;

	if (ent->waterlevel && (ent->watertype&CONTENTS_LAVA) )	//----(SA)	modified since slime is no longer deadly
		ent->s.loopSound = level.snd_fry;
	else
		ent->s.loopSound = 0;
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
	int		i, j;
	trace_t	trace;
	gentity_t	*other;

	memset( &trace, 0, sizeof( trace ) );
	for (i=0 ; i<pm->numtouch ; i++) {
		for (j=0 ; j<i ; j++) {
			if (pm->touchents[j] == pm->touchents[i] ) {
				break;
			}
		}
		if (j != i) {
			continue;	// duplicated
		}
		other = &g_entities[ pm->touchents[i] ];

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, other, &trace );
		}

		if ( !other->touch ) {
			continue;
		}

		other->touch( other, ent, &trace );
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	trace_t		trace;
	vec3_t		mins, maxs;
	static vec3_t	range = { 40, 40, 52 };

	if ( !ent->client ) {
		return;
	}

	// dead clients don't activate triggers!
	if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	VectorSubtract( ent->client->ps.origin, range, mins );
	VectorAdd( ent->client->ps.origin, range, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
	VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];

		if ( !hit->touch && !ent->touch ) {
			continue;
		}
		if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
			continue;
		}

		// ignore most entities if a spectator
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			if ( hit->s.eType != ET_TELEPORT_TRIGGER ) {
				continue;
			}
		}

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if ( hit->s.eType == ET_ITEM ) {
			if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
				continue;
			}
		} else {
			// MrE: always use capsule for player
			//if ( !trap_EntityContactCapsule( mins, maxs, hit ) ) {
			if ( !trap_EntityContact( mins, maxs, hit ) ) {
				continue;
			}
		}

		memset( &trace, 0, sizeof(trace) );

		if ( hit->touch ) {
			hit->touch (hit, ent, &trace);
		}

		if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
			ent->touch( ent, hit, &trace );
		}
	}
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
	pmove_t	pm;
	gclient_t	*client;

	client = ent->client;

	if ( client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		client->ps.pm_type = PM_SPECTATOR;
		client->ps.speed = 400;	// faster than normal
		if (client->ps.sprintExertTime)
			client->ps.speed *= 3;	// (SA) allow sprint in free-cam mode

		// OSPx - Pause
		if (level.match_pause != PAUSE_NONE) {
			client->ps.pm_type = PM_FREEZE;
			ucmd->buttons = 0;
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->upmove = 0;
			ucmd->wbuttons = 0;
		}

		// set up for pmove
		memset (&pm, 0, sizeof(pm));
		pm.ps = &client->ps;
		pm.pmext = &client->pmext;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = trap_Trace;
		pm.pointcontents = trap_PointContents;

		Pmove(&pm); // JPW NERVE

		// Rafael - Activate
		// Ridah, made it a latched event (occurs on keydown only)
		if (client->latched_buttons & BUTTON_ACTIVATE)
		{
			Cmd_Activate_f (ent);
		}

		// save results of pmove
		VectorCopy( client->ps.origin, ent->s.origin );

		G_TouchTriggers( ent );
		trap_UnlinkEntity( ent );
	}

	if (ent->flags & FL_NOFATIGUE)
		ent->client->ps.sprintTime = 20000;


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = ucmd->wbuttons;

	// attack button cycles through spectators
	if ((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK)) {
		Cmd_FollowCycle_f(ent, 1);
	}
	else if (
		(client->sess.sessionTeam == TEAM_SPECTATOR) && // don't let dead team players do free fly
		(client->sess.spectatorState == SPECTATOR_FOLLOW) &&
		(client->buttons & BUTTON_ACTIVATE) &&
		!(client->oldbuttons & BUTTON_ACTIVATE) &&
		G_allowFollow(ent, TEAM_RED) && G_allowFollow(ent, TEAM_BLUE)) { // OSPx - Speclock
		// code moved to StopFollowing
		StopFollowing(ent);
	}
}



/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped

L0 - Patched for g_spectatorInactivity and added ability to move
     clients to spectators rather than kick them..
NOTE: Dead players aren't accounted any more either..
NOTE 2: Spec's that spectate don't get kicked as they may be making a demo.
      - This particular behaviour is controlled by g_spectatorAllowDemo cvar.
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {

	if ((g_inactivity.integer <= 0 &&
			(client->sess.sessionTeam == TEAM_RED || client->sess.sessionTeam == TEAM_BLUE)) ||
		(g_spectatorInactivity.integer <= 0 && client->sess.sessionTeam == TEAM_SPECTATOR))
	{
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime = level.time + 60 * 1000;
		client->inactivityWarning = qfalse;

		return qtrue;
	} 
	else if ( client->pers.cmd.forwardmove || 
		client->pers.cmd.rightmove || 
		client->pers.cmd.upmove ||
		(client->pers.cmd.wbuttons & WBUTTON_ATTACK2) ||
		(client->pers.cmd.buttons & BUTTON_ATTACK) ||
		(client->pers.cmd.wbuttons & WBUTTON_LEANLEFT) ||
		(client->pers.cmd.wbuttons & WBUTTON_LEANRIGHT) ||
		client->ps.pm_type == PM_DEAD) 
	{
		client->inactivityTime = level.time +
			((client->sess.sessionTeam == TEAM_RED || client->sess.sessionTeam == TEAM_BLUE) ?
				g_inactivity.integer : g_spectatorInactivity.integer) * 1000;
		client->inactivityWarning = qfalse;

		return qtrue;
	} 
	else if ( !client->pers.localClient ) 
	{
		if ( level.time > client->inactivityTime && client->inactivityWarning) 
		{	
			// Playing client
			if ((client->sess.sessionTeam == TEAM_RED || client->sess.sessionTeam == TEAM_BLUE) 
				&& client->inactivityWarning)
			{
				if (g_inactivityToSpecs.integer ) 
				{				
					client->sess.sessionTeam = TEAM_SPECTATOR;
					client->sess.spectatorState = SPECTATOR_FREE;
					ClientUserinfoChanged( client - level.clients);	
					ClientBegin( client - level.clients );				
					AP(va("chat \"console: %s ^7was moved to Specators due inactivity.\n\"", client->pers.netname));
					return qfalse;
				} 
				else 
				{ 
					trap_DropClient( client - level.clients, "^3Dropped due to inactivity" );
					return qfalse;
				}
			} // Spectator && NOT admin
			else if (client->sess.sessionTeam == TEAM_SPECTATOR && 
					 client->sess.admin == ADM_NONE	&&				     
					 client->inactivityWarning)
			{
				if (g_spectatorInactivity.integer) 
				{		
					if (g_spectatorAllowDemo.integer && !(client->ps.pm_flags & PMF_FOLLOW))
					{
						trap_DropClient( client - level.clients, "^3Dropped due to inactivity" );
						return qfalse;
					}
					else if (!g_spectatorAllowDemo.integer)
					{
						trap_DropClient( client - level.clients, "^3Dropped due to inactivity" );
						return qfalse;
					}
				} 
			}			
		}
		else if ( !client->inactivityWarning && level.time > client->inactivityTime - 10 * 1000 ) 
		{
			if (client->sess.sessionTeam != TEAM_SPECTATOR)
			{
				if (g_inactivityToSpecs.integer) 
					trap_SendServerCommand( client - level.clients, "cp \"^3Ten seconds until forcing you to spectators!\n\"2" );
				else
					trap_SendServerCommand( client - level.clients, "cp \"^3Ten seconds until inactivity drop!\n\"2" );	
			}
			else if (client->sess.admin == ADM_NONE && 
				g_spectatorInactivity.integer && 
				client->sess.sessionTeam == TEAM_SPECTATOR ) 
			{
				if (g_spectatorAllowDemo.integer && !(client->ps.pm_flags & PMF_FOLLOW))
					trap_SendServerCommand( client - level.clients, "cp \"^3You have Ten seconds to join before ^3being dropped due inactivity!\n\"2" );
				else if (!g_spectatorAllowDemo.integer)
					trap_SendServerCommand( client - level.clients, "cp \"^3You have Ten seconds to join before ^3being dropped due inactivity!\n\"2" );
			}

			client->inactivityWarning = qtrue;
			client->inactivityTime = level.time + 10 * 1000; // Just for safety
		}
	}
	return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
	gclient_t *client;

	client = ent->client;
	client->timeResidual += msec;

	while ( client->timeResidual >= 1000 ) {
		client->timeResidual -= 1000;

		// regenerate
		// JPW NERVE, split these completely
		if (g_gametype.integer < GT_WOLF) {
			if ( client->ps.powerups[PW_REGEN] ) {
				if ( ent->health < client->ps.stats[STAT_MAX_HEALTH]) {
					ent->health += 15;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
					}
					G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
				} else if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] * 2) {
					ent->health += 2;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 2;
					}
					G_AddEvent( ent, EV_POWERUP_REGEN, 0 );
				}
			} else {
				// count down health when over max
				if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
					ent->health--;
				}
			}
		}
// JPW NERVE
		else { // GT_WOLF
			if ( client->ps.powerups[PW_REGEN] ) {
				if ( ent->health < client->ps.stats[STAT_MAX_HEALTH]) {
					ent->health += 3;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1){
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
					}
				} else if ( ent->health < client->ps.stats[STAT_MAX_HEALTH] * 1.12) {
						ent->health += 2;
					if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.12 ) {
						ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.12;
					}
				}
			} else {
				// count down health when over max
				if ( ent->health > client->ps.stats[STAT_MAX_HEALTH] ) {
					ent->health--;
				}
			}
		}
// jpw
		// count down armor when over max
		if ( client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH] ) {
			client->ps.stats[STAT_ARMOR]--;
		}

		// L0 - Global Stats - Track time on team/class
		globalStats_playerTimers(client);
		globalStats_statsTimers(client);
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
	client->ps.eFlags &= ~EF_TALK;
	client->ps.eFlags &= ~EF_FIRING;

	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = client->pers.cmd.buttons;

//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = client->pers.cmd.wbuttons;
	
	if(	( client->buttons & ( BUTTON_ATTACK | BUTTON_USE_HOLDABLE ) & ( client->oldbuttons ^ client->buttons ) ) ||
		( client->wbuttons & WBUTTON_ATTACK2 & ( client->oldwbuttons ^ client->wbuttons ) ) ) {
		client->readyToExit ^= 1;
	}
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
	int			i;
	int			event;
	gclient_t	*client;
	int			damage;

	client = ent->client;

	if ( oldEventSequence < client->ps.eventSequence - MAX_EVENTS ) {
		oldEventSequence = client->ps.eventSequence - MAX_EVENTS;
	}
	for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
		event = client->ps.events[ i & (MAX_EVENTS-1) ];

		switch ( event ) {
		case EV_FALL_NDIE:
		//case EV_FALL_SHORT:
		case EV_FALL_DMG_10:
		case EV_FALL_DMG_15:
		case EV_FALL_DMG_25:
		//case EV_FALL_DMG_30:
		case EV_FALL_DMG_50:
		//case EV_FALL_DMG_75:
			
			if ( ent->s.eType != ET_PLAYER ) {
				break;		// not in the player model
			}
			if ( g_dmflags.integer & DF_NO_FALLING ) {
				break;
			}
			if ( event == EV_FALL_NDIE ) 
			{
				damage = 9999;
			}
			else if (event == EV_FALL_DMG_50)
			{
				damage = 75; // L0 - Changed from 50..
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear (ent->client->ps.velocity);
			}
			else if (event == EV_FALL_DMG_25)
			{
				damage = 50; // L0 - Changed from 25..
				ent->client->ps.pm_time = 250;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear (ent->client->ps.velocity);
			}
			else if (event == EV_FALL_DMG_15)
			{
				damage = 20; // L0 - Changed from 15..
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear (ent->client->ps.velocity);
			}
			else if (event == EV_FALL_DMG_10)
			{
				damage = 10;
				ent->client->ps.pm_time = 1000;
				ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
				VectorClear (ent->client->ps.velocity);
			}
			else {
				damage = 5; // never used
			}
			
			// L0 - Goomba
			if ((g_goomba.value > 0) && 
				(ent->s.groundEntityNum < MAX_CLIENTS) && 
				(ent->s.groundEntityNum != -1) && 
				(g_entities[ent->s.groundEntityNum].client->ps.stats[STAT_HEALTH] > 0)) {
				int doDamage = damage * g_goomba.value;

				if (!OnSameTeam(&g_entities[ent->s.groundEntityNum], ent)) {
					G_Damage(&g_entities[ent->s.groundEntityNum], ent, ent, tv(0, 0, -1), NULL, doDamage, 0, MOD_GOOMBA);
				}				
			}
			
			ent->pain_debounce_time = level.time + 200;	// no normal pain sound
			G_Damage(ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
			break;
// JPW NERVE
		case EV_TESTID1:
		case EV_TESTID2:
		case EV_ENDTEST:
			break;
// jpw
		case EV_FIRE_WEAPON_MG42:
			// L0 - disable invincible time when player spawns and starts shooting
			if (g_disableInv.integer)
				ent->client->ps.powerups[PW_INVULNERABLE] = 0;
			// end
			mg42_fire( ent );
			break;

		case EV_FIRE_WEAPON:			
		case EV_FIRE_WEAPONB:
		case EV_FIRE_WEAPON_LASTSHOT:			
			FireWeapon( ent );
			break;

//----(SA)	modified
		case EV_USE_ITEM1:		// ( HI_MEDKIT )	medkit
		case EV_USE_ITEM2:		// ( HI_WINE )		wine
		case EV_USE_ITEM3:		// ( HI_SKULL )		skull of invulnerable
		case EV_USE_ITEM4:		// ( HI_WATER )		protection from drowning
		case EV_USE_ITEM5:		// ( HI_ELECTRIC )	protection from electric attacks
		case EV_USE_ITEM6:		// ( HI_FIRE )		protection from fire attacks
		case EV_USE_ITEM7:		// ( HI_STAMINA )	restores fatigue bar and sets "nofatigue" for a time period
		case EV_USE_ITEM8:		// ( HI_BOOK1 )		
		case EV_USE_ITEM9:		// ( HI_BOOK2 )		
		case EV_USE_ITEM10:		// ( HI_BOOK3 )		
			UseHoldableItem( ent, event - EV_USE_ITEM0);
			break;
//----(SA)	end

		default:
			break;
		}
	}

}

/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps ) {
	/*
	gentity_t *t;
	int event, seq;
	int extEvent, number;

	// if there are still events pending
	if ( ps->entityEventSequence < ps->eventSequence ) {
		// create a temporary entity for this event which is sent to everyone
		// except the client generated the event
		seq = ps->entityEventSequence & (MAX_EVENTS-1);
		event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );
		// set external event to zero before calling BG_PlayerStateToEntityState
		extEvent = ps->externalEvent;
		ps->externalEvent = 0;
		// create temporary entity for event
		t = G_TempEntity( ps->origin, event );
		number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, qtrue );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;
		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;
		// set back external event
		ps->externalEvent = extEvent;
	}
	*/
}

// DHM - Nerve
void WolfFindMedic( gentity_t *self ) {
	int i, medic=-1;
	gclient_t	*cl;
	vec3_t	start, end, temp;
	trace_t	tr;
	float	bestdist=1024, dist;

	self->client->ps.viewlocked_entNum = 0;
	self->client->ps.viewlocked = 0;
	self->client->ps.stats[STAT_DEAD_YAW] = 999;

	VectorCopy( self->s.pos.trBase, start );
	start[2] += self->client->ps.viewheight;

	for ( i=0; i<level.numPlayingClients; i++ ) {
		cl = &level.clients[ level.sortedClients[i] ];

		if ( cl->ps.clientNum == self->client->ps.clientNum )
			continue;
		if ( cl->sess.sessionTeam != self->client->sess.sessionTeam )
			continue;
		if ( cl->ps.stats[ STAT_HEALTH ] <= 0 )
			continue;
		if ( cl->ps.stats[ STAT_PLAYER_CLASS ] != PC_MEDIC )
			continue;

		VectorCopy( g_entities[level.sortedClients[i]].s.pos.trBase, end );
		end[2] += cl->ps.viewheight;

		trap_Trace (&tr, start, NULL, NULL, end, self->s.number, CONTENTS_SOLID);
		if ( tr.fraction < 0.95 )
			continue;

		VectorSubtract( end, start, end );
		dist = VectorNormalize( end );

		if ( dist < bestdist ) {
			medic = cl->ps.clientNum;
			vectoangles( end, temp );
			self->client->ps.stats[STAT_DEAD_YAW] = temp[YAW];
			bestdist = dist;
		}
	}

	if ( medic >= 0 ) {
		self->client->ps.viewlocked_entNum = medic;
		self->client->ps.viewlocked = 7;
	}
}

/*
==============
L0

LTinfoMsg
Shows ammo stocks of clients..
==============
*/
char *weaponStr( int weapon )
{
	switch ( weapon ) {
		case WP_MP40:				return "MP40";
		case WP_THOMPSON:			return "Thompson";
		case WP_STEN:				return "Sten";
		case WP_MAUSER:				return "Mauser";
		case WP_SNIPERRIFLE:		return "Sniper Rifle";
		case WP_FLAMETHROWER:		return "Flamethrower";
		case WP_PANZERFAUST:		return "Panzerfaust";
		case WP_VENOM:				return "Venom";
		case WP_GRENADE_LAUNCHER:	return "Grenade";
		case WP_GRENADE_PINEAPPLE:	return "Grenade";
		case WP_KNIFE:				return "Knife";
		case WP_KNIFE2:				return "Knife";
		case WP_LUGER:				return "Luger";
		case WP_COLT:				return "Colt";
		case WP_MEDIC_SYRINGE:		return "Syringe";
		default:
			return "";
	}
}
// Draw str
void LTinfoMSG( gentity_t *ent ) {
	unsigned int current = 0;
	unsigned int stock = 0;	
	unsigned int nades = 0;
	weapon_t weapon;

	gentity_t *target;
	trace_t tr;
	vec3_t start, end, forward;

		if (ent->client->ps.stats[STAT_HEALTH] <= 0)
		return;

		if(g_gamestate.integer != GS_PLAYING)
		return;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorCopy(ent->s.pos.trBase, start);	//set 'start' to the player's position (plus the viewheight)
	start[2] += ent->client->ps.viewheight;
	VectorMA (start, 512, forward, end);	//put 'end' 512 units forward of 'start'

		//see if we hit anything between 'start' and 'end'
	trap_Trace (&tr, start, NULL, NULL, end, ent->s.number, (CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE|CONTENTS_TRIGGER));

	if ( tr.surfaceFlags & SURF_NOIMPACT )	return;
	if ( tr.entityNum == ENTITYNUM_WORLD )	return;
	if (tr.entityNum >= MAX_CLIENTS)		return;

	target = &g_entities[ tr.entityNum ];
	if ((!target->inuse) || (!target->client))		return;
	if (target->client->ps.stats[STAT_HEALTH] <= 0)	return;
	if (!OnSameTeam(target,ent))					return;

	ent->client->infoTime = level.time;
	weapon = target->client->ps.weapon;
	current += target->client->ps.ammoclip[BG_FindClipForWeapon( weapon )];
	stock += target->client->ps.ammo[ BG_FindAmmoForWeapon( weapon )];
	nades += target->client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_PINEAPPLE )];
	nades += target->client->ps.ammoclip[BG_FindClipForWeapon( WP_GRENADE_LAUNCHER )];

	if (Q_stricmp(weaponStr( weapon ), ""))
	{
		if (weapon == WP_GRENADE_PINEAPPLE || weapon == WP_GRENADE_LAUNCHER )
			CP(va("cp \"%s: %i\n\"1", weaponStr( weapon ), current));
		else if (weapon == WP_KNIFE || weapon == WP_KNIFE2 )
			CP(va("cp \"%s - Grenades: %i\n\"1", weaponStr( weapon ), current, nades));
		else
			CP(va("cp \"%s: %i/%i - Grenades: %i\n\"1", weaponStr( weapon ), current, stock, nades));
	}
}

void limbo( gentity_t *ent, qboolean makeCorpse ); // JPW NERVE
void reinforce (gentity_t *ent ); // JPW NERVE

void ClientDamage( gentity_t *clent, int entnum, int enemynum, int id );		// NERVE - SMF

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
int teamRespawnTime(int team, qboolean warmup); // OSPx
void ClientThink_real( gentity_t *ent ) {
	gclient_t	*client;
	pmove_t		pm;
//	vec3_t		oldOrigin;
	int			oldEventSequence;
	int			msec;
	usercmd_t	*ucmd;
	int			monsterslick = 0;
// JPW NERVE
	int			i;
	vec3_t		muzzlebounce;
	gitem_t *item;
	gentity_t *ent2;
	vec3_t	velocity, org, offset;
	vec3_t	angles,mins,maxs;
	int		weapon;
	trace_t	tr;
// jpw

	// Rafael wolfkick
	//int			validkick;
	//static int	wolfkicktimer = 0;

	client = ent->client;

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if (client->pers.connected != CON_CONNECTED) {
		return;
	}

	if (client->cameraPortal) {
		G_SetOrigin( client->cameraPortal, client->ps.origin );
		trap_LinkEntity(client->cameraPortal);
		VectorCopy( client->cameraOrigin, client->cameraPortal->s.origin2);
	}

	// L0 - LT info
	if ( g_LTinfoMsg.integer && 
		( client->ps.stats[STAT_PLAYER_CLASS] == PC_LT ) && 
		( level.time >= client->infoTime + 1000 ) ) 
	{
		LTinfoMSG( ent );
	} 
	else if ( g_enableSpies.integer && 
		g_reportSpies.integer && 
		( level.time >= ent->client->infoTime + 1000 ) )        
	{
		checkSpies( ent );
	}// End

	// mark the time, so the connection sprite can be removed
	ucmd = &ent->client->pers.cmd;

	ent->client->ps.identifyClient = ucmd->identClient;		// NERVE - SMF

// JPW NERVE -- update counter for capture & hold display
	if (g_gametype.integer == GT_WOLF_CPH) {
		client->ps.stats[STAT_CAPTUREHOLD_RED] = level.capturetimes[TEAM_RED];
		client->ps.stats[STAT_CAPTUREHOLD_BLUE] = level.capturetimes[TEAM_BLUE];
	}
// jpw
	
	// sanity check the command time to prevent speedup cheating
	if ( ucmd->serverTime > level.time + 200 ) {
		ucmd->serverTime = level.time + 200;
//		G_Printf("serverTime <<<<<\n" );
	}
	if ( ucmd->serverTime < level.time - 1000 ) {
		ucmd->serverTime = level.time - 1000;
//		G_Printf("serverTime >>>>>\n" );
	} 

	msec = ucmd->serverTime - client->ps.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) {
		return;
		/*
		// Ridah, fixes savegame timing issue
		if (msec < -100) {
			client->ps.commandTime = ucmd->serverTime - 100;
			msec = 100;
		} else {
			return;
		}
		*/
		// done.
	}
	if ( msec > 200 ) {
		msec = 200;
	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap_Cvar_Set("pmove_msec", "33");
	}

	if ( pmove_fixed.integer || client->pers.pmoveFixed ) {
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	if ( level.intermissiontime ) {
		ClientIntermissionThink( client );
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	// OSP - moved here to allow for spec inactivity checks as well
	if ( !ClientInactivityTimer( client ) ) {
		return;
	}

	// spectators don't do much
	// DHM - Nerve :: In limbo use SpectatorThink
	if ( client->sess.sessionTeam == TEAM_SPECTATOR || client->ps.pm_flags & PMF_LIMBO ) {
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
			return;
		}
		SpectatorThink( ent, ucmd );
		return;
	}

	// JPW NERVE do some time-based muzzle flip -- this never gets touched in single player (see g_weapon.c)
	// #define RIFLE_SHAKE_TIME 150 // JPW NERVE this one goes with the commented out old damped "realistic" behavior below
	#define RIFLE_SHAKE_TIME 300 // per Id request, longer recoil time
	if (client->sniperRifleFiredTime) {
		if (level.time - client->sniperRifleFiredTime > RIFLE_SHAKE_TIME)
			client->sniperRifleFiredTime = 0;
		else {
			VectorCopy(client->ps.viewangles,muzzlebounce);

			// JPW per Id request, longer recoil time
			muzzlebounce[PITCH] -= 2*cos(2.5*(level.time - client->sniperRifleFiredTime)/RIFLE_SHAKE_TIME);
			muzzlebounce[YAW] += 0.5*client->sniperRifleMuzzleYaw*cos(1.0-(level.time - client->sniperRifleFiredTime)*3/RIFLE_SHAKE_TIME);
			muzzlebounce[PITCH] -= 0.25*random()*(1.0f-(level.time - client->sniperRifleFiredTime)/RIFLE_SHAKE_TIME);
			muzzlebounce[YAW] += 0.5*crandom()*(1.0f-(level.time - client->sniperRifleFiredTime)/RIFLE_SHAKE_TIME);
			SetClientViewAngle(ent,muzzlebounce);		
		}
	}
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC) {
		if (level.time > client->ps.powerups[PW_REGEN] + 5000) {
			client->ps.powerups[PW_REGEN] = level.time;
		}
	}
	// also update weapon recharge time

	// JPW drop button drops secondary weapon so new one can be picked up
	// TTimo explicit braces to avoid ambiguous 'else'
	if (g_gametype.integer != GT_SINGLE_PLAYER) {
		if (ucmd->wbuttons & WBUTTON_DROP) {
										// OSPx - Ignore all this if we're under pause..
			if (!client->dropWeaponTime && level.match_pause == PAUSE_NONE) {
				client->dropWeaponTime = 1; // just latch it for now

				// L0 - Throw knife & Drop Obj
				if ( client->ps.weapon == WP_KNIFE ) {
					if ( client->ps.stats[STAT_HEALTH] > 0 ) {
						if ( g_dropObj.integer > 0 ) 
							Cmd_DropObj( ent );
						else
							Cmd_ThrowKnives( ent );		
					}

					return;
				}// End

				// L0 - Patched it for g_unlockWeapons..
				if ( ( client->ps.stats[STAT_PLAYER_CLASS] == PC_SOLDIER ) || ( client->ps.stats[STAT_PLAYER_CLASS] == PC_LT ) || 
					  ( g_unlockWeapons.integer && client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC ) || 
					  ( g_unlockWeapons.integer && client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER )) 	
					for (i=0;i<MAX_WEAPS_IN_BANK_MP;i++) {
						weapon = weapBanksMultiPlayer[3][i];
							if (COM_BitCheck(client->ps.weapons,weapon)) {
		
								item = BG_FindItemForWeapon(weapon);
								VectorCopy( client->ps.viewangles, angles );

								// clamp pitch
								if ( angles[PITCH] < -30 )
									angles[PITCH] = -30;
								else if ( angles[PITCH] > 30 )
									angles[PITCH] = 30;

								AngleVectors( angles, velocity, NULL, NULL );
								VectorScale( velocity, 64, offset);
								offset[2] += client->ps.viewheight/2;
								VectorScale( velocity, 75, velocity );
								velocity[2] += 50 + random() * 35;
		
								VectorAdd(client->ps.origin,offset,org);
	
								VectorSet( mins, -ITEM_RADIUS, -ITEM_RADIUS, 0 );
								VectorSet( maxs, ITEM_RADIUS, ITEM_RADIUS, 2*ITEM_RADIUS );

								trap_Trace (&tr, client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID);
								VectorCopy( tr.endpos, org );
	
								ent2 = LaunchItem( item, org, velocity, client->ps.clientNum );
								COM_BitClear(client->ps.weapons,weapon);
	
								if ( weapon == WP_MAUSER )
									COM_BitClear(client->ps.weapons,WP_SNIPERRIFLE);
	
								// Clear out empty weapon, change to next best weapon
								G_AddEvent( ent, EV_NOAMMO, 0 );
	
								i=MAX_WEAPS_IN_BANK_MP;								
								if(client->ps.weapon == weapon) client->ps.weapon = 0;
								ent2->count = client->ps.ammoclip[BG_FindClipForWeapon(weapon)];					
								ent2->item->quantity = client->ps.ammoclip[BG_FindClipForWeapon(weapon)];					
								client->ps.ammoclip[BG_FindClipForWeapon(weapon)] = 0;
							}
					}
				}
		}
		else
			client->dropWeaponTime = 0;
  }
// jpw	

	// L0 - Admin bot, ping fluxation
	SB_maxPingFlux(client);
										  // OSPx - Pause
	if (reloading || client->cameraPortal || level.match_pause != PAUSE_NONE) {
		ucmd->buttons = 0;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		ucmd->wbuttons = 0;
		ucmd->wolfkick = 0;

		// OSP - Pause
		if (level.match_pause != PAUSE_NONE)
		{
			client->ps.pm_type = PM_FREEZE;
		} // -OSPx
		if (client->cameraPortal) {
			VectorClear(client->ps.velocity);
			client->ps.pm_type = PM_FREEZE;
		}
	// OSPx - So we don't miss any..
	} else if (level.match_pause != PAUSE_NONE) {
		client->ps.pm_type = PM_FREEZE;
	} else if (client->noclip) {
		client->ps.pm_type = PM_NOCLIP;
	} else if (client->ps.stats[STAT_HEALTH] <= 0) {
		client->ps.pm_type = PM_DEAD;
	} else {
		client->ps.pm_type = PM_NORMAL;
	}

	// L0 - Poison / Reset view when player dies from smth else while being poisoned..	
	if (ent->health <= 0) {
		ent->client->ps.eFlags &= ~EF_POISONED;
	} // end

	// set parachute anim condition flag
	BG_UpdateConditionValue( ent->s.number, ANIM_COND_PARACHUTE, (ent->flags & FL_PARACHUTE) != 0, qfalse );

	// all playing clients are assumed to be in combat mode
	if ( !client->ps.aiChar ) {
		client->ps.aiState = AISTATE_COMBAT;
	}

	client->ps.gravity = g_gravity.value;

	// set speed
	client->ps.speed = g_speed.value;

	if ( client->ps.powerups[PW_HASTE] ) {
		client->ps.speed *= 1.3;
	}

	// set up for pmove
	oldEventSequence = client->ps.eventSequence;

	client->currentAimSpreadScale = (float)client->ps.aimSpreadScale/255.0;

	memset (&pm, 0, sizeof(pm));

	pm.ps = &client->ps;
	pm.pmext = &client->pmext;
	pm.cmd = *ucmd;
	pm.oldcmd = client->pers.oldcmd;
	if ( pm.ps->pm_type == PM_DEAD ) {
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
		// DHM-Nerve added:: EF_DEAD is checked for in Pmove functions, but wasn't being set
		//              until after Pmove
		pm.ps->eFlags |= EF_DEAD;
		// dhm-Nerve end
	}
	else {
		pm.tracemask = MASK_PLAYERSOLID;
	}
	// MrE: always use capsule for AI and player
	//pm.trace = trap_TraceCapsule;//trap_Trace;
	//DHM - Nerve :: We've gone back to using normal bbox traces
	pm.trace = trap_Trace;
	pm.pointcontents = trap_PointContents;
	pm.debugLevel = g_debugMove.integer;
	pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

	pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec = pmove_msec.integer;

	pm.noWeapClips = ( g_dmflags.integer & DF_NO_WEAPRELOAD ) > 0;
	if (ent->aiCharacter && AICast_NoReload(ent->s.number))
		pm.noWeapClips = qtrue;	// ensure AI characters don't use clips if they're not supposed to.

	// L0 - Fixedpsyhics
	if ( g_fixedphysics.integer ) {
		pm.fixedphysicsfps = 125;
	} // End

	// Ridah
//	if (ent->r.svFlags & SVF_NOFOOTSTEPS)
//		pm.noFootsteps = qtrue;

	VectorCopy( client->ps.origin, client->oldOrigin );

	// NERVE - SMF
	pm.gametype = g_gametype.integer;
	pm.ltChargeTime = g_LTChargeTime.integer;
	pm.soldierChargeTime = g_soldierChargeTime.integer;
	pm.engineerChargeTime = g_engineerChargeTime.integer;
	pm.medicChargeTime = g_medicChargeTime.integer;
	// -NERVE - SMF

	monsterslick = Pmove (&pm);

	if (monsterslick && !(ent->flags & FL_NO_MONSTERSLICK))
	{
		//vec3_t	dir;
		//vec3_t	kvel;
		//vec3_t	forward;
  // TTimo gcc: might be used unitialized in this function
		float	angle = 0.0f;
		qboolean bogus = qfalse;

		// NE
		if ((monsterslick & SURF_MONSLICK_N) && (monsterslick & SURF_MONSLICK_E))
		{
			angle = 45;
		}
		// NW
		else if ((monsterslick & SURF_MONSLICK_N) && (monsterslick & SURF_MONSLICK_W))
		{
			angle = 135;
		}
		// N
		else if (monsterslick & SURF_MONSLICK_N)
		{
			angle = 90;
		}

		// SE
		else if ((monsterslick & SURF_MONSLICK_S) && (monsterslick & SURF_MONSLICK_E))
		{
			angle = 315;
		}
		// SW
		else if ((monsterslick & SURF_MONSLICK_S) && (monsterslick & SURF_MONSLICK_W))
		{
			angle = 225;
		}
		// S
		else if (monsterslick & SURF_MONSLICK_S)
		{
			angle = 270;
		}

		// E
		else if (monsterslick & SURF_MONSLICK_E)
		{
			angle = 0;
		}
		// W
		else if (monsterslick & SURF_MONSLICK_W)
		{
			angle = 180;
		}
		else
		{
			bogus = qtrue;
		}
	}

	// server cursor hints
	if ( ent->lastHintCheckTime < level.time ) {
		G_CheckForCursorHints(ent);

		ent->lastHintCheckTime = level.time + FRAMETIME;
	}

	// DHM - Nerve :: Set animMovetype to 1 if ducking
	if ( ent->client->ps.pm_flags & PMF_DUCKED )
		ent->s.animMovetype = 1;
	else
		ent->s.animMovetype = 0;

	// save results of pmove
	if ( ent->client->ps.eventSequence != oldEventSequence ) {
		ent->eventTime = level.time;
		ent->r.eventTime = level.time;
	}

	// Ridah, fixes jittery zombie movement
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
	}

	if ( !( ent->client->ps.eFlags & EF_FIRING ) ) {
		client->fireHeld = qfalse;		// for grapple
	}

//
//	// use the precise origin for linking
//	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
//
//	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy (pm.mins, ent->r.mins);
	VectorCopy (pm.maxs, ent->r.maxs);

	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;

	// OSPx - Pause wrapper
	if (level.match_pause == PAUSE_NONE) {
		// execute client events
		ClientEvents(ent, oldEventSequence);
	}

	// link entity now, after any personal teleporters have been used
	trap_LinkEntity (ent);
	if ( !ent->client->noclip ) {
		G_TouchTriggers( ent );
	}

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

	// L0 - antilag
	G_StoreTrail( ent );
	// L0 - end

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if (ent->client->ps.eventSequence != oldEventSequence) {
		ent->eventTime = level.time;
	}

	// swap and latch button actions
	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons = client->buttons & ~client->oldbuttons;
//	client->latched_buttons |= client->buttons & ~client->oldbuttons;	// FIXME:? (SA) MP method (causes problems for us.  activate 'sticks')

	//----(SA)	added
	client->oldwbuttons = client->wbuttons;
	client->wbuttons = ucmd->wbuttons;
	client->latched_wbuttons = client->wbuttons & ~client->oldwbuttons;
//	client->latched_wbuttons |= client->wbuttons & ~client->oldwbuttons;	// FIXME:? (SA) MP method

	// Rafael - Activate
	// Ridah, made it a latched event (occurs on keydown only)
	if (client->latched_buttons & BUTTON_ACTIVATE)
	{
		Cmd_Activate_f (ent);
		Cmd_Push( ent );	// L0 - Shove
		Cmd_Drag( ent );	// L0 - Drag bodies	
	}

	// L0 - drag bodies
	if (client->buttons & BUTTON_GESTURE || client->buttons & BUTTON_ACTIVATE) {
		Cmd_Drag( ent );
	} // End	

	if (ent->flags & FL_NOFATIGUE)
		ent->client->ps.sprintTime = 20000;


	// check for respawning
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) {

		// DHM - Nerve
		if ( g_gametype.integer >= GT_WOLF )
			WolfFindMedic( ent );
		// dhm - end

		// wait for the attack button to be pressed
		if ( level.time > client->respawnTime ) {
			// forcerespawn is to prevent users from waiting out powerups
			if ((g_gametype.integer != GT_SINGLE_PLAYER) &&
				(g_forcerespawn.integer > 0) && 
				(( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000)  && 
				(!(ent->client->ps.pm_flags & PMF_LIMBO)) ) { // JPW NERVE
				// JPW NERVE
				if (g_gametype.integer >= GT_WOLF)
					limbo(ent, qtrue);
				else
					respawn(ent);
				// jpw
				return;
			}

			// DHM - Nerve :: Single player game respawns immediately as before,
			//				  but in multiplayer, require button press before respawn
			if ( g_gametype.integer == GT_SINGLE_PLAYER )
				respawn( ent );
			// NERVE - SMF - we want to only respawn on jump button now
			else if (( ucmd->upmove > 0 ) &&
				(!(ent->client->ps.pm_flags & PMF_LIMBO)) ) { // JPW NERVE
				// JPW NERVE
				if (g_gametype.integer >= GT_WOLF) 
				{
					limbo(ent, qtrue);

					// L0 - Tap reports
					// 1 = Prints to team only
					if (g_tapReports.integer > 1)
					{						
						G_TeamCommand( 
							( (ent->client->sess.sessionTeam == TEAM_RED) ? TEAM_RED : TEAM_BLUE ),
							va("print \"%s ^7tapped out.\n\"", ent->client->pers.netname) 
						);
					} 
					// 2 = Prints to everyone..
					else if (g_tapReports.integer == 1) 
					{						
						AP(va("print \"%s ^7tapped out.\n\"", ent->client->pers.netname));
					} // End

				}
				else
				{	
					respawn(ent);
				}
				// jpw
			}
			// dhm - Nerve :: end
			// NERVE - SMF - we want to immediately go to limbo mode if gibbed
			else if ( client->ps.stats[STAT_HEALTH] <= GIB_HEALTH && !( ent->client->ps.pm_flags & PMF_LIMBO ) ) {
				if (g_gametype.integer >= GT_WOLF)
					limbo(ent, qfalse);
				else
					respawn(ent);
			}
			// -NERVE - SMF
		}
		return;
	}

	// L0 - Max TKs - Drop player if over limit and time has expired..
	if (sb_system.integer && sb_maxTKs.integer > 0)
	{
		int count = ent->client->pers.stats.teamKills+1 - ent->client->pers.sb_TKforgiven;

		if (count > sb_maxTKs.integer)
		{
			if (ent->client->pers.sb_TKkillTime < level.time)
			{
				// Tempban
				if (sb_makTKsTempbanMins.integer)
				{
					if (sb_tempbanIP.integer && IP_handling.integer)
						trap_SendConsoleCommand(EXEC_APPEND, va("tempban %i %i", ent->client->ps.clientNum, sb_makTKsTempbanMins ));
					else
						trap_SendConsoleCommand(EXEC_APPEND, va("tempbanguid %s %i", ent->client->sess.guid, sb_makTKsTempbanMins));	
				}

				trap_DropClient( ent-g_entities, "Kicked \n^3For Team Killing." );		
				AP(va("chat \"^3SB^7: %s ^7got kicked for ^3Team Killing^7.\n\"", ent->client->pers.netname));
			}
		}
	}

	// OSPx - Pause wrapper
	if (level.match_pause == PAUSE_NONE) {
		// perform once-a-second actions
		ClientTimerActions(ent, msec);
	}
}

/*
==================
L0 - ClientThink_cmd

ET->S4NDMoD port
==================
*/
void ClientThink_cmd(gentity_t *ent, usercmd_t *cmd) {
	ent->client->pers.oldcmd = ent->client->pers.cmd;
	ent->client->pers.cmd = *cmd;
	ClientThink_real(ent);
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/
void ClientThink( int clientNum ) {
	gentity_t *ent;
// L0 - Anti Warp port (Modified)
	usercmd_t newcmd; 

	ent = g_entities + clientNum;
	trap_GetUsercmd(clientNum, &newcmd);
// - End

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client->lastCmdTime = level.time;

	if ( !g_synchronousClients.integer ) {
		// L0 - Anti Warp port
		if (G_DoAntiwarp(ent)) {
			// josh: use zinx antiwarp code
			etpro_AddUsercmd(clientNum, &newcmd);
			DoClientThinks(ent);
		}
		else {
			ClientThink_cmd(ent, &newcmd);
		} // -End
	}
}


void G_RunClient( gentity_t *ent ) {
	// L0 - Anti Warp port
	if (G_DoAntiwarp(ent)) {
		// josh: use zinx antiwarp code
		DoClientThinks(ent);
	}

	if ( !g_synchronousClients.integer ) {
		return;
	}
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink_real( ent );
}

/*
==================
OSPx - Respawn check

Moved this here so it can be reused..
==================
*/
int teamRespawnTime(int team, qboolean warmup) {
	int time = 0;

	if (team == TEAM_RED) {
		if (warmup)
			time = level.time % 3000;
		else
			time = (level.dwRedReinfOffset + level.timeCurrent - level.startTime) % g_redlimbotime.integer;
	}
	else {
		if (warmup)
			time = level.time % 3000;
		else
			time = (level.dwBlueReinfOffset + level.timeCurrent - level.startTime) % g_bluelimbotime.integer;
	}
	return time;
}

/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
	gclient_t	*cl;
	int do_respawn=0; // JPW NERVE
	int	savedScore;		// DHM - Nerve
	int savedRespawns;	// DHM - Nerve
	int savedClass;		// NERVE - SMF
	int flags;
	int testtime;

	// if we are doing a chase cam or a remote view, grab the latest info
	if (( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) || (ent->client->ps.pm_flags & PMF_LIMBO)) { // JPW NERVE for limbo
		int		clientNum;

		if (ent->client->sess.sessionTeam == TEAM_RED) {
			// OSPx - Reinforcements Offset (patched)
			testtime = teamRespawnTime(TEAM_RED, qfalse);

			// L0 - If warmup damage is on, respawn instantly
			if ( g_warmupDamage.integer ) {
				if ( g_gamestate.integer != GS_PLAYING ) {
					testtime = teamRespawnTime(TEAM_RED, qtrue);
					do_respawn = 1;
				}
			} // End

			if (testtime < ent->client->pers.lastReinforceTime)
				do_respawn=1;

			ent->client->pers.lastReinforceTime = testtime;
		}
		else if (ent->client->sess.sessionTeam == TEAM_BLUE) {
			// OSPx - Reinforcements Offset (patched)
			testtime = teamRespawnTime(TEAM_BLUE, qfalse);

			// L0 - If warmup damage is on, respawn instantly
			if ( g_warmupDamage.integer ) {
				if ( g_gamestate.integer != GS_PLAYING ) {
					testtime = teamRespawnTime(TEAM_BLUE, qtrue);
					do_respawn = 1;
				}
			} // End

			if (testtime < ent->client->pers.lastReinforceTime)
				do_respawn=1;

			ent->client->pers.lastReinforceTime = testtime;
		}

		if ( ( g_maxlives.integer > 0 || g_alliedmaxlives.integer > 0 || g_axismaxlives.integer > 0 ) && ent->client->ps.persistant[PERS_RESPAWNS_LEFT] == 0 ) {
			do_respawn = 0;
		}

		if ( do_respawn ) {
			reinforce(ent);
			return;
		}

		clientNum = ent->client->sess.spectatorClient;

		// team follow1 and team follow2 go to whatever clients are playing
		if ( clientNum == -1 ) {
			clientNum = level.follow1;
		} else if ( clientNum == -2 ) {
			clientNum = level.follow2;
		}
		if ( clientNum >= 0 ) {
			cl = &level.clients[ clientNum ];
			if ( cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR ) {
				// L0 - Ping and Score fix (solves issue with \serverstatus showing who's spectating who)
				int ping = ent->client->ps.ping; 
				int score = ent->client->ps.persistant[PERS_SCORE];
				// DHM - Nerve :: carry flags over
				flags = (cl->ps.eFlags & ~(EF_VOTED)) | (ent->client->ps.eFlags & (EF_VOTED));				

				// JPW NERVE -- limbo latch
				if (ent->client->sess.sessionTeam != TEAM_SPECTATOR && ent->client->ps.pm_flags & PMF_LIMBO) {
					// abuse do_respawn var
					savedScore = ent->client->ps.persistant[PERS_SCORE];
					do_respawn = ent->client->ps.pm_time;
					savedRespawns = ent->client->ps.persistant[PERS_RESPAWNS_LEFT];
					savedClass = ent->client->ps.stats[STAT_PLAYER_CLASS];

					ent->client->ps = cl->ps;
					ent->client->ps.pm_flags |= PMF_FOLLOW;
					ent->client->ps.pm_flags |= PMF_LIMBO;

					ent->client->ps.persistant[PERS_RESPAWNS_LEFT] = savedRespawns;	
					ent->client->ps.pm_time = do_respawn;							// put pm_time back
					ent->client->ps.persistant[PERS_SCORE] = savedScore;			// put score back
					ent->client->ps.stats[STAT_PLAYER_CLASS] = savedClass;			// NERVE - SMF - put player class back
				}
				else {
					ent->client->ps = cl->ps;
					ent->client->ps.pm_flags |= PMF_FOLLOW;
				}
				// jpw
				// DHM - Nerve :: carry flags over
				ent->client->ps.eFlags = flags;
				// L0 - Ping & Score fix
				ent->client->ps.ping = ping; 
				ent->client->ps.persistant[PERS_SCORE] = score;
				return;
			} else {
				// drop them to free spectators unless they are dedicated camera followers
				if ( ent->client->sess.spectatorClient >= 0 ) {
					ent->client->sess.spectatorState = SPECTATOR_FREE;
					ClientBegin( ent->client - level.clients );
				}
			}
		}
	}

	if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
		ent->client->ps.pm_flags |= PMF_SCOREBOARD;
	} else {
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
	}

	// OSPx - Speclock
	ent->client->ps.powerups[PW_BLACKOUT] = (G_blockoutTeam(ent, TEAM_RED) * TEAM_RED) |
											(G_blockoutTeam(ent, TEAM_BLUE) * TEAM_BLUE);
}


// DHM - Nerve :: After reviving a player, their contents stay CONTENTS_CORPSE until it is determined
//					to be safe to return them to PLAYERSOLID

qboolean StuckInClient( gentity_t *self ) {
	gentity_t *hit;
	vec3_t	hitmin, hitmax;
	vec3_t	selfmin, selfmax;
	int i;

	hit = &g_entities[0];
	for ( i = 0; i < level.maxclients; i++, hit++ ) {
		if ( !hit->inuse ) {
			continue;
		}
		if ( hit == self ) {
			continue;
		}
		if ( !hit->client ) {
			continue;
		}
		if ( !hit->s.solid ) {
			continue;
		}
		if ( hit->health <= 0 ) {
			continue;
		}

		VectorAdd( hit->r.currentOrigin, hit->r.mins, hitmin );
		VectorAdd( hit->r.currentOrigin, hit->r.maxs, hitmax );
		VectorAdd( self->r.currentOrigin, self->r.mins, selfmin );
		VectorAdd( self->r.currentOrigin, self->r.maxs, selfmax );

		if (hitmin[0] > selfmax[0])
			continue;
		if (hitmax[0] < selfmin[0])
			continue;
		if (hitmin[1] > selfmax[1])
			continue;
		if (hitmax[1] < selfmin[1])
			continue;
		if (hitmin[2] > selfmax[2])
			continue;
		if (hitmax[2] < selfmin[2])
			continue;

		return qtrue;
	}
	return qfalse;
}

extern vec3_t	playerMins, playerMaxs;
#define WR_PUSHAMOUNT 25

void WolfRevivePushEnt( gentity_t *self, gentity_t *other ) {
	vec3_t	dir, push;

	VectorSubtract( self->r.currentOrigin, other->r.currentOrigin, dir );
	dir[2] = 0;
	VectorNormalizeFast( dir );

	VectorScale( dir, WR_PUSHAMOUNT, push );

	if ( self->client ) {
		VectorAdd( self->s.pos.trDelta, push, self->s.pos.trDelta );
		VectorAdd( self->client->ps.velocity, push, self->client->ps.velocity );
	}

	VectorScale( dir, -WR_PUSHAMOUNT, push );
	push[2] = WR_PUSHAMOUNT/2;

	VectorAdd( other->s.pos.trDelta, push, other->s.pos.trDelta );
	//VectorAdd( other->client->ps.velocity, push, other->client->ps.velocity );
	if ( other->client ) {
  		VectorAdd( other->client->ps.velocity, push, other->client->ps.velocity );
	}
}

void WolfReviveBbox( gentity_t *self ) {
	int			touch[MAX_GENTITIES];
	int			num,i, touchnum=0;
	gentity_t	*hit = NULL;
	vec3_t		mins, maxs;
	gentity_t	*capsulehit = NULL;

	VectorAdd( self->r.currentOrigin, playerMins, mins );
	VectorAdd( self->r.currentOrigin, playerMaxs, maxs );

	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	// Arnout, we really should be using capsules, do a quick, more refined test using mover collision
	if( num ) {
		capsulehit = G_TestEntityPosition( self );
	}

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		if ( hit->client ) {
			// ATVI Wolfenstein Misc #467
			// don't look at yourself when counting the hits
			if ( hit->client->ps.persistant[PERS_HWEAPON_USE] && hit!=self) {
				touchnum++;
				// Move corpse directly to the person who revived them
				if ( self->props_frame_state >= 0 ) {
					trap_UnlinkEntity( self );
					VectorCopy( g_entities[self->props_frame_state].client->ps.origin, self->client->ps.origin );
					VectorCopy( self->client->ps.origin, self->r.currentOrigin );
					trap_LinkEntity( self );

					// Reset value so we don't continue to warp them
					self->props_frame_state = -1;
				}
			} else if ( hit->health > 0 ) {
				if ( hit->s.number != self->s.number ) {
					WolfRevivePushEnt( hit, self );
					touchnum++;
				}
			}
		}
		else if ( hit->r.contents & ( CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_PLAYERCLIP ) ) {
			// Arnout: if hit is a mover, use capsulehit (this will only work if we touch one mover at a time - situations where you hit two are
			// really rare anyway though. The real fix is to move everything to capsule collision detection though
			if( hit->s.eType == ET_MOVER ) {
				if( capsulehit && capsulehit != hit ) {
					continue;	// we collided with a mover, but we're not stuck in this one
				} else {
					continue;	// we didn't collide with any movers
				}
			}

			WolfRevivePushEnt( hit, self );
			touchnum++;
		}
	}

	if (g_dbgRevive.integer)
		G_Printf( "WolfReviveBbox: touchnum: %d\n", touchnum );

	if ( touchnum == 0 ) {
		if (g_dbgRevive.integer)
			G_Printf( "WolfReviveBbox:  Player is solid now!\n" );
		self->r.contents = CONTENTS_BODY;
	}
}

// dhm

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEndFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
	int			i;

	// OSPx
	// used for informing of speclocked teams.
	// Zero out here and set only for certain specs
	ent->client->ps.powerups[PW_BLACKOUT] = 0;

	if (( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) || (ent->client->ps.pm_flags & PMF_LIMBO)) { // JPW NERVE
		SpectatorClientEndFrame( ent );
		return;
	}

	if (!ent->aiCharacter) {
		// turn off any expired powerups
		for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {

			if(	i == PW_FIRE ||				// these aren't dependant on level.time
				i == PW_ELECTRIC ||
				i == PW_BREATHER ||
				i == PW_NOFATIGUE) {

				continue;
			}

			// OSPx - Pause
			if (level.match_pause != PAUSE_NONE &&
				ent->client->ps.powerups[i] != INT_MAX) {
				ent->client->ps.powerups[i] += level.time - level.previousTime;
			}

			if (ent->client->ps.powerups[i] < level.time) {
				ent->client->ps.powerups[ i ] = 0;
			}
		}
	}

	// OSPx - Pause
	if (level.match_pause != PAUSE_NONE) {
		int time_delta = level.time - level.previousTime;

		ent->client->airOutTime += time_delta;
		ent->client->inactivityTime += time_delta;
		ent->client->lastBurnTime += time_delta;
		ent->client->pers.connectTime += time_delta;
		ent->client->pers.enterTime += time_delta;
		ent->client->pers.teamState.lastreturnedflag += time_delta;
		ent->client->pers.teamState.lasthurtcarrier += time_delta;
		ent->client->pers.teamState.lastfraggedcarrier += time_delta;
		ent->client->ps.classWeaponTime += time_delta;
		ent->client->respawnTime += time_delta;
		ent->client->sniperRifleFiredTime += time_delta;
		ent->lastHintCheckTime += time_delta;
		ent->pain_debounce_time += time_delta;
		ent->s.onFireEnd += time_delta;
	}

	// save network bandwidth
#if 0
	if ( !g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL ) {
		// FIXME: this must change eventually for non-sync demo recording
		VectorClear( ent->client->ps.viewangles );
	}
#endif

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if ( level.intermissiontime ) {
		return;
	}

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if ( level.time - ent->client->lastCmdTime > 1000 ) {
		ent->s.eFlags |= EF_CONNECTION;
	} else {
		ent->s.eFlags &= ~EF_CONNECTION;
	}

	ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

	G_SetClientSound (ent);

	// set the latest infor

	// Ridah, fixes jittery zombie movement
	if (g_smoothClients.integer) {
		BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, ((ent->r.svFlags & SVF_CASTAI) == 0) );
	}
	else {
		BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, ((ent->r.svFlags & SVF_CASTAI) == 0) );
	}

	//SendPendingPredictableEvents( &ent->client->ps );

	// DHM - Nerve :: If it's been a couple frames since being revived, and props_frame_state
	//					wasn't reset, go ahead and reset it
	if ( ent->props_frame_state >= 0 && ( (level.time - ent->s.effect3Time) > 100 ) )
		ent->props_frame_state = -1;

	if ( ent->health > 0 && StuckInClient( ent ) ) {
		G_DPrintf( "%s is stuck in a client.\n", ent->client->pers.netname );
		ent->r.contents = CONTENTS_CORPSE;
	}

	if ( g_gametype.integer >= GT_WOLF && ent->health > 0 && ent->r.contents == CONTENTS_CORPSE ) {
		WolfReviveBbox( ent );
	}

	// DHM - Nerve :: Reset 'count2' for flamethrower
	if ( !(ent->client->buttons & BUTTON_ATTACK) )
		ent->count2 = 0;
	// dhm
}
