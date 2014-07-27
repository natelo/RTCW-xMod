#include "g_local.h"

//
// Exporters
//
extern global_userList_t userStats[];
extern global_MODs_t globalMODs[];
extern global_killList_t killList[];
extern g_globalRoundStats_t globalRoundStats[];

global_userList_t userStats[MAX_CLIENTS];
global_MODs_t globalMODs[MAX_CLIENTS];
global_killList_t killList[MAX_CLIENTS];
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
		return qfalse;;
	}
	return qtrue;
}

/*
============
Dumps user stats into a structure..

NOTE: This is also used during a match but it's wiped when round ends..
============
*/
void buildUserStats( int entry, int clientNum ) {
	gentity_t *ent;

	ent = &g_entities[clientNum];
	
	// Build General info
	Q_strncpyz(userStats[entry].ip, 
		va("%d.%d.%d.%d", 
			ent->client->sess.ip[0], 
			ent->client->sess.ip[1], 
			ent->client->sess.ip[2], 
			ent->client->sess.ip[3]), 
			sizeof(userStats[entry].ip));
	Q_strncpyz(userStats[entry].name, ent->client->pers.netname, sizeof(userStats[clientNum].name));
	userStats[entry].clientClass = ent->client->ps.stats[STAT_PLAYER_CLASS];
	userStats[entry].team = ent->client->sess.sessionTeam;
	userStats[entry].ping = ent->client->ps.ping;
	
	// Build Stats
	userStats[entry].stats[GLOBAL_KILLS].value = ent->client->pers.kills;
	userStats[entry].stats[GLOBAL_DEATHS].value = ent->client->pers.deaths;
	userStats[entry].stats[GLOBAL_HEADSHOTS].value = ent->client->pers.headshots;
	userStats[entry].stats[GLOBAL_TEAMKILLS].value = ent->client->pers.teamKills;
	userStats[entry].stats[GLOBAL_TEAMBLEED].value = ent->client->pers.dmgTeam;
	userStats[entry].stats[GLOBAL_POISON].value = ent->client->pers.poison;
	userStats[entry].stats[GLOBAL_REVIVES].value = ent->client->pers.revives;
	userStats[entry].stats[GLOBAL_AMMOGIVEN].value = ent->client->pers.ammoPacks;
	userStats[entry].stats[GLOBAL_MEDGIVEN].value = ent->client->pers.medPacks;
	userStats[entry].stats[GLOBAL_GIBS].value = ent->client->pers.gibs;
	userStats[entry].stats[GLOBAL_SUICIDES].value = ent->client->pers.suicides;
	userStats[entry].stats[GLOBAL_GOOMBAS].value = ent->client->pers.goomba;
	userStats[entry].stats[GLOBAL_KNIFETHROW].value = ent->client->pers.knifeKills;
	userStats[entry].stats[GLOBAL_KNIFE].value = ent->client->pers.stabs;
	userStats[entry].stats[GLOBAL_KNIFE_STEALTH].value = ent->client->pers.fastStabs;
	userStats[entry].stats[GLOBAL_KILLPEAK].value = ent->client->pers.lifeKillsPeak;
	userStats[entry].stats[GLOBAL_DEATHPEAK].value = ent->client->pers.lifeDeathsPeak;
	userStats[entry].stats[GLOBAL_SHOTSFIRED].value = ent->client->pers.acc_shots;
	userStats[entry].stats[GLOBAL_SHOTSHIT].value = ent->client->pers.acc_hits;
	userStats[entry].stats[GLOBAL_DYNOPLANTED].value = ent->client->pers.dynoPlanted;
	userStats[entry].stats[GLOBAL_DYNODISARMED].value = ent->client->pers.dynoDisarmed;
	userStats[entry].stats[GLOBAL_MGREPAIRED].value = ent->client->pers.mgRepair;
	userStats[entry].stats[GLOBAL_ASCALLED].value = ent->client->pers.AScalled;
	userStats[entry].stats[GLOBAL_ASTHROWN].value = ent->client->pers.ASthrown;
	userStats[entry].stats[GLOBAL_ASBLOCKED].value = ent->client->pers.ASblocked;
	userStats[entry].stats[GLOBAL_DMG_GIVEN].value = ent->client->pers.dmgGiven;
	userStats[entry].stats[GLOBAL_DMG_RECEIVED].value = ent->client->pers.dmgReceived;
	userStats[entry].stats[GLOBAL_SCORE].value = ent->client->ps.persistant[PERS_SCORE];
	userStats[entry].stats[GLOBAL_CHICKEN].value = ent->client->pers.chicken;
}

