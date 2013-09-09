/*
===========================================================================
L0 - g_stats.h

Stats stuff

Created: 9. Sept / 2013
Last Updated: 
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
