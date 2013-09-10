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

/*
===========
Death spree
===========
*/
void stats_DeathSpree ( gentity_t *ent ) {	
	int deaths = ent->client->pers.spreeDeaths; 
	int n = rand() % 2; 
	char *snd="", *spree="";	

	if (!g_deathSprees.integer || deaths <= 0)
		return;

	if( deaths == 9 ) { 
		if (n == 0) { spree=va("(^710 Dth^0)"); snd = "dSpree1.wav"; }
		else { spree=va("(^710 Dth^0)"); snd = "dSpree1_alt.wav"; }		
	} else if( deaths == 14 ) { 
		if (n == 0) { spree=va("(^715 Dth^0)"); snd = "dSpree2.wav"; }
		else { spree=va("(^715 Dth^0)"); snd = "dSpree2_alt.wav"; }		
	} else if( deaths == 19 ) { 
		if (n == 0) { spree=va("(^720 Dth^0)"); snd = "dSpree3.wav"; }
		else { spree=va("(^720 Dth^0)"); snd = "dSpree3_alt.wav";  }		
	} else if( deaths == 24 ) { 
		if (n == 0) { spree=va("(^725 Dth^0)"); snd = "dSpree4.wav"; }
		else { spree=va("(^725 Dth^0)"); snd = "dSpree4_alt.wav"; }		
	}

	// Has to be offset by 1 as count comes late..
	if (deaths == 9 || deaths == 14 || deaths == 19 || deaths == 24) {
		AP(va("chat \"^0DEATHSPREE! %s: ^7%s\n\"", spree, ent->client->pers.netname));
		
		APS(va("xmod/sound/game/sprees/deathSpree/%s", snd));
	}
} 

/*
===========
Killer spree

Almost identical to Killing sprees, just uses different colors and female sounds.
===========
*/
void stats_KillerSpree(gentity_t *ent, int score) {
	int killRatio=ent->client->pers.lifeKills;
	int snd_idx;

	if(!g_killerSpree.integer)
		return;

	if(!ent || !ent->client) 
		return;	

	// if killer ratio is bellow 50 kills spam every 5th kill
	if (killRatio <= 50 && killRatio >= 5 && (killRatio % 5) == 0 ) 	{	
		snd_idx = (killRatio / 5) - 1;

		AP(va("chat \"^2%s ^2(^7%dk^2): ^7%s\n\"", 
			killerSprees[snd_idx <= 11 ? snd_idx : 10].msg, killRatio, ent->client->pers.netname));
				
		APS(va("xmod/sound/game/sprees/killerSprees/%s", killerSprees[snd_idx < 11 ? snd_idx : 10].snd));
	}

	// Anything above 50 gets spammed each 10th time..
	else if ( killRatio > 50 && killRatio >= 10 && (killRatio % 10) == 0 ) {
		snd_idx = (killRatio / 10) - 1;

		AP(va("chat \"^2%s ^2(^7%dk^2): ^7%s\n\"", 
			killerSprees[snd_idx <= 11 ? snd_idx : 10].msg, killRatio, ent->client->pers.netname));

		APS(va("xmod/sound/game/sprees/killerSprees/%s", killerSprees[snd_idx < 11 ? snd_idx : 10].snd ));
	}

	// could be done some other way but anyway...do the count... :)
	ent->client->ps.persistant[PERS_SCORE] += score;
	if (g_gametype.integer >= GT_TEAM)			
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;

	CalculateRanks(); 	
}

