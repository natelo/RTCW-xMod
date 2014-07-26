/*
===========================================================================
L0 - g_http.c
Basically all the core (socket) http functionality is here.

Credits: Core handling is ported (with few modifications) directly from s4ndmod.

Created: 14.02 / 2013
Last Updated: 17.02 / 2013
===========================================================================
*/
#include "g_local.h"

#ifdef WIN32
	#include <winsock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#define closesocket close
	#include <unistd.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif

#define _SEND(SOCK, MSG) \
	send(SOCK, MSG, (int)strlen(MSG), 0);

u_int GetHostAddress(char* host) {
	struct hostent *phe;
	char *p;

	phe = gethostbyname(host);

	if (phe == NULL) {
		return 0;
	}

	p = *phe->h_addr_list;
	return *((u_int*)p);
}

int ValidHostChar(char ch) {
	return(isalnum(ch) || ch == '-' || ch == '.' || ch == ':');
}

char *strupr2(char *string) {
	char *s;

	if (string) {
		for (s = string; *s; ++s)
			*s = toupper(*s);
	}
	return string;
}

void ParseURL(char* url, char* protocol, int lprotocol,
	char* host, int lhost, char* request, int lrequest, int *port) {
	char *work, *ptr, *ptr2;

	*protocol = *host = *request = 0;
	*port = 80;

	work = _strdup(url);

	strupr2(work);

	ptr = strchr(work, ':');
	if (ptr != NULL) {
		*(ptr++) = 0;
		strcpy(protocol, work);
	}
	else
	{
		strcpy(protocol, "HTTP");
		ptr = work;
	}

	if ((*ptr == '/') && (*(ptr + 1) == '/')) {
		ptr += 2;
	}

	ptr2 = ptr;
	while (ValidHostChar(*ptr2) && *ptr2)
		ptr2++;

	*ptr2 = 0;
	strcpy(host, ptr);

	strcpy(request, url + (ptr2 - work));

	ptr = strchr(host, ':');
	if (ptr != NULL) {
		*ptr = 0;
		*port = atoi(ptr + 1);
	}

	free(work);
}

/*
===============
httpGet

Sends a query command to a server and returns a reply.
===============
*/
char *httpGet(char *url, char *data) {
#ifdef WIN32
	WSADATA WsaData;
#endif
	struct  sockaddr_in sin;
	int sock;
	char buffer[512];
	char protocol[20], host[256], request[1024];
	int l, port, chars, err, done;
	char *header;
	char *out = NULL;

	// Parse the URL
	ParseURL(url, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port); 

	if (strcmp(protocol, "HTTP")) {		
		return NULL;
	}

#ifdef WIN32
	// Init Winsock
	err = WSAStartup(0x0101, &WsaData);      
	if (err != 0) {
		return NULL;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return NULL;
	}

	sin.sin_family = AF_INET;   
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof( sin))) {
		return NULL;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	header = va(
		"GET %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"User-Agent: rtcw//%s\r\n"
		"Mod: %s\r\n"
		"Token: %s\r\n"
		"Server: %s\r\n"
		"Content-Length: %i\r\n"
		"Host: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Command: %s\r\n"
		"\r\n\r\n",
		request,
		GAME_VERSION,
		GAMEVERSION,
		g_httpToken.string,
		sv_hostname.string,
		strlen(data),
		host,
		data
	);

	// Send Header
	_SEND(sock, header);

	// Data	
	_SEND(sock, "\r\n\r\n");

	// Receive the reply
	chars = 0;
	done = 0;
	while (!done)
	{
		l = recv(sock, buffer, 1, 0);
		if (l < 0) {
			done = 1;
		}

		switch (*buffer)
		{
		case '\r':
			break;
		case '\n':
			if (chars == 0) {
				done = 1;
			}
			chars = 0;
			break;
		default:
			chars++;
			break;
		}
	}

	do
	{
		l = recv(sock, buffer, sizeof(buffer)-1, 0);
		if (l < 0) {
			break;
		}
		*(buffer + l) = 0;

		if (strlen(buffer) > 0)		
			out = va("%s", buffer);		
	} while (l > 0);

	closesocket(sock);	

	AP("chat \"Done..\n\"");

	return out;
}

/*
===============
httpSubmit

Submits data and doesn't care about any replies and simply bails out..
===============
*/
void httpSubmit(char *url, char *data) {
#ifdef WIN32
	WSADATA WsaData;
#endif
	struct  sockaddr_in sin;
	int sock;
	char protocol[20], host[256], request[1024];
	int port, err;
	char *header;

	ParseURL(url, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port);

#ifdef WIN32
	// Init Winsock
	err = WSAStartup(0x0101, &WsaData);
	if (err != 0) {
		return;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		return;
	}

	//Connect to web sever
	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin))) {
		return;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	header = va(
		"POST %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"User-Agent: rtcw//%s\r\n"
		"Mod: %s\r\n"
		"Token: %s\r\n"
		"Server: %s\r\n"
		"Content-Length: %i\r\n"
		"Host: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"\r\n\r\n",
		request,
		GAME_VERSION,
		GAMEVERSION,
		g_httpToken.string,
		sv_hostname.string,
		strlen(data),
		host
	);

	// Header
	_SEND(sock, header);

	// Data
	_SEND(sock, data);  
	_SEND(sock, "\r\n\r\n");

	// We're done..bail out
	closesocket(sock);

	if (g_httpDebug.integer)
		AP("print \"g_httpDebug : Submitted data.\n\"");

	return;
}



