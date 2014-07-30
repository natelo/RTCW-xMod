/*
---------------------------
Nate 'L0 - g_http.h
Created: 29.07/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _STATS_UNIFIED
#define _STATS_UNIFIED

// PB
#define PB_GUID_LENGTH 33 // 32 + trailing zero

// Log file
#define WEBSTATS_LOG "webStats.log"

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
	int count;
} web_deathList_s;

//
// Stats
//
typedef struct {
	int		kills;
	int		deaths;
	int		headshots;
	int		teamKills;	
	int		poison;
	int		revives;
	int		revivesRec;
	int		ammoGiv;	
	int		medGiv;		
	int		medRec;
	int		ammoRec;
	int		gibs;
	int		suicides;
	int		goomba;
	int		knife;
	int		knifeStealth;
	int		knifeThrow;
	int		killPeak;
	int		deathPeak;
	int		shotsFired;
	int		shotsHit;
	int		dynoPlanted;
	int		dynoDisarmed;
	int		mgsRepaired;
	int		ASCalled;
	int		ASThrown;
	int		ASBlocked;
	int		chickenRun;
	int		dmgTeam;
	int		dmgGiv;
	int		dmgRec;
	int		objSteals;
	int		flagCapture;
	int		flagReclaim;

	// Non-mapped - Here for OSPx 
/*
	int wShotsFired[STATS_MAX];
	int wShotsHit[STATS_MAX];
	int wHeadshots[STATS_MAX];
	int wDmgGvn[STATS_MAX];
	int wDmgRcv[STATS_MAX];
	int wTDmgGvn[STATS_MAX];
	int wTDmgRcv[STATS_MAX];
*/
} statsClientData_t;

#endif // _STATS_UNIFIED
