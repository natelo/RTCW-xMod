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
Stat flags
============
*/
#define GLOBAL_KILLS			1
#define GLOBAL_DEATHS			2
#define GLOBAL_HEADSHOTS		3
#define GLOBAL_TEAMKILLS		4
#define GLOBAL_TEAMBLEED		5
#define GLOBAL_POISON			6
#define GLOBAL_REVIVES			7
#define GLOBAL_AMMOGIVEN		8
#define GLOBAL_MEDGIVEN			9
#define GLOBAL_GIBS				10
#define GLOBAL_SUICIDES			11
#define GLOBAL_GOOMBAS			12
#define GLOBAL_KNIFETHROW		13
#define GLOBAL_KNIFE			14
#define GLOBAL_KNIFE_STEALTH	15
#define GLOBAL_KILLPEAK			16
#define GLOBAL_DEATHPEAK		17
#define GLOBAL_SHOTSFIRED		18
#define GLOBAL_SHOTSHIT			19
#define GLOBAL_DYNOPLANTED		20
#define GLOBAL_DYNODISARMED		21
#define GLOBAL_MGREPAIRED		22
#define GLOBAL_ASCALLED			22
#define GLOBAL_ASTHROWN			23
#define GLOBAL_ASBLOCKED		24
#define GLOBAL_SCORE			25
#define GLOBAL_CHICKEN			26
#define GLOBAL_DMG_GIVEN		27
#define GLOBAL_DMG_RECEIVED		28
#define GLOBAL_LIMIT			29

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
	global_MODs_weapon_s mod[STATS_MAX];
} global_MODs_t;

/*
============
Client Stats
============
*/
typedef struct {
	int value;
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
Global unified structure
============
*/
typedef struct {
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
