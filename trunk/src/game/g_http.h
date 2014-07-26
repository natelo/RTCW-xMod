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
#define GLOBAL_KILLS		1
#define GLOBAL_DEATHS		2
#define GLOBAL_HEADSHOTS	3
#define GLOBAL_TEAMKILLS	4
#define GLOBAL_TEAMBLEED	5
#define GLOBAL_POISON		6
#define GLOBAL_REVIVES		7
#define GLOBAL_AMMOGIVEN	8
#define GLOBAL_MEDGIVEN		9
#define GLOBAL_GIBS			10
#define GLOBAL_SUICIDES		11
#define GLOBAL_GOOMBAS		12
#define GLOBAL_KNIFETHROWS	13
#define GLOBAL_FASTTABS		14
#define GLOBAL_STABS		15
#define GLOBAL_KILLPEAK		16
#define GLOBAL_DEATHPEAK	17
#define GLOBAL_SHOTSFIRED	18
#define GLOBAL_SHOTSHIT		19
#define GLOBAL_DYNOPLATED	20
#define GLOBAL_DYNODISARMED	21
#define GLOBAL_MGREPAIRED	22
#define GLOBAL_MEDIC		23
#define GLOBAL_SOLD			24
#define GLOBAL_ENG			25
#define GLOBAL_LT			26
#define GLOBAL_ASCALLED		27
#define GLOBAL_ASTHROWN		28
#define GLOBAL_SCORE		29
#define GLOBAL_LIMIT		30

//
//	Game Stats
//
typedef struct {
	int type;
	int value;
} g_http_userUniqueStats_s;

//
// Killer stats
//
typedef struct {
	char id[32];
	int	count;
	int weapon;
} g_http_userVictims_s;

//
// Victim stats
//
typedef struct {
	char id[32];
	int count;
	int weapon;
} g_http_userKillers_s;

//
// Individual Player Info structure
//
typedef struct {
	char token[32];			// Guid || Unique ID
	char name[MAX_NETNAME];
	unsigned int team;
	unsigned int cClass;
	char ip[15];			// No IPv6 support...	
	g_http_userUniqueStats_s stats[GLOBAL_LIMIT];
	g_http_userVictims_s hasKilled;
	g_http_userKillers_s wasKilled;
} g_http_matchinfo_t;

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
} g_http_roundStruct_t;

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
void *globalStats_roundInfo(void *args);

//
// g_http_cmds.c
//
void *http_sendQuery(void *args);
qboolean isHttpCommand(gentity_t *ent, char *cmd1, char *cmd2, char *cmd3);

#endif // _G_HTTP