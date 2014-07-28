/*
===========================================================================
L0 - g_http_stats.c
All essential WebStats functionality resides here.

Created: 23.07 / 14
Last Updated: 28.07 / 14
===========================================================================
*/
#include "g_local.h"

//
// Exporters
//
extern global_entryList_t globalEntryList[];
extern global_userList_t globalUserStats[];
extern global_MODs_t globalMODs[];
extern g_globalRoundStats_t globalRoundStats[];

global_entryList_t globalEntryList[1];
global_userList_t globalUserStats[MAX_CLIENTS];
global_MODs_t globalMODs[MAX_CLIENTS];
g_globalRoundStats_t globalRoundStats[1];

/*
============
Checks if WebStats are enabled on a server or not..
============
*/
qboolean webStatsAreEnabled(void) {

	if (!g_httpStatsUrl.string ||
		!Q_stricmp(g_httpStatsAPI.string, "") ||
		!g_httpToken.string ||
		!Q_stricmp(g_httpToken.string, "none") ||
		(!g_gamestate.integer == GS_PLAYING))
	{
		return qfalse;
	}
	return qtrue;
}

/*
============
Convert MODs
============
*/
statsMODs MODtoStats(meansOfDeath_t mod) {

	switch (mod) {
	case MOD_MP40:
		return STATS_MP40;
	case MOD_THOMPSON:
		return STATS_THOMPSON;
	case MOD_STEN:
		return STATS_STEN;
	case MOD_MAUSER:
		return STATS_MAUSER;
	case MOD_SNIPERRIFLE:
		return STATS_SNIPERRIFLE;
	case MOD_FLAMETHROWER:
		return STATS_FLAMETHROWER;
	case MOD_ROCKET_SPLASH:
		return STATS_PANZERFRAUST;
	case MOD_VENOM:
		return STATS_VENOM;
	case MOD_GRENADE_PINEAPPLE:
		return STATS_GRENADE;
	case MOD_LUGER:
		return STATS_LUGER;
	case MOD_COLT:
		return STATS_COLT;
	case MOD_DYNAMITE_SPLASH:
		return STATS_DYNAMITE;
	case MOD_MACHINEGUN:
		return STATS_MG42;
	case MOD_KNIFE2:
		return STATS_KNIFE;
	case MOD_KNIFE_STEALTH:
		return STATS_KNIFESTEALTH;
	case MOD_KNIFETHROW:
		return STATS_KNIFETHROW;
	case MOD_AIRSTRIKE:
		return STATS_AIRSTRIKE;
	case MOD_ARTILLERY:
		return STATS_ARTILLERY;
	case MOD_POISONED:
		return STATS_POISON;
	case MOD_GOOMBA:
		return STATS_GOOMBA;
	case MOD_FALLING:
		return STATS_FALLING;
	case MOD_MORTAR_SPLASH:
		return STATS_MORTAR;
	case MOD_SELFKILL:
		return STATS_SUICIDE;
	case MOD_SUICIDE:
		return STATS_SUICIDE;
	case MOD_CHICKEN:
		return STATS_CHICKEN;
	case MOD_WATER:
		return STATS_DROWN;
	case MOD_TRIGGER_HURT:
		return STATS_WORLD;
	case MOD_CRUSH:
		return STATS_WORLD;
	case MOD_ADMIN:
		return STATS_ADMIN;
	default:
		return STATS_MAX;
	}
}

/*
============
Store client's MOD (means of death)

NOTE: 
- Called from g_combat.c
- Main structure resides in g_local.h
============
*/
void write_globalMODs(gentity_t *victim, meansOfDeath_t MOD) {
	
	if (!webStatsAreEnabled())
		return;
		
	MOD = MODtoStats(MOD);
	if (MOD < STATS_MAX) {
		victim->client->pers.MODs[MOD].count = victim->client->pers.MODs[MOD].count + 1;
	}
	return;
}