/*
===============
httpQuery

Sends a query command to a server and returns a reply.
===============
*/
char *httpQuery(char *url, char *data) {
#ifdef WIN32
	WSADATA WsaData;
#endif
	struct  sockaddr_in sin;
	int sock;
	char buffer[512];
	char protocol[20], host[256], request[1024];
	int l, port, chars, err, done;
	char *header;
	char *out = NULL;
	
	// Parse the URL
	ParseURL(url, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port);

	if (strcmp(protocol, "HTTP")) {
		return out;
	}

#ifdef WIN32
	// Init Winsock
	err = WSAStartup(0x0101, &WsaData);
	if (err != 0) {
		return out;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return out;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin))) {
		return out;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	header = va(
		"POST %s HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"User-Agent: rtcw//%s\r\n"
		"Mod: %s\r\n"
		"Token: %s\r\n"
		"Server: %s\r\n"
		"Content-Length: %d \r\n"
		"Host: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"\r\n\r\n",
		request,
		GAME_VERSION,
		GAMEVERSION,
		g_httpToken.string,
		sv_hostname.string,
		strlen(data) + 8,
		host
		);

	// Send Header
	_SEND(sock, header);

	// Data	
	_SEND(sock, va("cmd=%s\r\n", data));

	// Null it...
	_SEND(sock, "\r\n\r\n");

	// Receive the reply
	chars = 0;
	done = 0;
	while (!done)
	{
		l = recv(sock, buffer, 1, 0);
		if (l < 0) {
			done = 1;
		}

		switch (*buffer)
		{
		case '\r':
			break;
		case '\n':
			if (chars == 0) {
				done = 1;
			}
			chars = 0;
			break;
		default:
			chars++;
			break;
		}
	}

	do
	{
		l = recv(sock, buffer, sizeof(buffer)-1, 0);
		if (l < 0) {
			break;
		}
		*(buffer + l) = 0;

		if (strlen(buffer) > 0)			
			out = va("%s", buffer);

	} while (l > 0);

	closesocket(sock);
	return out;
}

/*
void *globalStats_sendCommand(void *args) {
#ifdef WIN32
	WSADATA WsaData;
#endif
	struct  sockaddr_in sin;
	int sock;
	char buffer[512];
	char protocol[20], host[256], request[1024];
	int l, port, chars, err, done;
	char *header;
	g_http_cmd_t *cmd = (g_http_cmd_t *)args;

	AP("print \"Enter\n\"");
	AP(va("chat \"Cmd: %s\n\"", cmd->cmd));

	// Parse the URL
	ParseURL(g_httpStatsAPI.string, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port);

	if (strcmp(protocol, "HTTP")) {
		AP("print \"HTTP failed\n\"");
		return 0;
	}

#ifdef WIN32
	// Init Winsock
	err = WSAStartup(0x0101, &WsaData);
	if (err != 0) {
		AP("print \"WSAStartup failed\n\"");
		return 0;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		AP("print \"Sock failed\n\"");
		return 0;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin))) {
		AP("print \"Connect failed\n\"");
		return 0;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	AP("print \"Prepping\n\"");

	header = va(
		"POST %s HTTP/1.0\r\n"
		
		"User-Agent: rtcw//%s\r\n"
		"Mod: %s\r\n"
		"Token: %s\r\n"
		"Server: %s\r\n"
		"Content-Length: %d \r\n"
		"Host: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"		
		"\r\n\r\n",
		request,
		GAME_VERSION,
		GAMEVERSION,
		g_httpToken.string,
		sv_hostname.string,
		strlen(cmd->cmd) + 8,
		host		
		);

	// Send Header
	_SEND(sock, header);

	// Data	
	_SEND(sock, va("cmd=%s\r\n", cmd->cmd));
	
	// Null it...
	_SEND(sock, "\r\n\r\n");
	
	// Receive the reply
	chars = 0;
	done = 0;
	while (!done)
	{
		l = recv(sock, buffer, 1, 0);

		if (WSAGetLastError() != 0) {
			AP(va("Error: %s", WSAGetLastError()));
		}

		if (l < 0) {
			done = 1;
		}

		switch (*buffer)
		{
		case '\r':
			break;
		case '\n':
			if (chars == 0) {
				done = 1;
			}
			chars = 0;
			break;
		default:
			chars++;
			break;
		}
	}

	do
	{
		l = recv(sock, buffer, sizeof(buffer)-1, 0);
		if (l < 0) {
			break;
		}
		*(buffer + l) = 0;

		if (strlen(buffer) > 0)
			AP(va("chat \"Reply: %s\n\"", buffer));

	} while (l > 0);	

	closesocket(sock);

	AP("print \"Freeing memory\n\"");
	free(cmd);

	AP("print \"Thread Destroyed\n\"");
	return 0;
}
*/

