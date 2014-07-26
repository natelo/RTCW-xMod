#include "g_local.h"

void write_globalUserStats(gentity_t *ent, int type, int value) {
	g_http_userInfo_t userStats[HTTP_USERINFO_LIMIT];

	if (!g_httpStatsUrl.string ||
		!Q_stricmp(g_httpStatsAPI.string, "") ||
		!g_httpToken.string ||
		!Q_stricmp(g_httpToken.string, "none") ||
		(!g_gamestate.integer == GS_PLAYING))
	{
		return;
	}

	// Check if client is already added so just track the stats..
	if (!Q_stricmp(userStats[ent->client->ps.clientNum].id, ent->client->sess.guid))	{
		userStats[ent->client->ps.clientNum].stats[type].value = value;
	}
	else {
		int i, j = -1;

		// Loop through stack and figure out if we have a space for it
		for (i = 0; i < HTTP_USERINFO_LIMIT; ++i) {
			if (userStats[i].id == NULL) {
				j = i;
				break;
			}
		}

		if (j == -1) {
			G_Printf("WARNING: web-userStats list is full! (This should not happen..)\n");
			return;
		}
		else {
			// Save user stuff now..			
			Q_strncpyz(userStats[j].id, ent->client->sess.guid, sizeof(userStats[j].id));
			Q_strncpyz(userStats[j].name, ent->client->pers.netname, sizeof(userStats[j].name));
			Q_strncpyz(userStats[j].ip, va("%d.%d.%d.%d",
				ent->client->sess.ip[0],
				ent->client->sess.ip[1],
				ent->client->sess.ip[2],
				ent->client->sess.ip[3]),
				sizeof(userStats[j].ip));
			userStats[j].stats[type].value = value;
		}
	}
	return;
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
	g_http_userInfo_t *post_roundinfo = (g_http_userInfo_t*)args;
	char *data = NULL;	

	if (g_httpStatsUrl.string)
	{
		/*
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
		*/

		// Send it now
		if (g_httpDebug.integer)			
			AP("print \"g_httpDebug : Sending global round stats.\n\"");

		http_Submit(g_httpStatsUrl.string, data);
	}

	free(post_roundinfo);
	return 0;
}


void listStructure( void ) {	
	g_http_userInfo_t userInfo[HTTP_USERINFO_LIMIT];
	int i;

	for (i = 0; i < HTTP_USERINFO_LIMIT; i++)
	{
		if (userInfo[i].id != NULL)
			AP(va("print \"Stats Info: %s %s \n", userInfo[i].name, userInfo[i].id));
	}
	return;
}