/*
===========
EOM (End of Match) stats
===========
*/
void stats_MatchInfo( void ) {
	int i, j, cnt, eff;
	float tot_acc = 0.00f;
	int tot_kills, tot_deaths, tot_gp, tot_hs, tot_sui, tot_tk, tot_dg, tot_dr, tot_td, tot_hits, tot_shots;
	gclient_t *cl;
	char *ref;
	char n1[MAX_NETNAME];
	char n2[MAX_NETNAME];
	

	qtime_t ct;
	trap_RealTime(&ct);
	AP(va("print \"\nMod: %s \n^7Server: %s  \n^7Time: ^7%02d:%02d:%02d ^3(^7%02d %s %d^3)\n\n\"", 
			GAMEVERSION, sv_hostname.string, ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_mday, dMonths[ct.tm_mon], 1900+ct.tm_year));

	cnt = 0;
	for ( i = TEAM_RED; i <= TEAM_BLUE; i++ ) {
		if ( !TeamCount( -1, i ) ) {
			continue;
		}

		tot_kills = 0;
		tot_deaths = 0;
		tot_hs = 0;
		tot_sui = 0;
		tot_tk = 0;
		tot_dg = 0;
		tot_dr = 0;
		tot_td = 0;
		tot_gp = 0;
		tot_hits = 0;
		tot_shots = 0;
		tot_acc = 0;

		AP( va("print \"%s ^7Team\n"
			     "^7----------------------------------------------------------------------"
				 "\nPlayer          ^3Kll ^7Dth Sui TK ^5Eff ^7Accrcy   ^2HS    DG    DR   TD  ^7Score\n"
				 "^7----------------------------------------------------------------------\n\"", (i == TEAM_RED) ? "^1Axis" : "^4Allied"  ));

		for ( j = 0; j < level.numPlayingClients; j++ ) {
			cl = level.clients + level.sortedClients[j];			

			if ( cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam != i ) {
				continue;
			}

			// Bug fix - ^Pentagram always manages to break stats so it needs different approach. ^^
			DecolorString(cl->pers.netname, n1);
			SanitizeString(n1, n2, qfalse); 
			Q_CleanStr(n2);				
			n2[15] = 0;

			ref = "^7";
			tot_kills += cl->pers.kills;
			tot_deaths += cl->pers.deaths;			
			tot_sui += cl->pers.suicides;
			tot_tk += cl->pers.teamKills;
			tot_hs += cl->pers.headshots;
			tot_dg += cl->pers.dmgGiven;
			tot_dr += cl->pers.dmgReceived;
			tot_td += cl->pers.dmgTeam;	
			tot_gp += cl->ps.persistant[PERS_SCORE];						
			tot_hits += cl->pers.acc_hits;
			tot_shots += cl->pers.acc_shots;

			eff = ( cl->pers.deaths + cl->pers.kills == 0 ) ? 0 : 100 * cl->pers.kills / ( cl->pers.deaths + cl->pers.kills );
			if ( eff < 0 ) {
				eff = 0;
			}	

			cnt++;
			AP( va( "print \"%s%-15s^3%4d^7%4d%4d%3d%s^5%4d ^7%6.2f^2%5d%6d%6d%5d^7%7d\n\"",					
					ref,
					n2,
					cl->pers.kills,
					cl->pers.deaths,					
					cl->pers.suicides,
					cl->pers.teamKills,
					ref,
					eff,						
					( (cl->pers.acc_shots == 0) ? 0.00 : ((float)cl->pers.acc_hits / (float)cl->pers.acc_shots ) * 100.00f ),
					cl->pers.headshots,
					cl->pers.dmgGiven,
					cl->pers.dmgReceived,
					cl->pers.dmgTeam,
					cl->ps.persistant[PERS_SCORE] ) );
		}

		eff = ( tot_kills + tot_deaths == 0 ) ? 0 : 100 * tot_kills / ( tot_kills + tot_deaths );
		if ( eff < 0 ) {
			eff = 0;
		}
		tot_acc = ( (tot_shots == 0) ? 0.00 : ((float)tot_hits / (float)tot_shots ) * 100.00f );

		AP( va( "print \"^7----------------------------------------------------------------------\n"
				"%-19s^3%4d^7%4d%4d%3d^5%4d ^7%6.2f^2%5d%6d%6d%5d^7%7d\n\n\n\"",				
				"^3Totals^7",
				tot_kills,
				tot_deaths,				
				tot_sui,
				tot_tk,
				eff,
				tot_acc,
				tot_hs,
				tot_dg,
				tot_dr,
				tot_td,
				tot_gp ) );
	}
	AP( va( "print \"%s\n\" 0", ( ( !cnt ) ? "^3\nNo scores to report." : "" ) ) );
}
