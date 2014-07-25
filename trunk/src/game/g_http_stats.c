#include "g_http_stats.h"

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
	post_roundinfo->alliedStartPlayers = 9;
	post_roundinfo->alliedEndPlayers = 6;
	post_roundinfo->finishedRound = qtrue;

	//if (g_httpStatsUrl.string /*&& g_httpToken.string*/)
	{
		data = va(
			"GlobalStatsRound: TW: %d\\M: %s\\T: %s\\R: %d\\GT: %d\\AGT: %d\\RRS: %d\\RRE: %i\\BRS: %d\\BRE: %i\\RE: %d ",
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

		httpPost(g_httpStatsUrl.string, data);
	}

	free(post_roundinfo);
	return 0;
}

void *globalStats_sendCommand(void *args) {
	char *msg;


	if (g_httpStatsAPI.string) {
		msg = httpGet(g_httpStatsAPI.string, args);
	}

	AP(va("chat \"console: YEAH %s ..DOH\n\"", msg));

	return 0;
}