/*
============
Store Client's "hit list"
NOTE: Called from g_combat.c

We store guid of killer as if we store slot, client may be long gone before round ends
so this guid makes sure correct data is sent to a web server and no data is lost.

Note:
When player leaves, data is send to a server so we don't lose the track of it.
In case if client rejoins count is increased otherwise it's resetted.
============
*/
void write_globalKillList(gentity_t *victim, gentity_t *attacker) {
	int killer = attacker->client->ps.clientNum;	

	if (!webStatsAreEnabled())
		return;

	// Figure out if we already have an entry
	if (!Q_stricmp(victim->client->pers.killerList[killer].guid, attacker->client->sess.guid)) {
		victim->client->pers.killerList[killer].count = victim->client->pers.killerList[killer].count + 1;
	}
	// We do not care about attacker since he fired a packet to a server when he left..
	// NOTE: Probably prone to data loss if client forcefully leaves (crash..) but it needs further testing as structure may not be wiped yet.
	else {		
		Q_strncpyz(victim->client->pers.killerList[killer].guid, attacker->client->sess.guid, sizeof(victim->client->pers.killerList[killer].guid));
		victim->client->pers.killerList[killer].count = 1;
	}
	return;
}

/*
============
Remaps stats 
============
*/
int remapStats(int type, gentity_t *ent) {

	switch ( type ) {
		case GLOBAL_KILLS:			return ent->client->pers.kills;
		case GLOBAL_DEATHS:			return ent->client->pers.deaths;
		case GLOBAL_HEADSHOTS:		return ent->client->pers.headshots;
		case GLOBAL_TEAMKILLS:		return ent->client->pers.teamKills;
		case GLOBAL_TEAMBLEED:		return ent->client->pers.dmgTeam;
		case GLOBAL_POISON:			return ent->client->pers.poison;
		case GLOBAL_REVIVES:		return ent->client->pers.revives;
		case GLOBAL_AMMOGIVEN:		return ent->client->pers.ammoPacks;
		case GLOBAL_MEDGIVEN:		return ent->client->pers.medPacks;
		case GLOBAL_GIBS:			return ent->client->pers.gibs;
		case GLOBAL_SUICIDES:		return ent->client->pers.suicides;
		case GLOBAL_GOOMBAS:		return ent->client->pers.goomba;
		case GLOBAL_KNIFETHROW:		return ent->client->pers.knifeKills;
		case GLOBAL_KNIFE:			return ent->client->pers.stabs;
		case GLOBAL_KNIFE_STEALTH:	return ent->client->pers.fastStabs;
		case GLOBAL_KILLPEAK:		return ent->client->pers.lifeKillsPeak;
		case GLOBAL_DEATHPEAK:		return ent->client->pers.lifeDeathsPeak;
		case GLOBAL_SHOTSFIRED:		return ent->client->pers.acc_shots;
		case GLOBAL_SHOTSHIT:		return ent->client->pers.acc_hits;
		case GLOBAL_DYNOPLANTED:	return ent->client->pers.dynoPlanted;
		case GLOBAL_DYNODISARMED:	return ent->client->pers.dynoDisarmed;
		case GLOBAL_MGREPAIRED:		return ent->client->pers.mgRepair;
		case GLOBAL_ASCALLED:		return ent->client->pers.AScalled;
		case GLOBAL_ASTHROWN:		return ent->client->pers.ASthrown;
		case GLOBAL_ASBLOCKED:		return ent->client->pers.ASblocked;
		case GLOBAL_DMG_GIVEN:		return ent->client->pers.dmgGiven;
		case GLOBAL_DMG_RECEIVED:	return ent->client->pers.dmgReceived;
		case GLOBAL_SCORE:			return ent->client->ps.persistant[PERS_SCORE];
		case GLOBAL_CHICKEN:		return ent->client->pers.chicken;
		default:					return 0;
	}
	return 0;
}

