/*
---------------------------
Nate 'L0 - g_http.h
Created: 23.07/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _G_HTTP
#define _G_HTTP

// Upload is capped at 10mb..anything more @ your own risk.
#define UFILE_SIZELIMIT 10000000

// Player stats commands
#define PSTATS_GLOBAL "stats"

/*
============
Client commands structure
============
*/
typedef struct {
	char *cmd;
	unsigned int id;
	qboolean toClient;
} g_http_cmd_t;

/*
============
Prototypes
============
*/
//
// g_http.c
//
void http_Submit(char *url, char *data);
void http_SubmitFile(char *url, char *file, qboolean wipe);
char *http_Query(char *url, char *data);

//
// g_http_stats.c
//
void globalStats_submit(void);

//
// g_http_cmds.c
//
void *http_sendQuery(void *args);
qboolean isHttpCommand(gentity_t *ent, char *cmd, char *param, qboolean help);

#endif // _G_HTTP
