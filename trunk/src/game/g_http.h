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
#define HTTP_QUERYCOMMAND	"http_QueryCommand"
#define HTTP_SUBMIT			"http_submit"

// Player data
typedef struct {
	unsigned int kills;
	unsigned int deaths;
	unsigned int headshots;
	unsigned int teamKills;
	unsigned int teamBleed;
	unsigned int poison;
	unsigned int revives;
	unsigned int ammoGiven;
	unsigned int medGiven;
	unsigned int gibs;
	unsigned int suicides;
	unsigned int goombas;
	unsigned int knifeThrows;
	unsigned int fastStabs;
	unsigned int killPeak;
	unsigned int deathPeak;
	unsigned int shotsFired;
	unsigned int shorsHit;
	unsigned int accuracy;
} g_http_userStats_s;

// Player Info
typedef struct {
	char token[32];			// Guid || Unique ID
	char name[MAX_NETNAME];
	unsigned int team;
	unsigned int cClass;
	char ip[15];			// No IPv6 support...	
	g_http_userStats_s stats;
} g_http_matchinfo_t;

// Round info
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
	qboolean finishedRound;	// So we can track round stuff..
} g_http_roundStruct_t;

// Client commands
typedef struct {
	char *cmd;
	unsigned int id;
	qboolean toClient;
} g_http_cmd_t;



void *globalStats_roundInfo(void *args);

//
// g_http.c
//
char *http_Get(char *url, char *data);
void http_Submit(char *url, char *data);
char *http_Query(char *url, char *data);

//
// g_http_stats.c
//

//
// g_http_cmds.c
//
void *http_sendQuery(void *args);
void http_clientCommand(gentity_t *ent, char *cmd, qboolean toClient);


#endif // _G_HTTP