/*
============
Dumps user stats into a structure..
============
*/
void buildUserStats(global_Stats_t *clientStats, int entry, int clientNum) {	
	gentity_t *ent;
	int i;

	ent = &g_entities[clientNum];

	// Build General info
	Q_strncpyz(clientStats->players[entry].ip,
		va("%d.%d.%d.%d", 
			ent->client->sess.ip[0], 
			ent->client->sess.ip[1], 
			ent->client->sess.ip[2], 
			ent->client->sess.ip[3]), 
			sizeof(clientStats->players[entry].ip));
	// We replace any & in name so we do not break the POST fields - we'll remap it back on PHP side..
	Q_strncpyz(clientStats->players[entry].name, Q_CharReplace(ent->client->pers.netname, '&', '~'), sizeof(clientStats->players[clientNum].name));
	clientStats->players[entry].clientClass = ent->client->ps.stats[STAT_PLAYER_CLASS];
	clientStats->players[entry].team = ent->client->sess.sessionTeam;
	clientStats->players[entry].ping = ent->client->ps.ping;
	
	// Build Stats
	for (i = 0; i < GLOBAL_LIMIT; i++) {	
		clientStats->players[entry].stats[i].value = remapStats(i, ent);
		clientStats->players[entry].stats[i].label = global_statsTypes[i].label;
	}
}

/*
============
Fills stats structure so it can be processed..
============
*/
void parseClientStats(global_Stats_t *clientStats) {	
	int i=0, \
		j=0;

	for (i = 0; i < MAX_CLIENTS; i++)	{		
		j++;

		if (level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		// Add entry
		buildUserStats(clientStats, j, level.clients[i].ps.clientNum);

		// Store the count
		clientStats->entries.players = j;		
	}
}

/*
============
Get any custom gametype (deathmatch, goldrush..)

Add accordingly..
============
*/
int altGameType( void ) {
	return 0;
}

/*
============
Fills round structure
============
*/
void parseRoundStats(global_Stats_t *roundStats, qboolean finished) {
	char mapName[64];

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));
		
	roundStats->roundStats.winner = level.winningTeam;
	roundStats->roundStats.map = va("%s", mapName);
	roundStats->roundStats.time = g_nextTimeLimit.value;
	roundStats->roundStats.round = g_currentRound.integer;
	roundStats->roundStats.gametype = g_gametype.integer;
	roundStats->roundStats.altGametype = altGameType();
	Q_strncpyz(roundStats->roundStats.firstBloodAttacker, level.firstBloodAttacker, sizeof(roundStats->roundStats.firstBloodAttacker));
	Q_strncpyz(roundStats->roundStats.firstBloodVictim, level.firstBloodVictim, sizeof(roundStats->roundStats.firstBloodVictim));
	Q_strncpyz(roundStats->roundStats.firstHeadshotAttacker, level.firstHeadshotAttacker, sizeof(roundStats->roundStats.firstHeadshotAttacker));
	Q_strncpyz(roundStats->roundStats.firstHeadshotVictim, level.firstHeadshotVictim, sizeof(roundStats->roundStats.firstHeadshotVictim));
	Q_strncpyz(roundStats->roundStats.lastBloodAttacker, level.lastBloodAttacker, sizeof(roundStats->roundStats.lastBloodAttacker));
	Q_strncpyz(roundStats->roundStats.lastBloodVictim, level.lastBloodVictim, sizeof(roundStats->roundStats.lastBloodVictim));
	roundStats->roundStats.finishedRound = finished;
}

/*
============
Fill each field
============
*/
void buildMODsStats(global_Stats_t *modStats, int entry, int cl) {
	gentity_t *ent;
	int i = 0, \
		k = 0;

	ent = &g_entities[cl];
	Q_strncpyz(modStats->mods[entry].guid, ent->client->sess.guid, sizeof(modStats->mods[entry].guid));
	// Build structure
	for (i = 0; i < STATS_MAX; i++) {
		if (ent->client->pers.MODs[i].count > 0) {
			modStats->mods[entry].MODs[k].count = ent->client->pers.MODs[i].count;
			modStats->mods[entry].MODs[k].label = va("%s", global_modTypes[i].label);
		}	
	}
	
}

/*
============
Fills mod structure
============
*/
void parseMODs(global_Stats_t *modStats) {
	int i = 0, \
		j = 0;

	for (i = 0; i < MAX_CLIENTS; i++)	{
		j++;

		if (level.clients[i].pers.connected != CON_CONNECTED)
			continue;

		// Add entry
		buildMODsStats(modStats, j, level.clients[i].ps.clientNum);

		// Store the count
		modStats->entries.MODs = j;
	}
}

