/*
===========================================================================
g_match.c

Handle match related stuff, much like in et..

Author: Nate 'L0
Created: 8.Sept/13
Updated:
===========================================================================
*/
#include "g_local.h"

/*
=================
FlagBalance

Checks if teams are uneven and favours one with less to claim last take-retake 
till teams even up again, then it unlocks so it can be (re)claimed again.
=================
*/
int FlagBalance( void ) {

	if (!g_balanceFlagRetake.integer)
		return 0;

	sortedActivePlayers();

	// Avoid dealing with check if there's less than 2 players..
	if (level.axisPlayers < 2 && level.alliedPlayers < 2)
		return 0;

	// Even
	if (level.axisPlayers == level.alliedPlayers)
		return 0;
	// Axis (less)
	else if (level.axisPlayers < level.alliedPlayers)
		return 1;
	// Allied (less)
	else if (level.axisPlayers > level.alliedPlayers)
		return 2;
	// We don't know what happen...
	else
		return 0;
}

/*
================
Selected weapon
================
*/
void setCustomMG( gentity_t* ent, int type ) {

	if (!g_customMGs.integer) 
	{
		CP("cp \"Custom MGs are ^3disabled ^7on this server!\n\"2");
		return;
	}

	if (ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC ||
		ent->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER ) {

		if (type == 1) {
			if (ent->client->sess.selectedWeapon == WP_MP40)
			{
				ent->client->sess.selectedWeapon = 
					(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
				CP("cp \"You will spawn with default weapon!\n\"2");
			} else {
				CP("cp \"You will spawn with ^3MP40^7!\n\"2");
				ent->client->sess.selectedWeapon = WP_MP40;
			}

		} else if (type == 2) {
			if (ent->client->sess.selectedWeapon == WP_THOMPSON)
			{
				ent->client->sess.selectedWeapon = 
					(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
				CP("cp \"You will spawn with default weapon!\n\"2");
			} else {
				CP("cp \"You will spawn with ^3THOMPSON^7!\n\"2");
				ent->client->sess.selectedWeapon = WP_THOMPSON;
			}

		} else if (type == 3) {
			if (ent->client->sess.selectedWeapon == WP_STEN)
			{
				ent->client->sess.selectedWeapon = 
					(ent->client->sess.sessionTeam == TEAM_RED) ? WP_MP40 : WP_THOMPSON;
				CP("cp \"You will spawn with default weapon!\n\"2");
			} else {
				CP("cp \"You will spawn with ^3STEN^7!\n\"2");
				ent->client->sess.selectedWeapon = WP_STEN;
			}
		} 

	// Lt's and soliders can't choose weapons..
	} else {
		CP("cp \"^3Only Medics and Engineers can choose custom SMG!\n\"2");
	}
}

/*
=================
Weapon limiting

See if player can spawn with weapon...
TODO: Add first in line...
=================
*/
int sortWeaponLimit(int weap) {

	if (weap == 6) {		
		if (g_maxTeamSniper.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamSniper.integer;
	}

	if (weap == 8) {		
		if (g_maxTeamPF.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamPF.integer;
	}

	if (weap == 9) {	
		if (g_maxTeamVenom.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamVenom.integer;
	}

	if (weap == 10) {		
		if (g_maxTeamFlamer.integer == (-1))
			return g_maxclients.integer;
		else
			return g_maxTeamFlamer.integer;
	}

	return g_maxclients.integer;
}

int isWeaponLimited( gclient_t *client, int weap ) {
	int count=0;

	// Limit
	if (( weap == 6 ) && ( client->pers.restrictedWeapon != WP_MAUSER ) )
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisSniper : level.alliedSniper;
	else if (( weap == 8 ) && ( client->pers.restrictedWeapon != WP_PANZERFAUST ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisPF : level.alliedPF;
	else if (( weap == 9 )  && ( client->pers.restrictedWeapon != WP_VENOM ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisVenom : level.alliedVenom;
	else if (( weap == 10 ) && ( client->pers.restrictedWeapon != WP_FLAMETHROWER ))
		count = (client->sess.sessionTeam == TEAM_RED) ? level.axisFlamer : level.alliedFlamer;

	if (count >= sortWeaponLimit(weap))
		return 1;
	else
		return 0;

	return 0;	
}

/*
=================
Weapon balance

Checks if there's enough of players per team so player
can spawn with heavy weapon..
=================
*/
qboolean isWeaponBalanced( int weapon ) {
	sortedActivePlayers();

	// Sniper
	if (g_balanceSniper.integer && (weapon == 6)) {
		if ((level.axisPlayers >= g_balanceSniper.integer) && (level.alliedPlayers >= g_balanceSniper.integer))
			return qtrue;
		else
			return qfalse;	
	// PF check
	} else if (g_balancePF.integer && (weapon == 8)) {
		if ((level.axisPlayers >= g_balancePF.integer) && (level.alliedPlayers >= g_balancePF.integer))
			return qtrue;
		else
			return qfalse;
	// Venom check
	} else if (g_balanceVenom.integer && (weapon == 9)) {
		if ((level.axisPlayers >= g_balanceVenom.integer) && (level.alliedPlayers >= g_balanceVenom.integer))
			return qtrue;
		else
			return qfalse;
	// Flamer check
	} else if (g_balancePF.integer && (weapon == 10)) {
		if ((level.axisPlayers >= g_balanceFlamer.integer) && (level.alliedPlayers >= g_balanceFlamer.integer))
			return qtrue;
		else
			return qfalse;	
	}

	return qtrue;
}


/*
================
Default weapon 

Accounts for "selected weapon" as well.
================
*/
///////////
// Deals only with soldier for weapon restrictions (To avoid breaking anything..).
void setDefWeap(gclient_t *client, int clips) {
	if (client->sess.sessionTeam == TEAM_RED)
	{	
		COM_BitSet(client->ps.weapons, WP_MP40);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_MP40)] += 32;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_MP40)] += (32 * clips);
		client->ps.weapon = WP_MP40;
	} else {
		COM_BitSet(client->ps.weapons, WP_THOMPSON);
		client->ps.ammoclip[BG_FindClipForWeapon(WP_THOMPSON)] += 30;
		client->ps.ammo[BG_FindAmmoForWeapon(WP_THOMPSON)] += (30 * clips);
		client->ps.weapon = WP_THOMPSON;
	}
}

// NOTE: Selected weapons only works for eng and med..sold and lt can pick their weapons already..
//       so setting it can potentialy overlap with client spawn scripts..
void setDefaultWeapon(gclient_t *client, qboolean isSold) {
	int ammo;

	// This deals with weapon restrictions.
	if (isSold)  {
		setDefWeap(client, g_soldierClips.integer);
	return;
	}

	// Sorts ammo
	ammo = (client->sess.selectedWeapon == WP_THOMPSON) ? 30 : 32;

	// Medic
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC) {
		if (g_customMGs.integer && client->sess.selectedWeapon != 0) {
			COM_BitSet(client->ps.weapons, client->sess.selectedWeapon);
			client->ps.ammoclip[BG_FindClipForWeapon(client->sess.selectedWeapon)] += ammo;
			client->ps.ammo[BG_FindAmmoForWeapon(client->sess.selectedWeapon)] += (ammo * g_medicClips.integer);
			client->ps.weapon = client->sess.selectedWeapon;
		return;
		} else {
			setDefWeap(client, g_medicClips.integer);
		return;
		}
	}

	// Engineer
	if (client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER) {
		if (g_customMGs.integer && client->sess.selectedWeapon != 0) {
			COM_BitSet(client->ps.weapons, client->sess.selectedWeapon);
			client->ps.ammoclip[BG_FindClipForWeapon(client->sess.selectedWeapon)] += ammo;
			client->ps.ammo[BG_FindAmmoForWeapon(client->sess.selectedWeapon)] += (ammo * g_engineerClips.integer);
			client->ps.weapon = client->sess.selectedWeapon;
		return;
		} else {
			setDefWeap(client, g_engineerClips.integer);
		return;
		}
	}	
}

/*
=================
Chicken Run

Originally from etPUB (i think)

TODO: Once stats are ported make sure it counts as kill for attacker.
=================
*/
gentity_t *G_FearCheck( gentity_t *ent ) {
	qboolean fear = qfalse;
	gentity_t *attacker = &level.gentities[ent->client->lasthurt_client];

	if(g_chicken.integer && attacker && attacker->client &&
		(level.time - ent->client->lasthurt_time) < g_chicken.integer &&
		attacker->client->sess.sessionTeam != ent->client->sess.sessionTeam &&
		attacker->health > 0) {

		fear = qtrue;
	}

	return (fear ? attacker : NULL);
}

/*
===========
Global sound
===========
*/
void APSound(char *sound){
	gentity_t *ent;
	gentity_t *te;

	ent = g_entities;

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
	te->s.eventParm = G_SoundIndex(sound);  
	te->r.svFlags |= SVF_BROADCAST;
}

/*
===========
Client sound
===========
*/
void CPSound(gentity_t *ent, char *sound){
	gentity_t *te;	

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND );
	te->s.eventParm = G_SoundIndex(sound);
	te->s.teamNum = ent->s.clientNum;
}

/*
===========
Global sound with limited range
===========
*/
void APRSound(gentity_t *ent, char *sound){
	gentity_t   *te;

	te = G_TempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = G_SoundIndex(sound);
}

/*
=================
AutoBalanceTeam

Checks if any team needs balancing
=================
*/
void checkEvenTeams( void ) {
	if (!g_teamAutoBalance.integer ||		
		(g_gamestate.integer != GS_PLAYING) ||		
		!g_teamForceBalance.integer ||	
		g_needBalance.integer)
	return;

	sortedActivePlayers();

	if ((level.alliedPlayers - level.axisPlayers) > 1){
		trap_Cvar_Set( "g_needBalance", "1" );		
		AP("chat \"console: ^1Axis ^7team will be balanced in ^115^7 seconds.\n\"");
		level.balanceTimer = level.time + 15000;
	}

	if ((level.axisPlayers - level.alliedPlayers) > 1) {
		trap_Cvar_Set( "g_needBalance", "1" );		
		AP("chat \"console: ^4Allied ^7team will be balanced in ^415^7 seconds.\n\"");
		level.balanceTimer = level.time + 15000;	
	}
}

void balanceTeams( void ) {
	int lowScoreClient = -1;
	int lowScore=0;
	int i;

	while ( ((level.axisPlayers - level.alliedPlayers) > 1) || 
			((level.alliedPlayers - level.axisPlayers) > 1) ) {

		sortedActivePlayers();

		lowScoreClient = -1;

		if ((level.axisPlayers - level.alliedPlayers) > 1) {

			for (i=0; i < level.maxclients; i++) {
				if (level.clients[i].pers.connected != CON_CONNECTED)
					continue;
				if (level.clients[i].sess.sessionTeam != TEAM_RED)
					continue;

				if (lowScoreClient == -1) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];

				} else if (level.clients[i].ps.persistant[PERS_SCORE] < lowScore) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];
				}
			}

			SetTeam( &g_entities[lowScoreClient], "b", qtrue);			
			AP(va("chat \"console: %s ^7was forced to ^4Allies ^7to balance the teams.\n\"",
				level.clients[lowScoreClient].pers.netname));
		}

		if ((level.alliedPlayers - level.axisPlayers) > 1) {

			for (i=0; i < level.maxclients; i++) {
				if (level.clients[i].pers.connected != CON_CONNECTED)
					continue;
				if (level.clients[i].sess.sessionTeam != TEAM_BLUE)
					continue;

				if (lowScoreClient == -1) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];

				} else if (level.clients[i].ps.persistant[PERS_SCORE] < lowScore) {
					lowScoreClient = i;
					lowScore = level.clients[i].ps.persistant[PERS_SCORE];
				}
			}

			SetTeam( &g_entities[lowScoreClient], "r", qtrue);			
			AP(va("chat \"console: %s ^7was forced to ^1Axis ^7to balance the teams.\n\"", 
				level.clients[lowScoreClient].pers.netname));
		}
	}
	trap_Cvar_Set( "g_needBalance", "0" );
}

