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

static qboolean firstheadshot;						
static qboolean firstblood;	

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


