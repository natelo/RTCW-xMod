/*
---------------------------
Nate 'L0 - g_http.h
Created: 23.07/14

Hold declarations and structures of all the HTTP related functionality..
---------------------------
*/
#ifndef _G_HTTP
#define _G_HTTP

// HTTP Commands for invoking functionality thru a wrapper
#define HTTP_CLIENT_STATS	"stats"

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
qboolean isHttpCommand(gentity_t *ent, char *cmd1, char *cmd2, char *cmd3);

#endif // _G_HTTP