/*
=================
Countdown 

Causes some troubles on client side so done it here.
=================
*/
void CountDown( void ) {
	char *index="";

	if (level.cnStarted == qfalse) {
		return;
	}
		// Countdown...
	if (level.cnNum == 0) 
		{ index = "prepare.wav"; AP( "cp \"Prepare to fight^2!\n\"2" );}	
	if (level.cnNum == 1) 
		{ index = "cn_5.wav"; AP( "cp \"Match starts in: ^25\n\"2" );}
	if (level.cnNum == 2) 
		{ index = "cn_4.wav"; AP( "cp \"Match starts in: ^24\n\"2" );}	
	if (level.cnNum == 3) 
		{ index = "cn_3.wav"; AP( "cp \"Match starts in: ^23\n\"2" );}	
	if (level.cnNum == 4)
		{ index = "cn_2.wav"; AP( "cp \"Match starts in: ^22\n\"2" );}
	if (level.cnNum == 5) 
		{ index = "cn_1.wav"; AP( "cp \"Match starts in: ^21\n\"2" );}
	if (level.cnNum == 6 ) 
		{ index = "fight.wav"; AP( "print \"^2Fight!\n\""); }	

	// Prepare to fight takes 2 seconds..
	if(level.cnNum == 0){
		level.cnPush = level.time+2000;
	// Just enough to fix the bug and skip to action..
	} else if (level.cnNum == 6) {
		level.cnPush = level.time+200;
	// Otherwise, 1 second.
	} else {
		level.cnPush = level.time+1000;  
	} 
	
	// We're done.. restart the game
	if (level.cnNum == 7) {
		
		level.warmupTime += 10000;
		trap_Cvar_Set( "g_restarted", "1" );
		trap_SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;			
		
		return;
	}
		
	if (level.clients->pers.connected == CON_CONNECTED) 
		APS(va("xmod/sound/scenaric/countdown/%s", index));

	level.cnNum++; 
}
