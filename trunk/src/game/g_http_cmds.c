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

Sends client query and ready reply..
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

	if (g_httpDebug.integer)
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

Wrapper/Entry point for client commands
===========
*/
void http_clientCommand(gentity_t *ent, char *cmd, qboolean toClient) {
	g_http_cmd_t *post_cmd = (g_http_cmd_t*)malloc(sizeof(g_http_cmd_t));
	qboolean sFree = qfalse;

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
	if (_CMD(cmd, HTTP_QUERYCOMMAND)) {
		post_cmd->cmd = va("stats||guid=%s", ent->client->sess.guid);		
	}
	else {
		sFree = qtrue;
	}

	// Do the magic..
	if (!sFree) {
		create_thread(http_sendQuery, (void*)post_cmd);
	}
	else {
		free(post_cmd);
	}
	return;
}