/*
============
Fills stats structure so it can be processed..
============
*/
void parseClientStats( void ) {
	gclient_t *cl;
	int i;

	// Make sure there's no 'leftovers'
	memset(userStats, 0, sizeof(userStats));

	for (i = 0; i < level.numConnectedClients; i++)	{
		cl = level.clients + level.sortedClients[i];
		
		if (cl->pers.connected == CON_CONNECTED)
			buildUserStats(i, cl->ps.clientNum);
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
void parseRoundStats( qboolean finished ) {
	char mapName[64];

	// Make sure there's no 'leftovers'
	memset(globalRoundStats, 0, sizeof(globalRoundStats));

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));
	
	globalRoundStats[0].winner = level.winningTeam;
	globalRoundStats[0].map = va("%s", mapName);
	globalRoundStats[0].time = g_nextTimeLimit.value;
	globalRoundStats[0].round = g_currentRound.integer;
	globalRoundStats[0].gametype = g_gametype.integer;
	globalRoundStats[0].altGametype = altGameType();
	Q_strncpyz(globalRoundStats[0].firstBloodAttacker, level.firstBloodAttacker, sizeof(globalRoundStats[0].firstBloodAttacker));
	Q_strncpyz(globalRoundStats[0].firstBloodVictim, level.firstBloodVictim, sizeof(globalRoundStats[0].firstBloodVictim));
	Q_strncpyz(globalRoundStats[0].firstHeadshotAttacker, level.firstHeadshotAttacker, sizeof(globalRoundStats[0].firstHeadshotAttacker));
	Q_strncpyz(globalRoundStats[0].firstHeadshotVictim, level.firstHeadshotVictim, sizeof(globalRoundStats[0].firstHeadshotVictim));
	Q_strncpyz(globalRoundStats[0].lastBloodAttacker, level.lastBloodAttacker, sizeof(globalRoundStats[0].lastBloodAttacker));
	Q_strncpyz(globalRoundStats[0].lastBloodVictim, level.lastBloodVictim, sizeof(globalRoundStats[0].lastBloodVictim));
	globalRoundStats[0].finishedRound = finished;
}

/*
============
Convert MODs
============
*/
statsMODs MODtoStats(meansOfDeath_t mod) {

	switch ( mod ) {
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
		case MOD_THROWKNIFE:
			return STATS_KNIFETHROW;
		case MOD_AIRSTRIKE:
			return STATS_AIRSTRIKE;
		case MOD_ARTILLERY:
			return STATS_ARTILLERY;
		case MOD_POISONDMED:
			return STATS_POISON;
		case MOD_GOOMBA:
			return STATS_GOOMBA;
		case MOD_FALLING:
			return STATS_FALLING;
		case MOD_MORTAR_SPLASH:
			return STATS_MORTAR;
		case MOD_SELFKILL:
			return STATS_SUICIDE;
		case MOD_CHICKEN:
			return STATS_CHICKEN;
		case MOD_WATER:
			return STATS_DROWN;
		case MOD_TRIGGER_HURT:
			return STATS_WORLD;
		case MOD_CRUSH:
			return STATS_WORLD;
		case MOD_ADMKILL:
			return STATS_ADMIN;			
		default:
			return STATS_MAX;
	}
}

