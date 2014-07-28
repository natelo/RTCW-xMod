/*
---------------------------
Nate 'L0 - g_http.h
Created: 23.07/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _G_HTTP
#define _G_HTTP

// HTTP Commands for invoking functionality thru a wrapper
#define HTTP_CLIENT_STATS	"stats"

/*
============
Client's MOD list
============
*/
typedef struct {
	int count;
	char *label;
} global_MODs_count_s;

typedef struct {	
	char guid[PB_GUID_LENGTH + 1];
	global_MODs_count_s MODs[STATS_MAX];
} global_MODs_t;

static const global_MODs_count_s global_modTypes[] = {
	{ STATS_MP40, "mp40" },
	{ STATS_THOMPSON, "thom" },
	{ STATS_STEN, "sten" },
	{ STATS_MAUSER, "maur" },
	{ STATS_SNIPERRIFLE, "snipr" },
	{ STATS_FLAMETHROWER, "flam" },
	{ STATS_PANZERFRAUST, "pf" },
	{ STATS_VENOM, "venm" },
	{ STATS_GRENADE, "grnd" },
	{ STATS_LUGER, "lugr" },
	{ STATS_COLT, "colt" },
	{ STATS_DYNAMITE, "dyna" },
	{ STATS_MG42, "mg42" },
	{ STATS_KNIFE, "knf" },
	{ STATS_KNIFESTEALTH, "knfs" },
	{ STATS_KNIFETHROW, "knft" },
	{ STATS_AIRSTRIKE, "airs" },
	{ STATS_ARTILLERY, "arty" },
	{ STATS_POISON, "pois" },
	{ STATS_GOOMBA, "goom" },
	{ STATS_FALLING, "fall" },
	{ STATS_MORTAR, "mrtr" },
	{ STATS_SUICIDE, "sui" },
	{ STATS_CHICKEN, "chkn" },
	{ STATS_DROWN, "drwn" },
	{ STATS_WORLD, "wrld" },
	{ STATS_ADMIN, "admn" },
	{ STATS_MAX, "" }
};

/*
============
Client Stats
============
*/
typedef enum {
	GLOBAL_KILLS,			// 1
	GLOBAL_DEATHS,			// 2
	GLOBAL_HEADSHOTS,		// 3
	GLOBAL_TEAMKILLS,		// 4
	GLOBAL_TEAMBLEED,		// 5
	GLOBAL_POISON,			// 6
	GLOBAL_REVIVES,			// 7
	GLOBAL_AMMOGIVEN,		// 8
	GLOBAL_MEDGIVEN,		// 9
	GLOBAL_GIBS,			// 10
	GLOBAL_SUICIDES,		// 11
	GLOBAL_GOOMBAS,			// 12
	GLOBAL_KNIFETHROW,		// 13
	GLOBAL_KNIFE,			// 14
	GLOBAL_KNIFE_STEALTH,	// 15
	GLOBAL_KILLPEAK,		// 16
	GLOBAL_DEATHPEAK,		// 17
	GLOBAL_SHOTSFIRED,		// 18
	GLOBAL_SHOTSHIT,		// 19
	GLOBAL_DYNOPLANTED,		// 20
	GLOBAL_DYNODISARMED,	// 21
	GLOBAL_MGREPAIRED,		// 22
	GLOBAL_ASCALLED,		// 23
	GLOBAL_ASTHROWN,		// 24
	GLOBAL_ASBLOCKED,		// 25
	GLOBAL_SCORE,			// 26
	GLOBAL_CHICKEN,			// 27
	GLOBAL_DMG_GIVEN,		// 28
	GLOBAL_DMG_RECEIVED,	// 29
	GLOBAL_LIMIT
} global_StatsMap_list_t;

typedef struct {	
	char *label;
} global_statsMap_stats_s;

static const global_statsMap_stats_s global_statsTypes[] = {
	{ "kls" },	// 1
	{ "dth" },	// 2
	{ "hs" },	// 3
	{ "tk" },	// 4
	{ "tb" },	// 5
	{ "poi" },	// 6
	{ "rev" },	// 7
	{ "agiv" },	// 8
	{ "mgiv" },	// 9
	{ "gbs" },	// 10
	{ "sui" },	// 11
	{ "gomb" },	// 12
	{ "knft" },	// 13
	{ "knf" },	// 14
	{ "knfs" },	// 15
	{ "kllp" },	// 16
	{ "dthp" },	// 17
	{ "accf" },	// 18
	{ "acch" },	// 19
	{ "dynp" },	// 20
	{ "dynd" },	// 21
	{ "mgr" },	// 22
	{ "asc" },	// 23
	{ "ast" },	// 24
	{ "asb" },	// 25
	{ "scr" },	// 26
	{ "chkn" },	// 27
	{ "ddiv" },	// 28
	{ "drec" },	// 29
	{  "" }	// 30	
};

typedef struct {
	int value;
	char *label;
} global_userList_stats_s;

typedef struct {
	char guid[PB_GUID_LENGTH + 1];
	char name[MAX_NETNAME];	
	char ip[16];
	int clientClass;
	int team;
	int ping;
	global_userList_stats_s stats[GLOBAL_LIMIT];		// Game stats
} global_userList_t;

/*
============
Round info structure
============
*/
typedef struct {
	int winner;
	char *map;
	float time;
	int round;
	int gametype;
	int altGametype; // Obj, DM..
	char firstBloodAttacker[PB_GUID_LENGTH + 1];
	char firstBloodVictim[PB_GUID_LENGTH + 1];
	char firstHeadshotAttacker[PB_GUID_LENGTH + 1];
	char firstHeadshotVictim[PB_GUID_LENGTH + 1];
	char lastBloodAttacker[PB_GUID_LENGTH + 1];
	char lastBloodVictim[PB_GUID_LENGTH + 1];
	qboolean finishedRound;	// If round is cut short..	
} g_globalRoundStats_t;

/*
============
Entry Table

Holds the entry list count for each table
============
*/
typedef struct {
	int hitList;
	int hitListClients;
	int MODs;
	int players;
} global_entryList_t;

/*
============
Unified table

Table is passed to thread so everything can be allocated.
============
*/
typedef struct {
	global_entryList_t		entries;
	global_MODs_t			mods[MAX_CLIENTS];
	global_userList_t		players[MAX_CLIENTS];
	g_globalRoundStats_t	roundStats;
} global_Stats_t;

/*
============
Client commands structure
============
*/
typedef struct {
	char *cmd;
	unsigned int id;
	qboolean toClient;
} g_http_cmd_t;

/*
============
Prototypes
============
*/
//
// g_http.c
//
void http_Submit(char *url, char *data);
char *http_Query(char *url, char *data);

//
// g_http_stats.c
//
qboolean webStatsAreEnabled(void);
void write_globalMODs(gentity_t *victim, meansOfDeath_t MOD);
void write_globalKillList(gentity_t *victim, gentity_t *attacker);
void globalStats(qboolean finished);
void cleanGlobalStats(void);
void sentClientStats(int clientNum);

//
// g_http_cmds.c
//
void *http_sendQuery(void *args);
qboolean isHttpCommand(gentity_t *ent, char *cmd1, char *cmd2, char *cmd3);

#endif // _G_HTTP
