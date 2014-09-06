/*
---------------------------
Nate 'L0 - g_http_func.c
Created: 26.07/14
Updated: 05.07/14

All http client related commands.
---------------------------
*/
#include "g_local.h"

// So it's little easier..
#define _CMD(x, y) !Q_stricmp(x, y)

/*
===========
http_processMessage

Prints message back to public/client.
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

Sends client query and reads reply.
===========
*/
void *http_sendQuery(void *args) {
	g_http_cmd_t *cmdSt = (g_http_cmd_t *)args;
	char  *out = NULL;
	char *url = va("%s/%s", g_httpStatsAPI.string, cmdSt->bit);
		
	if (g_httpStatsAPI.string) {
		out = http_Query(url, cmdSt->cmd);
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
void http_clientCommand(gentity_t *ent, char *bit, char *cmd, qboolean toClient) {
	g_http_cmd_t *post_cmd = (g_http_cmd_t*)malloc(sizeof(g_http_cmd_t));

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
	post_cmd->bit = va("%s", bit);
	post_cmd->cmd = va("%s", cmd);		
	

	// Fill the generic data..	
	post_cmd->id = ent->client->ps.clientNum;
	post_cmd->toClient = toClient;

	// Do the magic..	
	create_thread(http_sendQuery, (void*)post_cmd);

	return;
}

/*
===========
Prints Categories
===========
*/
void sCmd_listCommands(gentity_t *ent) {
	char *cmds;

	// Print commands..
	cmds = va(
		"^3Global Stats Commands^7\n"
		"----------------------------\n"
		"!stats top \n"
		"!stats bottom \n"
		"!stats rank\n"
		"!stats chances\n"
		"!stats info\n"
		"!stats lastseen\n"
		"----------------------------\n"
		"^3Tip: ^7Use ?stats <type> to view info & usage\n"
	);

	CP(va("print \"%s\n", cmds));
}


/*
===========
Stats Structure
===========
*/
typedef struct {
	char *command;
	void(*pCommand)(gentity_t *ent, qboolean fParam);
	qboolean fParam;
	const char *help;
	const char *usage;
	const char *extra;
} globalStats_cmds_t;

/*
	Commands..
*/
static const globalStats_cmds_t statsCmd[] = {
	{ "top",		sCmd_top,		qfalse, "Shows top player on this server. Use @ to show overall top player.", "!stats top", "Use @ to show overall- !stats top @" },
	{ "bottom",		sCmd_bottom,	qfalse, "Shows worst player on this server. Use @ to show overall worst player.", "!stats top", "" },
	{ "rank",		sCmd_rank,		qfalse, "Shows your or selected players rank.", "!stats rank or !stats rank", "" },
	{ "chances",	sCmd_chances,	qfalse, "Compares you with a player and calculcates your chances to prevail in a dog fight.", "!stats chances <slot>", "" },
	{ "info",		sCmd_info,		qfalse, "Prints your or targeted player's info stats.", "!stats info <kr|eff|seen> <slot>", "" },
	{ "lastseen",	sCmd_lastseen,	qfalse, "Prints date for when targeted player was last seen.", "!stats lastseen <stats-player-id>", "" },

	{ NULL, NULL, qfalse, NULL, NULL }
};

/*
===========
http_isHttpCommand

Wrapper for client commands
===========
*/
qboolean isHttpCommand(gentity_t *ent, char *cmd, char *param, qboolean help) {
	char cmd1[128];
	char cmd2[128];
	char cmd3[128];
	char arg[MAX_SAY_TEXT];
	unsigned int i, \
		cmds = ARRAY_LEN(statsCmd);
	const globalStats_cmds_t *sCMD;
	qboolean wasUsed = qfalse;
	
	// Bail out if stuff is not set..
	if (_CMD(g_httpStatsUrl.string, "") ||
		_CMD(g_httpStatsAPI.string, "") ||
		_CMD(g_httpToken.string, "") ||
		_CMD(g_httpToken.string, "none"))
	{
		if (_CMD(g_httpToken.string, "none") || !g_httpToken.string)
			CP("print \"^3INFO^7: WebStats are disabled on this server.\n");
		else
			CP("print \"^3INFO^7: Owner of this server has not configured WebStats correctly..\n");
		return qtrue;
	}

	// Do not allow any web commands right before restart as it will crash due theads & unloading of dll..
	if (level.warmupTime && level.time > level.warmupTime - 2400 || level.intermissiontime) {
		return qtrue;
	}

	// Sort commands
	ParseAdmStr(param, cmd1, arg);
	ParseAdmStr(arg, cmd2, cmd3);
	Q_strncpyz(ent->client->pers.cmd1, cmd1, sizeof(ent->client->pers.cmd1));
	Q_strncpyz(ent->client->pers.cmd2, cmd2, sizeof(ent->client->pers.cmd2));
	Q_strncpyz(ent->client->pers.cmd3, cmd3, sizeof(ent->client->pers.cmd3));

	// Command itself is a helper..
	if (_CMD(cmd, "")) {
		sCmd_listCommands(ent);
		wasUsed = qtrue;
	} 
	else {
		for (i = 0; i < cmds; i++) {
			sCMD = &statsCmd[i];

			if (NULL != sCMD->command && _CMD(cmd, sCMD->command)) {
				// Helpers have no function..but they exist for some
				// reason, so print usage info as chances are it's help..
				if (sCMD->pCommand == NULL)
					help = qtrue;

				if (help) {				
						CP(va("print \"^3Command: ^7%s\n^3Info: ^7%s\n^3Usage: ^7%s\n^3Extra: ^7%s\n", cmd, sCMD->help, sCMD->usage, sCMD->extra));
				}
				else {
					sCMD->pCommand(ent, sCMD->fParam);
				}
				wasUsed = qtrue;
			}
		}
	}
	return wasUsed;
}





