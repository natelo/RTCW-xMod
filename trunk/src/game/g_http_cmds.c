/*
---------------------------
Nate 'L0 - g_http_cmds.c
Created: 06.Sept/14

All http client related commands.
---------------------------
*/
#include "g_local.h"

/*
===========
Stats Structure
===========
*/
void sCmd_top(gentity_t *ent, qboolean fParam) {

	AP(va("chat \"Cmd1: %s - Cmd2: %s - Cmd3: %s \n",
		ent->client->pers.cmd1,	
		ent->client->pers.cmd2,
		ent->client->pers.cmd3
	));

	http_clientCommand(ent, "top", "", fParam);
}

/*
===========
Stats Structure
===========
*/
void sCmd_bottom(gentity_t *ent, qboolean fParam) {

	http_clientCommand(ent, "bottom", "", fParam);
}

/*
===========
Stats Structure
===========
*/
void sCmd_rank(gentity_t *ent, qboolean fParam) {

	http_clientCommand(ent, "rank", "", fParam);
}

/*
===========
Stats Structure
===========
*/
void sCmd_chances(gentity_t *ent, qboolean fParam) {

	http_clientCommand(ent, "chances", "", fParam);
}

/*
===========
Stats Structure
===========
*/
void sCmd_info(gentity_t *ent, qboolean fParam) {

	http_clientCommand(ent, "info", "", fParam);
}

/*
===========
Stats Structure
===========
*/
void sCmd_lastseen(gentity_t *ent, qboolean fParam) {

	http_clientCommand(ent, "lastseen", "", fParam);
}
