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