/*
============
Store client's MOD (means of death)
============
*/
void write_globalMODs(gentity_t *victim, meansOfDeath_t MOD) {
	int client = victim->client->ps.clientNum;

	if (!webStatsAreEnabled())
		return;

	// Convert MOD
	MOD = MODtoStats(MOD);

	// Just blindly write in..
	if (MOD != STATS_MAX) {
		if (globalMODs[client].mod[MOD].count > 0)
			globalMODs[client].mod[MOD].count = globalMODs[client].mod[MOD].count + 1;
		else
			globalMODs[client].mod[MOD].count = 1;
	}
}		

/*
============
Store Client's "hit list"

We store guid of victim as if we store slot, client may be long gone before round ends
so this guid makes sure correct data is sent to a web server and no data is lost.

Note: 
When player leaves, data is send to a server so we don't lose the track of it.
In case if client rejoins, it resets the count (token check) so data is not multiplied.
============
*/
void write_globalKillList(gentity_t *victim, gentity_t *attacker) {
	int killer = attacker->client->ps.clientNum;
	int target = victim->client->ps.clientNum;

	if (!webStatsAreEnabled())
		return;

	// Check if there's already a match
	if (killList[killer].victim[target].guid == victim->client->sess.guid) {
		// Check if client rejoined
		if (killList[killer].victim[target].token == victim->client->pers.uniqueToken) {
			killList[killer].victim[target].count = killList[killer].victim[target].count + 1;
		}
		else {
			killList[killer].victim[target].count = 1;			
			killList[killer].victim[target].token = victim->client->pers.uniqueToken;	// Update token
		}
	} 
	else { 
		// Create a new entry - 
		// Note: No worries if we reset it, as leaving player already fired a packet with data to a web server..
		killList[killer].victim[target].count = 1;
		killList[killer].victim[target].token = victim->client->pers.uniqueToken;
		Q_strncpyz(killList[killer].victim[target].guid, victim->client->sess.guid, sizeof(killList[killer].victim[target].guid));
	}
}

/*
============
Builds data and fires a packet to a web server
============
*/
void *sendGlobalStats(void *args) {	
	global_Stats_t *globalStats = (global_Stats_t*)args;
	//g_http_cmd_t *globalStats = (g_http_cmd_t*)args;
	char *round = NULL;
	char *players = NULL;
	char *mods = NULL;
	char *killList = NULL;
	char *data = NULL;	

	// Round Info
	if (globalStats->roundStats.finishedRound == qtrue)	
	/*
		round = va(
			"roundInfo||%d\\%s\\%f\\%i\\%i\\%i\\%s\\%s\\%s\\%s\\%s\\%s",
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
	*/
		round = va("roundInfo||%s", globalStats->roundStats.map);		
	else
		round = "roundInfo||null";

	// Build Stats
	data = va(
		"data=%s||",
		round
	);

	// Send it now
	if (g_httpDebug.integer)
		AP("print \"g_httpDebug : Sending global round stats.\n\"");

	http_Submit(g_httpStatsUrl.string, data);
	
	
	free(globalStats);	
	return 0;
}

/*
============
Utilizes stuff and sets thing in motion
============
*/
void prepGlobalStats( void ) {	
	global_Stats_t *globalStats = (global_Stats_t *)malloc(sizeof(global_Stats_t));
	//g_http_cmd_t *globalStats = (g_http_cmd_t*)malloc(sizeof(g_http_cmd_t));

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

	// Build Stats now
	parseClientStats();
	parseRoundStats(finished);

	// Set things in motion
	prepGlobalStats();
}

void cleanGlobalStats() {

	memset(userStats, 0, sizeof(userStats));
	memset(globalRoundStats, 0, sizeof(globalRoundStats));
	memset(globalMODs, 0, sizeof(globalMODs));
	memset(killList, 0, sizeof(killList));
}
