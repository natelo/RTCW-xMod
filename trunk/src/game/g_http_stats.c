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

/*
=============
Replace some stuff so form can handle it..
=============
*/
char *webReplace(char *token, char old, char out) {
	char *p = token;

	while (*p)
	{
		if (*p == old)
			*p = out;

		++p;
	}
	return token;
}

/*
=============
Build stats

Builds full stats list.
NOTE: This is not going to be cheap..
=============
*/
char *buildWebStats(gentity_t *ent) {
	char *stats;


	// This queries everything under a sun for X players...*caugh*
	stats = va("%s\"%d\"\"EOUS",
		webReplace(ent->client->pers.netname, '&', '~'), // To avoid falsifying POST and break everything..		
		ent->client->ps.persistant[PERS_SCORE]
		);

	return stats;
}

/*
=============
Build list

Builds a list of all the players
----------------------
* If you're planning to run your own ranked web stats then note few things.
*
* Data sent is basically a single text chunk.
* To grab it on webserver query for data (post field request).
*
* Basically you'll probably need to use explode (PHP) or altenative function
* for any other language.
*
* Simply break text into strings on EOUS (End Of User Stats),
* after it additionally break strings on "" param and check function above
* to see in what order data comes and dump it in database.
*
* NOTE: If you want to replace "" then find a valid alternative so player names can't
* break it. If you'll fillter on // any player with // in name, will break the stats etc.
*
* Word of caution - Engine limits some of chars but doesn't check against SQL injects
* or XSS attacks, so be sure you sanitize your data before you let it anywhere near DB.
* How you sanitize data is up to you, but to deal with SQL I suggest PDO layer (in PHP)
* and you'll have to either use some kinda of filtering layer for XSS or write custom rules
* as name and guid can be sent in any format and cause problems.
*
* Additionally use some kinda of "acceptance" scheme to filter entries according to what you expect..
* Death, kills, hits, gibs..are integers so filter to accept only numeric values as source code is open
* so anyone can apply for pass and then recompile mod with malicious data that is sent to you.
----------------------

=============
*/
qboolean webStats(void) {
	int i;
	char list[2048];
	gentity_t *player;

	Q_strncpyz(list, "", sizeof(list));
	for (i = 0; i < level.numConnectedClients; i++) {

		player = &g_entities[level.sortedClients[i]];
		if (player->client->sess.sessionTeam == TEAM_SPECTATOR) {
			continue;
		}

		// Build the list
		strcat(list, buildWebStats(player));
	}


	// Fire a packet
	//if (httpSubmit(g_etpub_stats_master_url.string, list))
	//if (httpSubmit(g_etpub_stats_master_url.string, "Fill with some data.."))
	//	return qtrue;

	return qfalse;
}
