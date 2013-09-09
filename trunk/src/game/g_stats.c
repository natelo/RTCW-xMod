/*
===========================================================================
g_stats.c

Mostly eye candy stuff.

Author: Nate 'L0
Created: 9.Sept/13
Updated:
===========================================================================
*/
#include "g_local.h"
#include "g_stats.h"

/*
===========
Set time so it's more accessible..
===========
*/
extern int trap_RealTime ( qtime_t * qtime );
const char *dMonths[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*
===========
Double+ kills
===========
*/
void stats_DoubleKill (gentity_t *ent, int meansOfDeath ) {
	char *random;	
	int n = rand() % 3; 

	if (!g_doubleKills.integer) {
		return;
	}
	if(!ent || !ent->client) {
		return;
	}

	// White list approach makes more sense.
	if ( meansOfDeath == MOD_LUGER				// handgun
		|| meansOfDeath == MOD_COLT				// handgun
		|| meansOfDeath == MOD_KNIFE_STEALTH	// knife -- can be done :)
		|| meansOfDeath == MOD_THROWKNIFE		// Kill & a knive throw etc
		|| meansOfDeath == MOD_THOMPSON			// Thompson
		|| meansOfDeath == MOD_MP40				// MP40
		|| meansOfDeath == MOD_STEN				// STEN 
		|| meansOfDeath == MOD_POISONDMED		// Poison & kill etc
	) {	
	
		if((level.time - ent->client->lastKillTime) > 1000) {
			ent->client->doublekill = 0;
			ent->client->lastKillTime = level.time;
			return;
		} else {
			ent->client->doublekill++;
			ent->client->lastKillTime = level.time;
		}

		switch ( ent->client->doublekill ) {
			// 2 kills
			case 1: 				
				if (n == 0) random = "doublekill.wav";
				else if (n == 1) random = "doublekill2.wav";
				else if (n == 2) random = "doublekill3.wav";
				else random = "doublekill.wav";

				APS(va("xmod/sound/game/sprees/doubleKills/%s", random));
				AP(va("chat \"^3Double Kill! ^7%s\n\"", ent->client->pers.netname));
			break;
				// 3 kills
			case 2:
				APS("xmod/sound/game/sprees/doubleKills/tripplekill.wav");
				AP(va("chat \"^3Tripple Kill! ^7%s\n\"", ent->client->pers.netname));
			break;
			// 4 kills
			case 3: 
				APS("xmod/sound/game/sprees/doubleKills/oneandonly.wav");
				AP(va("chat \"^3Pure Ownage! ^7%s\n\"", ent->client->pers.netname));
			break;
		}
	}
}

/*
===========
First headshots 

Prints who done first headshots when round starts.
===========
*/
void stats_FirstHeadshot (gentity_t *attacker, gentity_t *targ) {
	qboolean 	onSameTeam = OnSameTeam( targ, attacker);

	if (g_showFirstHeadshot.integer) {

		if ( !firstheadshot &&
			targ &&
			targ->client &&
			attacker &&
			attacker->client &&
			attacker->s.number != ENTITYNUM_NONE &&
			attacker->s.number != ENTITYNUM_WORLD &&
			attacker != targ &&
			g_gamestate.integer == GS_PLAYING &&
			!onSameTeam )
		{
			AP(va("chat \"%s ^7blew out %s^7's brains with the ^3FIRST HEAD SHOT^7!\"", attacker->client->pers.netname, targ->client->pers.netname));
			APS("xmod/sound/game/events/headshot.wav");				
			firstheadshot = qtrue;
		}
	} 
}

/*
===========
First blood 

Prints who draw first blood when round starts.
NOTE: Atm it's only a print..once I'm not lazy I'll set it in a way it can decide winner once timelimit hits on 
	  specific maps (like depot, destuction) - so first blood decides who won.
===========
*/
void stats_FirstBlood (gentity_t *self, gentity_t *attacker) {
	qboolean 	onSameTeam = OnSameTeam( self, attacker); 

	if (g_showFirstBlood.integer) {

		if (! firstblood &&
			self &&
			self->client &&
			attacker &&
			attacker->client &&
			attacker->s.number != ENTITYNUM_NONE &&
			attacker->s.number != ENTITYNUM_WORLD &&
			attacker != self &&
			g_gamestate.integer == GS_PLAYING &&
			!onSameTeam)
		{	
			AP(va("chat \"%s ^7drew ^1FIRST BLOOD ^7from ^7%s^1!\"", attacker->client->pers.netname, self->client->pers.netname));
			APS("xmod/sound/game/events/firstblood.wav");
			firstblood = qtrue;
		}
	}
}

/*
===========
Killing sprees
===========
*/
void stats_KillingSprees ( gentity_t *ent, int score ) {
	int killRatio = ent->client->pers.kills; 	
	int snd_idx;
	
	if (!g_killingSprees.integer) 
		return;
	
	// if killer ratio is bellow 100 kills spam every 5th kill
	if (killRatio <= 100 && killRatio >= 5 && (killRatio % 5) == 0 ) 	{	
		snd_idx = (killRatio / 5) - 1;

		AP(va("chat \"^4%s ^4(^7%dK %dhs^4): ^7%s\n\"", 
			killingSprees[snd_idx <= 20 ? snd_idx : 19].msg, killRatio, ent->client->pers.headshots, ent->client->pers.netname));

		APS(va("xmod/sound/game/sprees/Sprees/%s", killingSprees[snd_idx < 20 ? snd_idx : 19].snd));
	}	
	// Anything above 100 gets spammed each 10th time..
	else if ( killRatio > 100 && killRatio >= 10 && (killRatio % 10) == 0 ) {
		snd_idx = (killRatio / 10) - 1;

		AP(va("chat \"^4HOLY SHIT ^4(^7%dK %dhs^4): ^7%s\n\"", killRatio, ent->client->pers.headshots, ent->client->pers.netname));		
		APS("xmod/sound/game/sprees/Sprees/holyshit_alt.wav");
	}

	// could be done some other way but anyway...do the count... :)
	ent->client->ps.persistant[PERS_SCORE] += score;
	if (g_gametype.integer >= GT_TEAM)			
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;

	CalculateRanks(); 
}