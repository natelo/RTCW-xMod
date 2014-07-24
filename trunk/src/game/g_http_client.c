/*
===========================================================================

wolfX GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of wolfX source code.

wolfX Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

wolfX Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with wolfX Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the wolfX Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the wolfX Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
L0 - http_func.c
Basically all the core http functionality is here.

Credits: Core handling is ported (with few modifications) directly from s4ndmod.

Created: 14.02 / 2013
Last Updated: 17.02 / 2013
===========================================================================
*/
#include "g_local.h"

#ifdef INFINITE
#undef INFINITE
#endif

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
httpSubmit

Submits data and only checks if it was accepted otherwise
alternatively, tries once more.
===============
*/
int httpSubmit(char *url, char *data) {
#ifdef WIN32
	WSADATA WsaData;
#endif
	struct  sockaddr_in sin;
	int sock;
	char protocol[20], host[256], request[1024];
	int port, err;
	char mapName[64];

	ParseURL(url, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port);

#ifdef WIN32
	// Init Winsock
	err = WSAStartup(0x0101, &WsaData);
	if (err != 0) {
		return 0;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		return 0;
	}

	//Connect to web sever
	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof(sin))) {
		return 0;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));

	//Start Sending header
	_SEND(sock, "POST ");
	_SEND(sock, request);
	_SEND(sock, " HTTP/1.0\r\n"
		"Accept: */*\r\n");
	_SEND(sock, va("User-Agent: wolfX//%s//%s//%s\r\n", GAMEVERSION, mapName, sv_hostname.string));
	_SEND(sock, "Accept-Language: en-us\r\n");
	_SEND(sock, "Accept-Encoding: gzip, deflate\r\n");
	//_SEND(sock, va("Content-Length: %d\r\n", strlen(data)));
	_SEND(sock, va("Content-Length: %d\r\n", strlen("some fucked up data..\r\n")));
	_SEND(sock, va("Host: %s\r\n", host));
	_SEND(sock, "Content-Type: application/x-www-form-urlencoded\r\n");
	//_SEND(sock, "\r\n\r\n");

	// Data
	//_SEND(sock, va("data=%s", data)); // Sends a large text block.
	_SEND(sock, "User[1]: some fucked up data..\r\n");
	_SEND(sock, "User[2]: some fucked up data..\r\n");
	_SEND(sock, "User[3]: some fucked up data..\r\n");
	_SEND(sock, "User[4]: some fucked up data..\r\n");
	_SEND(sock, "User[5]: some fucked up data..\r\n");
	_SEND(sock, "User[6]: some fucked up data..\r\n");
	_SEND(sock, "User[7]: some fucked up data..\r\n");
	_SEND(sock, "User[8]: some fucked up data..\r\n");
	_SEND(sock, "User[9]: some fucked up data..\r\n");
	_SEND(sock, "User[10]: some fucked up data..\r\n");

	//_SEND(sock, "\r\n");
	_SEND(sock, "\r\n\r\n");

	// We don't care about replay..
	closesocket(sock);

	// As far as game cares, it was send so replay as such..
	return 1;
}

int httpGet(char*url, char*filename) {
#ifdef WIN32
	WSADATA WsaData;
#endif

	struct  sockaddr_in sin;
	int sock;
	char buffer[512];
	char protocol[20], host[256], request[1024];
	int l, port, chars, err;
	int done;
	//FILE*        out;	

	//out = fopen(filename, "w+");

	ParseURL(url, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port); // Parse the URL

	if (strcmp(protocol, "HTTP")) {
		//fclose(out);
		return 0;
	}

#ifdef WIN32
	err = WSAStartup(0x0101, &WsaData);       // Init Winsock
	if (err != 0) {
		return 0;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		return 0;
	}

	sin.sin_family = AF_INET;                                           //Connect to web sever
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof( /*struct sockaddr_in*/ sin))) {
		return 0;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	//Start Sending header
	_SEND(sock, "GET ");
	_SEND(sock, request);
	_SEND(sock, " HTTP/1.0\r\n"
		"User-Agent: S4NDMoD/2.4.0\r\n"
		"Host: ");
	_SEND(sock, host);
	_SEND(sock, "\r\n\r\n");        // Send a blank line to signal end of HTTP headerReceive

	//receive the header from the server really not doing anything with this info...
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
		//fputs(buffer, out);

		if (strlen(buffer) > 0)
			AP(va("chat \"console: Got reply: %s \n\"", buffer));

	} while (l > 0);

	closesocket(sock);
	//fclose(out);
	return 1;

}


void *httpTest(void *args) {
#ifdef WIN32
		WSADATA WsaData;
#endif

	struct  sockaddr_in sin;
	int sock;
	char buffer[512];
	char protocol[20], host[256], request[1024];
	int l, port, chars, err;
	int done;

	ParseURL(g_httpPostURL_chat.string, protocol, sizeof(protocol), host, sizeof(host), request, sizeof(request), &port); // Parse the URL

	if (strcmp(protocol, "HTTP")) {		
		return 0;
	}

#ifdef WIN32
	err = WSAStartup(0x0101, &WsaData);       // Init Winsock
	if (err != 0) {
		return 0;
	}
#endif

	sock = (int)socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		return 0;
	}

	sin.sin_family = AF_INET;                                           //Connect to web sever
	sin.sin_port = htons((unsigned short)port);
	sin.sin_addr.s_addr = GetHostAddress(host);

	if (connect(sock, (struct sockaddr*)&sin, sizeof( /*struct sockaddr_in*/ sin))) {
		return 0;
	}

	if (!*request) {
		strcpy(request, "/");
	}

	//Start Sending header
	_SEND(sock, "GET ");
	_SEND(sock, request);
	_SEND(sock, " HTTP/1.0\r\n"
		"User-Agent: S4NDMoD/2.4.0\r\n"
		"Host: ");
	_SEND(sock, host);
	_SEND(sock, "\r\n\r\n");        // Send a blank line to signal end of HTTP headerReceive

	//receive the header from the server really not doing anything with this info...
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
			AP(va("chat \"console: Got reply: %s \n\"", buffer));

	} while (l > 0);

	closesocket(sock);
	return 0;
}

void testData(void) 
{
	int num_lines = 4;

	g_http_matchinfo_t *post_matchinfo = (g_http_matchinfo_t *)malloc(sizeof(g_http_matchinfo_t));

	post_matchinfo->info_lines = malloc(num_lines * sizeof(char*));
	post_matchinfo->info_lines_lengths = malloc(num_lines * sizeof(int));
	post_matchinfo->num_lines = num_lines;
	Q_strncpyz(post_matchinfo->url, g_etpub_stats_master_url.string, sizeof(post_matchinfo->url));

	post_matchinfo->info_lines_lengths[0] = (strlen("wtf") + 1) * sizeof(char); // +1 for \0 at the end
	post_matchinfo->info_lines[0] = malloc(post_matchinfo->info_lines_lengths[0]);
	Q_strncpyz(post_matchinfo->info_lines[0], "wtf", post_matchinfo->info_lines_lengths[0]);

	// Let's get it now..
	create_thread(httpTest, (void*)post_matchinfo);
}
