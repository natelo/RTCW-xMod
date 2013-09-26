/*
===========================================================================
L0 - g_stats.h

Stats stuff

Created: 9. Sept / 2013
Last Updated: 10. Sept / 2013
===========================================================================
*/

static qboolean firstheadshot;						
static qboolean firstblood;	

/**** Killing sprees (consistent over round) ****/
typedef struct {
	char *msg;
	char *snd;	
} killing_sprees_t;

static const killing_sprees_t killingSprees[] = {
	{"MULTI KILL!",			"multikill.wav"},
	{"MEGA KILL!",			"megakill.wav"},
	{"RAMPAGE!",			"rampage.wav"},
	{"GUNSLINGER!",			"gunslinger.wav"},
	{"ULTRA KILL!",			"ultrakill.wav"},
	{"MANIAC!",				"maniac.wav"},
	{"SLAUGHTER!",			"slaughter.wav"},
	{"MASSACRE!",			"massacre.wav"},
	{"IMPRESSIVE!",			"impressive.wav"},
	{"DOMINATING!",			"dominating.wav"},
	{"BLOODBATH!",			"bloodbath.wav"},
	{"KILLING MACHINE!",	"killingmachine.wav"},
	{"MONSTER KILL!",		"monsterkill.wav"},
	{"LUDICROUS KILL!",		"ludicrouskill.wav"},
	{"UNSTOPPABLE!",		"unstoppable.wav"},
	{"UNREAL!",				"unreal.wav"},
	{"OUTSTANDING!",		"outstanding.wav"},
	{"WICKED SICK!",		 "wickedsick.wav"},
	{"HOLY SHIT!",			"holyshit.wav"},
	{"BLAZE OF GLORY!!",	"blazeofglory.wav"},
	{NULL,					NULL}
};

/**** Killer Sprees (resets when player dies) ****/
typedef struct {
	char *msg;
	char *snd;	
} killer_sprees_t;

static const killer_sprees_t killerSprees[] = {
	{"MULTI KILL!",		"multikill.wav"},
	{"KILLING SPREE!",	"killingspree.wav"},
	{"RAMPAGE!",		"rampage.wav"},
	{"ULTRA KILL!",		"ultraKill.wav"},
	{"MONSTER KILL!",	"monsterkill.wav"},
	{"LUDICROUS KILL!", "ludicrouskill.wav"},
	{"DOMINATING!",		"dominating.wav"},
	{"GODLIKE!",		"godlike.wav"},
	{"UNSTOPPABLE!",	"unstoppable.wav"},
	{"WICKED SICK!",	"wickedsick.wav"},
	{"HOLY SHIT!!",		"holyshit.wav"},
	{NULL,				NULL}
};

/**** Map Stats ****/
#define MAP_KILLER			1
#define MAP_KILLING_SPREE   2
#define MAP_VICTIM			3
#define MAP_DEATH_SPREE		4
#define MAP_REVIVES			5
#define MAP_HEADSHOTS		6

/**** Map Stats & RMS (Round (Warmup) Match Stats) ****/
static char * stats_chars[]={
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k",
	"l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v",
	"w", "x", "y", "z", "A", "B", "C", "D", "E", "F", "G", 
	"H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", 
	"S", "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3",
	"4", "5", "6", "7", "8", "9", "0", "!", "@", "#", "$",
	"%", "^", "&", "*", "(", ")", "[", "]", "|", "'", ";", 
	":", ",", ".", "?", "/", ">", "<", "-", " ", "+", "=", 
	"-", "_", "~" 
};

/**** RMS (Round Match Stats) ****/
#define ROUND_PRINT			1	// Should always be First
#define ROUND_KILLS			2
#define ROUND_DEATHS		3
#define ROUND_HEADSHOTS		4
#define ROUND_TEAMKILLS		5
#define ROUND_TEAMBLEED		6
#define ROUND_POISON		7
#define ROUND_REVIVES		8
#define ROUND_AMMOGIVEN		9
#define ROUND_MEDGIVEN		10
#define ROUND_GIBS			11
#define ROUND_SUICIDES		12
#define ROUND_KNIFETHROW	13
#define ROUND_FASTSTABS		14
#define ROUND_CHICKEN		15
#define ROUND_KILLPEAK		16
#define ROUND_DEATHPEAK		17
#define ROUND_ACC			18
#define ROUND_KR			19
#define ROUND_EFF			20
#define ROUND_LIMIT			21	// Should always be Last!

typedef struct {
	char *reward;
	char *snd;
	char *label;
} round_stats_t;

static const round_stats_t rSM[] = {	
	{"Results from last match are", "intro.wav",			""},
	{"Most kills",					"blazeofglory.wav",		""},	
	{"Born to Die",					"yousuck.wav",			""},
	{"Headhunter",					"headhunter.wav",		"hs"},
	{"TeamKiller",					"teamkiller.wav",		""},
	{"TeamBleeder",					"teambleeder.wav",		"hp"},
	{"Toxic Doctor",				"toxic.wav",			""},
	{"Most Revives",				"excellent.wav",		""},
	{"Most Ammo Given",				"ammo.wav",				""},
	{"Most Med Given",				"med.wav",				""},
	{"Corpse Expert",				"ownage.wav",			" gibs"},
	{"Most Depressed",				"suicides.wav",			" suicides"},
	{"Knife Juggler",				"knife_juggler.wav",	""},
	{"Stealth Killer",				"assasin.wav",			""},
	{"Coward",						"chicken.wav",			""},
	{"Highest Killer Spree",		"killingmachine.wav",	""},
	{"Highest Death Spree",			"slaughter.wav",		""},
	{"Accuracy",					"accuracy.wav",			""},
	{"Killer Ratio",				"outstanding.wav",		""},
	{"Most Efficient player",		"impressive.wav",		""},
	{NULL,							NULL,		NULL}
};

struct round_stats_structure_s {
	unsigned int stats;				// Label (see defines above)
	unsigned int score;				// Most of stats
	char player[MAX_TOKEN_CHARS];	// Players that made the list
	char out[MAX_TOKEN_CHARS];		// For output
};

typedef struct round_stats_structure_s roundStruct;
extern roundStruct roundStats[];

