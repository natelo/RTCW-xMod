/*
---------------------------
Nate 'L0 - g_http.h
Created: 29.07/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _G_HTTP2
#define _G_HTTP2

/*
============
Linked list
============
*/
struct statEntry {
	char info[2048]; 
	struct statEntry *next; 
};

struct statEntry *statHead, *statTail;
int statNumber;

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

//
// MODs
//
typedef struct {
	int count;
} web_MODs_s;

//
// Hit List
//
typedef struct {
	char guid[PB_GUID_LENGTH];
	int count;
} web_deathList_s;

//
// Stats
//
typedef struct {
	int kills;
	int deaths;
	int headshots;
	int teamkills;	
	int poison;
	int revives;
	int ammoGiven;	
	int medGiven;		
	int gibs;
	int suicides;
	int goombas;
	int knifeThrow;
	int knife;
	int knifeStealth;
	int killPeak;
	int deathPeak;
	int shotsFired;
	int shotsHit;
	int dynoPlanted;
	int dynoDisarmed;
	int mgsRepaired;
	int ASCalled;
	int ASThrown;
	int chickenRuns;
	int dmgTeam;
	int dmgGiv;
	int dmgRec;
	int score;

	// Non-mapped - TODO
	int medRec;
	int ammoRec;
	int revivesRec;
	int objSteals;
	int objDestroyed;
	int flagCapture;
	int flagReclaim;

	int wShotsFired[STATS_MAX];
	int wShotsHit[STATS_MAX];
	int wHeadshots[STATS_MAX];
	int wDmgGvn[STATS_MAX];
	int wDmgRcv[STATS_MAX];
	int wTDmgGvn[STATS_MAX];
	int wTDmgRcv[STATS_MAX];
} statsClientData_s;

typedef struct {
	char id[PB_GUID_LENGTH]; // For non pb (1.0) ID can be a password string etc - but then modify constant since it's 33
	int ping;	
	statsClientData_s stats;

	// Non-Mapped
	int timeAxis;	
	int timeAllies;
}statsData_t;

#endif // _G_HTTP2
