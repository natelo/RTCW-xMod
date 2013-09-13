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
	{"MULTI KILL!", "multikill.wav"},
	{"MEGA KILL!", "megakill.wav"},
	{"RAMPAGE!", "rampage.wav"},
	{"GUNSLINGER!", "gunslinger.wav"},
	{"ULTRA KILL!", "ultrakill.wav"},
	{"MANIAC!", "maniac.wav"},
	{"SLAUGHTER!", "slaughter.wav"},
	{"MASSACRE!", "massacre.wav"},
	{"IMPRESSIVE!", "impressive.wav"},
	{"DOMINATING!", "dominating.wav"},
	{"BLOODBATH!", "bloodbath.wav"},
	{"KILLING MACHINE!", "killingmachine.wav"},
	{"MONSTER KILL!", "monsterkill.wav"},
	{"LUDICROUS KILL!", "ludicrouskill.wav"},
	{"UNSTOPPABLE!", "unstoppable.wav"},
	{"UNREAL!", "unreal.wav"},
	{"OUTSTANDING!", "outstanding.wav"},
	{"WICKED SICK!", "wickedsick.wav"},
	{"HOLY SHIT!", "holyshit.wav"},
	{"BLAZE OF GLORY!!", "blazeofglory.wav"},
	{NULL, NULL}
};

/**** Killer Sprees (resets when player dies) ****/
typedef struct {
	char *msg;
	char *snd;	
} killer_sprees_t;

static const killer_sprees_t killerSprees[] = {
	{"MULTI KILL!" , "multikill.wav"},
	{"KILLING SPREE!" , "killingspree.wav"},
	{"RAMPAGE!" , "rampage.wav"},
	{"ULTRA KILL!" , "ultraKill.wav"},
	{"MONSTER KILL!" , "monsterkill.wav"},
	{"LUDICROUS KILL!" , "ludicrouskill.wav"},
	{"DOMINATING!" , "dominating.wav"},
	{"GODLIKE!" , "godlike.wav"},
	{"UNSTOPPABLE!" , "unstoppable.wav"},
	{"WICKED SICK!" , "wickedsick.wav"},
	{"HOLY SHIT!!" , "holyshit.wav"},
	{NULL, NULL}
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

/**** Round Stats ****/
typedef struct {
	char *type;
	char *label;
	char *snd;		
} round_stats_t;

static const round_stats_t roundStatsMSGs[] = {
	{"Results from last Match are", "", "0.wav"},
	{"Most Kills", "", "1.wav"},
	{"Born to Die" , "deaths", "2.wav"},
	{"Most Revives", "", "3.wav"},
	{"Most Team Kills",  "", "4.wav"},
	{"Most Headshots",  "", "5.wav"},
	{"Most Poison Kills", "", "6.wav"},
	{"Most Ammo Given", "", "7.wav"},
	{"Most Med Given", "", "8.wav"},
	{"Top Stealth Killer", "knife stabs", "9.wav"},
	{"Knife Juggling Expert", "", "10.wav"},
	{"Most Depressed Player",  "suicides", "11.wav"},
	{"Chicken Shit", "bail outs", "12.wav"},
	{"Corpse Expert",  "gibs", "13.wav"},
	{"Highest Killer Ratio", "", "14.wav"},
	{"Top Accuracy", "%", "15.wav"},
	{"Most Efficient", "%", "16.wav"},
	{NULL, NULL}
};

#define ROUND_MOSTKILLS		1
#define ROUND_BORNTODIE		2
#define ROUND_REVIVES		3
#define ROUND_TEAMKILLS		4
#define ROUND_HEADSHOTS		5
#define ROUND_POISONKILLS	6
#define ROUND_AMMOGIVEN		7
#define ROUND_MEDGIVEN		8
#define ROUND_FASTSTABS		9
#define ROUND_KNIFETHROW	10
#define ROUND_SUICIDES		11
#define ROUND_CHICKEN		12
#define ROUND_GIBS			13
#define ROUND_KILLERRATIO	14
#define ROUND_ACCURACY		15
#define ROUND_EFFICIENCY	16
#define ROUND_LIMIT			17	// If adding more, increase this (it should always be last!)

struct round_stats_structure_s {
	unsigned int stats;				// Label (see defines above)
	unsigned int score;				// Most of stats
	float score2;					// Accuracy, killer ratio..
	char out[32];					// For output
	char player[MAX_STRING_CHARS];	// Players that made the list
};

typedef struct round_stats_structure_s roundStruct;
extern roundStruct roundStats[];