/*
============
Builds data and fires a packet to a web server
============
*/
void *sendGlobalStats(void *args) {
	global_Stats_t *globalStats = (global_Stats_t*)args;
	char *round =	"null";
	char *stats =	"null";
	char *mods =	"null";
	char *hitList = "null";
	char *data =	"null";

	//
	// Round Info
	//
	if (globalStats->roundStats.finishedRound == qtrue)
		round = va(
		"%d\\%s\\%3.2f\\%i\\%i\\%i\\%s\\%s\\%s\\%s\\%s\\%s",
		globalStats->roundStats.winner,
		globalStats->roundStats.map,
		globalStats->roundStats.time,
		globalStats->roundStats.round,
		globalStats->roundStats.gametype,
		globalStats->roundStats.altGametype,
		globalStats->roundStats.firstBloodAttacker,
		globalStats->roundStats.firstBloodVictim,
		globalStats->roundStats.firstHeadshotAttacker,
		globalStats->roundStats.firstHeadshotVictim,
		globalStats->roundStats.lastBloodAttacker,
		globalStats->roundStats.lastBloodVictim
	);

	//
	// Stats Info
	//
	if (globalStats->entries.players > 0) {
		int i;
		char *append = "";

		for (i = 1; i <= globalStats->entries.players; i++) {
			int j;			
			// FIXME :: ADD GUID!
			stats = va(
				"%s\\IP:%s\\Name:%s\\Class:%i\\Team:%i\\Ping:%i",
				(!Q_stricmp(stats, "null") ? "" : va("%s\\\\", stats)),
				globalStats->players[i].ip,
				globalStats->players[i].name,
				globalStats->players[i].clientClass,
				globalStats->players[i].team,
				globalStats->players[i].ping
			);

			// Build Stats now
			// Note: Only writes ones that aren't 0..so we keep packet size to bare essential..
			for (j = 0; j < GLOBAL_LIMIT; j++) {
				if (globalStats->players[i].stats[j].value != 0)
					append = va("%s\\%s:%i", append, globalStats->players[i].stats[j].label, globalStats->players[i].stats[j].value);
			}
			stats = va("%s%s", stats, append);
		}
	}

	//
	// MOD Info
	//
	if (globalStats->entries.MODs > 0) {
		int i;
		char *append = "";

		for (i = 1; i <= globalStats->entries.MODs; i++) {
			int j;

			mods = va("%sguid:%s",
				(!Q_stricmp(mods, "null") ? "" : va("%s\\\\", mods)),
				globalStats->mods[i].guid
			);

			// Build Mods now
			for (j = 0; j < STATS_MAX; j++) {
				if (globalStats->mods[i].MODs[j].count > 0)					
					append = va("%s\\%s:%i", append, globalStats->mods[i].MODs[j].label, globalStats->mods[i].MODs[j].count);
			}
			mods = va("%s%s", mods, append);
		}
	}
	
	// Build Stats
	data = va(
		"data=webstats&round=%s&stats=%s&mods=%s",
		round,
		stats,
		mods,
		hitList
	);

	if (g_httpDebug.integer)
		G_Printf("g_httpDebug : Sending global round stats.\n");

	// Send it now
	http_Submit(g_httpStatsUrl.string, data);
	
	free(globalStats);	
	return 0;
}

/*
============
Utilizes stuff and sets things in motion
============
*/
void prepGlobalStats( qboolean finished ) {	
	global_Stats_t *globalStats = (global_Stats_t *)malloc(sizeof(global_Stats_t));

	// Build Stats now
	parseClientStats(globalStats);
	parseRoundStats(globalStats, finished);	
	parseMODs(globalStats);

	// Go for it..
	create_thread(sendGlobalStats, (void*)globalStats);
}

/*
============
Prep's and sends global stats to a Web Server
============
*/
void globalStats(qboolean finished) {

	if (!webStatsAreEnabled())
		return;

	// Set things in motion
	prepGlobalStats(finished);
}

/*
============
Wipes the structures
============
*/
void cleanGlobalStats( void ) {
	memset(globalEntryList, 0, sizeof(globalEntryList));
	memset(globalUserStats, 0, sizeof(globalUserStats));
	memset(globalRoundStats, 0, sizeof(globalRoundStats));
	memset(globalMODs, 0, sizeof(globalMODs));
}
