#include "g_local.h"

// Exporter
//userInfoStats userStats[HTTP_USERINFO_LIMIT];

void write_globalUserStats(gentity_t *ent, int type, int value) {	
	/*
	if (!g_httpStatsUrl.string ||
		!Q_stricmp(g_httpStatsAPI.string, "") ||
		!g_httpToken.string ||
		!Q_stricmp(g_httpToken.string, "none") ||
		(!g_gamestate.integer == GS_PLAYING))
	{
		return;
	}

	// Check if we have a entry
	if (!Q_stricmp(userStats[ent->client->ps.clientNum].guid, ent->client->sess.guid)) {
		userStats[ent->client->ps.clientNum].stats[type].value = value;
	} 
	else { // A new entry..
		userStats[ent->client->ps.clientNum].slot = ent->client->ps.clientNum;
		Q_strncpyz(userStats[ent->client->ps.clientNum].guid, ent->client->sess.guid, sizeof(userStats[ent->client->ps.clientNum].guid));
		Q_strncpyz(userStats[ent->client->ps.clientNum].name, ent->client->pers.netname, sizeof(userStats[ent->client->ps.clientNum].name));
		Q_strncpyz(userStats[ent->client->ps.clientNum].ip, va("%d.%d.%d.%d",
			ent->client->sess.ip[0],
			ent->client->sess.ip[1],
			ent->client->sess.ip[2],
			ent->client->sess.ip[3]),
			sizeof(userStats[ent->client->ps.clientNum].ip));
		userStats[ent->client->ps.clientNum].stats[type].value = value;
		userStats[ent->client->ps.clientNum].uClass = ent->client->ps.stats[STAT_PLAYER_CLASS];
	}
	*/
}


void write_globalMODS(gentity_t *victim, gentity_t *attacker, int weapon) {

	if (!g_httpStatsUrl.string ||
		!Q_stricmp(g_httpStatsAPI.string, "") ||
		!g_httpToken.string ||
		!Q_stricmp(g_httpToken.string, "none") ||
		(!g_gamestate.integer == GS_PLAYING))
	{
		return;
	}


}

void *globalStats_playersList(void *args) {
	unsigned int i;

	// Loop through list
	for (i = 0; i < g_maxclients.integer; i++)
	{

	}
	return 0;
}

void *globalStats_roundInfo(void *args) {
	/*
	g_http_userInfo_t *post_roundinfo = (g_http_userInfo_t*)args;
	char *data = NULL;	

	if (g_httpStatsUrl.string)
	{
		
		data = va(
			"data=roundInfo\\%d\\%s\\%s\\%d\\%d\\%d\\%d\\%i\\%d\\%i\\%d\r\n",
			post_roundinfo->round,
			post_roundinfo->map,
			post_roundinfo->time,
			post_roundinfo->round,
			post_roundinfo->gametype,
			post_roundinfo->altGametype,
			post_roundinfo->axisStartPlayers,
			post_roundinfo->axisEndPlayers,
			post_roundinfo->alliedStartPlayers,
			post_roundinfo->alliedEndPlayers,
			(post_roundinfo->finishedRound ? 1 : 0)
		);
		

		// Send it now
		if (g_httpDebug.integer)			
			AP("print \"g_httpDebug : Sending global round stats.\n\"");

		http_Submit(g_httpStatsUrl.string, data);
	}

	free(post_roundinfo);
	return 0;
	*/

	return 0;
}


void listStructure( int num ) {	

	//AP(va("chat \"User data: Slot %i Guid - %s Name - %s IP - %s\n", userStats[num].slot, userStats[num].guid, userStats[num].name, userStats[num].ip));
	//AP(va("chat \"User data: AS THROWN: %d\n", userStats[num].stats[GLOBAL_ASTHROWN].value));
}

// Stuff we'll convert and track..
statsMODs MODtoStats(meansOfDeath_t mod) {

	switch ( mod ) {
		case MOD_MP40:
			return STATS_MP40;
		case MOD_THOMPSON:
			return STATS_THOMPSON;
		case MOD_STEN:
			return STATS_STEN;
		case MOD_MAUSER:
			return STATS_MAUSER;
		case MOD_SNIPERRIFLE:
			return STATS_SNIPERRIFLE;
		case MOD_FLAMETHROWER:
			return STATS_FLAMETHROWER;
		case MOD_ROCKET_SPLASH:
			return STATS_PANZERFRAUST;
		case MOD_VENOM:
			return STATS_VENOM;
		case MOD_GRENADE_PINEAPPLE:
			return STATS_GRENADE;
		case MOD_LUGER:
			return STATS_LUGER;
		case MOD_COLT:
			return STATS_COLT;
		case MOD_DYNAMITE_SPLASH:
			return STATS_DYNAMITE;
		case MOD_MACHINEGUN:
			return STATS_MG42;		
		case MOD_KNIFE2:
			return STATS_KNIFE;
		case MOD_KNIFE_STEALTH:
			return STATS_KNIFESTEALTH;
		case MOD_THROWKNIFE:
			return STATS_KNIFETHROW;
		case MOD_AIRSTRIKE:
			return STATS_AIRSTRIKE;
		case MOD_ARTILLERY:
			return STATS_ARTILLERY;
		case MOD_POISONDMED:
			return STATS_POISON;
		case MOD_GOOMBA:
			return STATS_GOOMBA;
		case MOD_FALLING:
			return STATS_FALLING;
		case MOD_MORTAR_SPLASH:
			return STATS_MORTAR;
		case MOD_SELFKILL:
			return STATS_SUICIDE;
		case MOD_CHICKEN:
			return STATS_CHICKEN;
		case MOD_WATER:
			return STATS_DROWN;
		case MOD_TRIGGER_HURT:
			return STATS_WORLD;
		case MOD_CRUSH:
			return STATS_WORLD;
		case MOD_ADMKILL:
			return STATS_ADMIN;			
		default:
			return STATS_MAX;
	}
}


void write_globalMODs(gentity_t *victim, meansOfDeath_t mod) {

}

void write_globalKillList(gentity_t *victim, gentity_t *attacker) {

}