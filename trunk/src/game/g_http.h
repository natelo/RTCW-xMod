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

// h_http_userInfo Structure limit
#define HTTP_USERINFO_LIMIT	64

/*
============
Stats types and converions
============
*/
#define GLOBAL_KILLS		1 // x
#define GLOBAL_DEATHS		2 // x
#define GLOBAL_HEADSHOTS	3 // x
#define GLOBAL_TEAMKILLS	4 // x
#define GLOBAL_TEAMBLEED	5 // x
#define GLOBAL_POISON		6 // x
#define GLOBAL_REVIVES		7 // x
#define GLOBAL_AMMOGIVEN	8 // x
#define GLOBAL_MEDGIVEN		9 // x
#define GLOBAL_GIBS			10 // x
#define GLOBAL_SUICIDES		11 // x
#define GLOBAL_GOOMBAS		12 // x
#define GLOBAL_KNIFETHROW	13 // x
#define GLOBAL_FASTSTABS	14 // x
#define GLOBAL_STABS		15 // x
#define GLOBAL_KILLPEAK		16 // x
#define GLOBAL_DEATHPEAK	17 // x
#define GLOBAL_SHOTSFIRED	18 // x
#define GLOBAL_SHOTSHIT		19 // x
#define GLOBAL_DYNOPLANTED	20 // x
#define GLOBAL_DYNODISARMED	21 // x
#define GLOBAL_MGREPAIRED	22 // x
#define GLOBAL_ASCALLED		22 // x
#define GLOBAL_ASTHROWN		23 // x
#define GLOBAL_ASBLOCKED	24 // x
#define GLOBAL_SCORE		25 // x
#define GLOBAL_CHICKEN		26 // x
#define GLOBAL_LIMIT		27

//
// Stats MODs we'll track..
//
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
Global Stats Structure

Layout:
	Round
		- Round Data

		- [array] Player's list
			- Player data
			- Player Rename History
			- Player Stats
				- Stats type [id]
				- Score to track

			- Player MODs
				- Attacker Data
				- MOD Data
					- Weapon
					- Times
============
*/

// Kill list details
typedef struct {
	char guid[PB_GUID_LENGTH + 1];
	int count;
} global_userKillers_list_s;

// User Kill list
/*
	Victim = slot
	if Victim leaves it fill fire a packet to a web server with his kill list..
*/
typedef struct {
	int slot;
	global_userKillers_list_s victim[MAX_CLIENTS];
} global_userKillers_t;

// MOD count
typedef struct {	
	int count;	
} global_MODs_weapon_s;

// User MODs
typedef struct {	
	int slot;	
	global_MODs_weapon_s mod[STATS_MAX];
} global_MODs_t;

// User stats
typedef struct {
	int value;
} global_userList_stats_s;

// User names
typedef struct {
	char name[MAX_NETNAME];
} global_userList_nameHistory_s;

// General properties
typedef struct {
	int slot;
	char name[MAX_NETNAME];
	char ip[15];
	char uClass;	
	global_userList_nameHistory_s nameHistory;			// Renaming
	global_userList_stats_s stats[GLOBAL_LIMIT];		// Game stats
} global_userList_s;

//
// Round info structure
//
typedef struct {
	unsigned int teamWon;
	char *map;
	char *time;
	int round;
	int gametype;
	int altGametype; // Obj, DM..
	qboolean finishedRound;	// If round is cut short..	
} g_globalStats_t;

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
Declarations
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
void write_globalMODs(gentity_t *victim, meansOfDeath_t mod);
void write_globalKillList(gentity_t *victim, gentity_t *attacker);

void listStructure(int num);

//
// g_http_cmds.c
//
void *http_sendQuery(void *args);
qboolean isHttpCommand(gentity_t *ent, char *cmd1, char *cmd2, char *cmd3);

//
// Exporters
//
typedef struct g_http_userInfo_s userInfoStats;
extern userInfoStats userStats[];

#endif // _G_HTTP