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
MOD Conversions
============
*/
typedef enum {	
	STATS_MP40,
	STATS_THOMPSON,
	STATS_STEN,
	STATS_MAUSER,
	STATS_SNIPERRIFLE,
	STATS_FLAMETHROWER,
	STATS_PANZERFRAUST,
	STATS_VENOM,
	STATS_GRENADE,
	STATS_LUGER,
	STATS_COLT,
	STATS_DYNAMITE,
	STATS_MG42,
	STATS_KNIFE,
	STATS_KNIFESTEALTH,
	STATS_KNIFETHROW,
	STATS_AIRSTRIKE,
	STATS_ARTILLERY,
	STATS_POISON,
	STATS_GOOMBA,
	STATS_FALLING,
	STATS_MORTAR,
	STATS_SUICIDE,
	STATS_CHICKEN,
	STATS_DROWN,
	STATS_WORLD,
	STATS_ADMIN,
	STATS_MAX
} statsMODs;

/*
============
Kill List
============
*/
typedef struct {
	char guid[PB_GUID_LENGTH + 1];
	int token;
	int count;
} global_killList_players_s;

typedef struct {	
	global_killList_players_s victim[MAX_CLIENTS];
} global_killList_t;

/*
============
Client's MOD list
============
*/
typedef struct {	
	int count;	
} global_MODs_weapon_s;

typedef struct {
	int	client;
	global_MODs_weapon_s mod[STATS_MAX];
} global_MODs_t;

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
	{ "Tk" },	// 4
	{ "Tb" },	// 5
	{ "poi" },	// 6
	{ "rev" },	// 7
	{ "aGiv" },	// 8
	{ "mGiv" },	// 9
	{ "gbs" },	// 10
	{ "sui" },	// 11
	{ "gomb" },	// 12
	{ "knfT" },	// 13
	{ "knf" },	// 14
	{ "knfS" },	// 15
	{ "kllP" },	// 16
	{ "dthP" },	// 17
	{ "accF" },	// 18
	{ "accH" },	// 19
	{ "dynP" },	// 20
	{ "dynD" },	// 21
	{ "mgR" },	// 22
	{ "ASc" },	// 23
	{ "ASt" },	// 24
	{ "ASb" },	// 25
	{ "scr" },	// 26
	{ "chkn" },	// 27
	{ "dDiv" },	// 28
	{ "dRec" },	// 29
	{  "" }	// 30	
};

typedef struct {
	int value;
	char *label;
} global_userList_stats_s;

typedef struct {
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
	int MODs;
	int players;
} global_entryList_t;

/*
============
Global unified structure
============
*/
typedef struct {
	global_entryList_t entries;
	global_killList_t hitList[MAX_CLIENTS];
	global_MODs_t		mods[MAX_CLIENTS];
	global_userList_t	players[MAX_CLIENTS];
	g_globalRoundStats_t roundStats;
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

//
// g_http_cmds.c
//
void *http_sendQuery(void *args);
qboolean isHttpCommand(gentity_t *ent, char *cmd1, char *cmd2, char *cmd3);

#endif // _G_HTTP
