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

//
// Stats types
//
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
#define GLOBAL_CHICKEN		26 // wx
#define GLOBAL_LIMIT		27

//
//	Game Stats
//
typedef struct {
	int value;
} g_http_userUniqueStats_s;

//
// Killer stats
//
typedef struct {
	char id[PB_GUID_LENGTH];
	int	count;
	int weapon;
} g_http_userVictims_s;

//
// Victim stats
//
typedef struct {
	char id[PB_GUID_LENGTH];
	int count;
	int weapon;
} g_http_userKillers_s;

//
// Individual Player Info structure
//
struct g_http_userInfo_s {
	int	slot;
	char guid[PB_GUID_LENGTH+1];
	char name[MAX_NETNAME];
	char ip[15];			// No IPv6 support...
	int uClass;
	g_http_userUniqueStats_s stats[GLOBAL_LIMIT];	
};

// h_http_userInfo Structure limit
#define HTTP_USERINFO_LIMIT	64

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
	int axisStartPlayers;
	int axisEndPlayers;
	int alliedStartPlayers;
	int alliedEndPlayers;
	qboolean finishedRound;	// If round is cut short..
} g_http_roundInfo_t;

//
// Client commands structure
//
typedef struct {
	char *cmd;
	unsigned int id;
	qboolean toClient;
} g_http_cmd_t;

//
// g_http.c
//
void http_Submit(char *url, char *data);
char *http_Query(char *url, char *data);

//
// g_http_stats.c
//
void write_globalUserStats(gentity_t *ent, int type, int value);
void listStructure(int num);

//
// g_http_cmds.c
//
void *http_sendQuery(void *args);
qboolean isHttpCommand(gentity_t *ent, char *cmd1, char *cmd2, char *cmd3);

//
// Macros
//
#define GLOBALSTATS(x,y,z) write_globalUserStats(x, y, z)

//
// Exporter
//
typedef struct g_http_userInfo_s userInfoStats;
extern userInfoStats userStats[];

#endif // _G_HTTP