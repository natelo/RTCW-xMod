/*
---------------------------
Nate 'L0 - g_http_cmds.c
Created: 26.07/14

All http client related commands.
---------------------------
*/
#include "g_local.h"

// So it's little easier for the eyes..
#define _CMD(x, y) !Q_stricmp(x, y)

/*
===========
http_processMessage

Sends client query and reads reply..
===========
*/
void http_processMessage(int clientNum, char *msg, qboolean toClient) {

	if (toClient)
		CPx(clientNum, va("chat \"^5webStats^7: %s\n", msg));
	else
		AP(va("chat \"^5webStats^7: %s\n", msg));
	
	return;
}

/*
===========
http_sendQuery

Sends client query and ready reply..
===========
*/
void *http_sendQuery(void *args) {
	g_http_cmd_t *cmdSt = (g_http_cmd_t *)args;
	char  *out = NULL;
		
	if (g_httpStatsAPI.string) {
		out = http_Query(g_httpStatsAPI.string, cmdSt->cmd);
	}

	if (g_httpDebug.integer > 1)
		G_Printf(va("[httpQuery] WebReply: %s\n", out));

	// Figure out what to do with reply..
	if (out)
		http_processMessage(cmdSt->id, out, cmdSt->toClient);
	else
		CPx(cmdSt->id, "print \"^3HTTP ERROR^7: Could not connect to a webserver.\n");

	// Cleanup
	free(cmdSt);	

	if (g_httpDebug.integer)
		G_Printf("Thread destroyed.\n");

	return 0;
}

/*
===========
http_clientCommand

Entry point for client commands
===========
*/
void http_clientCommand(gentity_t *ent, char *cmd, qboolean toClient) {
	g_http_cmd_t *post_cmd = (g_http_cmd_t*)malloc(sizeof(g_http_cmd_t));	

	// Fill the generic data..	
	post_cmd->id = ent->client->ps.clientNum;
	post_cmd->toClient = toClient;

	// A cheap flood protection
	if (ent->client->pers.httpCmdIssued >= level.time)
	{
		CP(va("print \"^3Error: ^7Web commands will be available in ^3%i ^7seconds.\n", (((ent->client->pers.httpCmdIssued - level.time) / 1000) + 1)  ));

		free(post_cmd);
		return;
	}
	else {
		ent->client->pers.httpCmdIssued = level.time + 6000; // 6 sec should be enough..
	}

	// Client Stats
	if (_CMD(cmd, HTTP_CLIENT_STATS)) {
		post_cmd->cmd = va("stats||guid=%s", ent->client->sess.guid);		
	}

	// Do the magic..	
	create_thread(http_sendQuery, (void*)post_cmd);
	
	return;
}

/*
===========
http_isHttpCommand

Wrapper for client commands
===========
*/
qboolean isHttpCommand(gentity_t *ent, char *cmd1, char *cmd2, char *cmd3) {	
	char alt[128];
	char cmd[128];

	// Do not allow any web commands right before restart..
	//
	// XXX: Trying to figure out if it's the PB 
	// crashing it randomly or is it a thread related issue..
	if (level.warmupTime && level.time > level.warmupTime - 2400) {
		return qtrue;
	}

	parseCmds(cmd1, alt, cmd, qfalse);
	if (_CMD(cmd, HTTP_CLIENT_STATS))
	{
		http_clientCommand(ent, HTTP_CLIENT_STATS, qtrue);
		return qtrue;
	}
	return qfalse;
}



