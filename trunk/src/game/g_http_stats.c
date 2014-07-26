#include "g_local.h"

void *globalStats_playersList(void *args) {
	unsigned int i;

	// Loop through list
	for (i = 0; i < g_maxclients.integer; i++)
	{

	}
	return 0;
}

void *globalStats_roundInfo(void *args) {
	g_http_roundStruct_t *post_roundinfo = (g_http_roundStruct_t*)args;
	char *data;

	// Fil it with some data (debug mainly)
	post_roundinfo->round = 1;
	post_roundinfo->map = "blah map";
	post_roundinfo->time = "10 mins";
	post_roundinfo->gametype = 6;
	post_roundinfo->altGametype = 0;
	post_roundinfo->axisStartPlayers = 10;
	post_roundinfo->axisEndPlayers = 5;
	post_roundinfo->alliedStartPlayers = 965464;
	post_roundinfo->alliedEndPlayers = 643242342;
	post_roundinfo->finishedRound = qtrue;

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
}
