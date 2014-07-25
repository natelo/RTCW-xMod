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
L0 - g_http.c
Basically all the core http functionality is here.

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
char *httpGet(char *url, char *cmd) {
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
		"GET "
		"%s"
		" HTTP/1.0\r\n"
		"User-Agent: S4NDMoD/2.4.0\r\n"
		"Host: %s\r\n"
		"Token: %s\r\n"
		"Command: %s\r\n"
		"\r\n\r\n",
		request,
		host,
		g_httpToken.string,
		cmd
	);

	// Send Header
	_SEND(sock, header);

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
		{
			if (g_httpDebug.integer)
				AP(va("print \"g_httpDebug - Got reply: %s \n\"", buffer));

			out = va("%s", buffer);
		}
	} while (l > 0);

	closesocket(sock);	
	return out;
}

/*
===============
httpPost

Submits data and doesn't care about any replies..
===============
*/
void httpPost(char *url, char *data) {
#ifdef WIN32
	WSADATA WsaData;
#endif
	struct  sockaddr_in sin;
	int sock;
	char protocol[20], host[256], request[1024];
	int port, err;
	char mapName[64];
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

	trap_Cvar_VariableStringBuffer("mapname", mapName, sizeof(mapName));

	// Header
	header = va(
		"POST "
		"%s"
		" HTTP/1.0\r\n"
		"Accept: */*\r\n"
		"User-Agent: rtcwx\\%s\\%s\r\n"
		"Accept-Language: en-us\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"Content-Length: %d\r\n"
		"Host: %s\r\n"
		"Token: %s\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n",
		request, 
		GAMEVERSION, sv_hostname.string,
		strlen(data),
		host,
		g_httpToken.string
	);

	// Send Header
	_SEND(sock, header);

	// Send Data	
	_SEND(sock, va("%s\r\n", data));	

	//_Kill it now
	_SEND(sock, "\r\n\r\n");

	// We don't care about replay..
	closesocket(sock);

	if (g_httpDebug.integer)
		AP("print \"g_httpDebug : Posted data.\n\"");

	return;
}
