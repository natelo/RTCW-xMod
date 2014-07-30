/*
---------------------------
Nate 'L0 - g_http.h
Created: 29.07/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _G_HTTP2
#define _G_HTTP2

struct statEntry {
	char info[2048]; 
	struct statEntry *next; 
};

struct statEntry *statHead, *statTail;
int statNumber;

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
