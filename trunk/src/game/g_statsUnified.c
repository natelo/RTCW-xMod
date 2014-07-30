/*
===========================================================================
L0 - g_http_stats.c
All essential WebStats functionality resides here.

Created: 29.07 / 14
Last Updated: /
===========================================================================
*/
#include "g_local.h"


/*
==============================
stats_addEntry

Linked list
==============================
*/
void stats_addEntry(char* entry) {
	struct statEntry *node;

	node = (struct statEntry *)malloc(sizeof(struct statEntry));
	Q_strncpyz(node->info, entry, sizeof(node->info));

	if (statHead == NULL) {
		statHead = node;
	}
	else {
		statTail->next = node;
	}
	statTail = node;
	node->next = NULL;
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
void globalStats_writeMOD(gentity_t *victim, meansOfDeath_t MOD) {	

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
void globalStats_hitList(gentity_t *victim, gentity_t *attacker) {
	int killer = attacker->client->ps.clientNum;

	victim->client->pers.hitList[killer].count = victim->client->pers.hitList[killer].count + 1;
}

/*
============
Dumps in file that will be later send to web (stats) server
============
*/
void globalStats_dump( void ) {
	struct statEntry *node;
	FILE *statsDump;

	statsDump = fopen("webStats.log", "a+"); 	
	while (statHead != NULL) {
		node = statHead;
		fputs(va("%s\n", node->info), statsDump);
		statHead = statHead->next;
	}
	fclose(statsDump);
}

/*
============
Get any custom gametype (deathmatch, goldrush..)
============
*/
int altGameType(void) {
	return 0;
}

/*
============
Generates unique round token so clients can be remaped..
============
*/
void globalStats_roundToken( void ) {
	if (!level.roundID)
		level.roundID = va("%s", getTime(qtrue));

	stats_addEntry(va("stamp:%s", getTime(qtrue)));
	//globalStats_dump();
}

/*
============
Builds round stats
============
*/
char *buildRound( void ) {
	char mapName[64];

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));

	return va("round\\%s\\%s\\%i\\%i\\%i\\%s\\%s\\%s\\%s\\%s\\%s",
		level.roundID,
		mapName,
		g_currentRound.integer,
		g_gametype.integer,
		altGameType(),
		level.firstBloodAttacker,
		level.firstBloodVictim,
		level.firstHeadshotAttacker,
		level.firstHeadshotVictim,
		level.lastBloodAttacker,
		level.lastBloodVictim
	);
}

/*
============
Builds client hit list (players who killed him)
============
*/
void client_hitList(gentity_t *ent) {
	int i;
	char *data = NULL;

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (ent->client->pers.hitList[i].count > 0)
			data = va("%s\\%d:%d", (!data ? va("hitlist\\%s", ent->client->sess.guid) : data), i, ent->client->pers.hitList[i].count);
	}

	if (data)
		stats_addEntry(data);
}

/*
============
Builds client MODs stats
============
*/
void client_buildMODList(gentity_t *ent) {
	int i;
	char *data=NULL;

	for (i = 0; i < STATS_MAX; i++) {
		if (ent->client->pers.MODs[i].count > 0)
			data = va("%s\\%d:%d", (!data ? va("mods\\%s", ent->client->sess.guid) : data), i, ent->client->pers.MODs[i].count);
	}
	if (data)
		stats_addEntry(data);
}

/*
============
Builds client stats
============
*/
char *client_buildStats(gentity_t *ent) {	
	return va("\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d\\%d",
		ent->client->stats.kills,
		ent->client->stats.deaths,
		ent->client->stats.headshots,
		ent->client->stats.teamKills,
		ent->client->stats.poison,
		ent->client->stats.revives,
		ent->client->stats.revivesRec,
		ent->client->stats.ammoGiv,
		ent->client->stats.medGiv,
		ent->client->stats.medRec,
		ent->client->stats.ammoRec,
		ent->client->stats.gibs,
		ent->client->stats.suicides,
		ent->client->stats.goomba,
		ent->client->stats.knife,
		ent->client->stats.knifeStealth,
		ent->client->stats.knifeThrow,
		ent->client->stats.killPeak,
		ent->client->stats.deathPeak,
		ent->client->stats.shotsFired,
		ent->client->stats.shotsHit,
		ent->client->stats.dynoPlanted,
		ent->client->stats.dynoDisarmed,
		ent->client->stats.mgsRepaired,
		ent->client->stats.ASCalled,
		ent->client->stats.ASThrown,
		ent->client->stats.ASBlocked,
		ent->client->stats.chickenRun,
		ent->client->stats.dmgTeam,
		ent->client->stats.dmgGiv,
		ent->client->stats.dmgRec,
		ent->client->stats.objSteals,
		ent->client->stats.flagCapture,
		ent->client->stats.flagReclaim
	);
}

/*
============
Builds client general info
============
*/
char *client_buildGeneral(gentity_t *ent) {

	return va("client\\%i\\%s\\%s\\%s\\%d\\%d\\%d\\%d",
		ent->client->ps.clientNum,
		ent->client->sess.guid,		
		level.roundID,
		ent->client->pers.netname,
		ent->client->sess.sessionTeam,
		ent->client->ps.stats[STAT_PLAYER_CLASS],
		ent->client->ps.ping,
		ent->client->ps.persistant[PERS_SCORE]
	);
}

/*
============
Builds client data and dumps it in file
============
*/
void stats_clientData(gentity_t *ent) {
	char *data;

	data = va("%s%s",		
		client_buildGeneral(ent),
		client_buildStats(ent)
	);

	stats_addEntry(data);
}

/*
============
Dumps in file all the details
============
*/
void globalStats_clientDisconnect(gentity_t *ent) {
	char *data;

	// We only want game stats..
	if (!g_gamestate.integer == GS_PLAYING)
		return;
	
	// Build MODs
	client_buildMODList(ent);
	// Build HitList
	client_hitList(ent);

	// Build all the data..
	data = va("%s%s",
		client_buildGeneral(ent),
		client_buildStats(ent)
	);

	stats_addEntry(data);
}

/*
==========================
Wipe it

Clears/frees linked lists
==========================
*/
void globalStats_cleanList(void) {
	struct statEntry *node;

	while (statHead != NULL) {
		node = statHead;
		statHead = statHead->next;
		free(node);
	}
}

/*
============
Dump stats now
============
*/
void globalStats_buildStats(void) {
	int i;

	// Dump round info
	stats_addEntry(buildRound());

	for (i = 0; i < g_maxclients.integer; i++) {		
		if (level.clients[i].pers.connected != CON_CONNECTED) 
			continue;

		stats_clientData(&g_entities[i]);		
		client_buildMODList(&g_entities[i]);		
		client_hitList(&g_entities[i]);
	}
}