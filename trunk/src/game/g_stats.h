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

