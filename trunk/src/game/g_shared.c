/*
===========================================================================
g_shared.c

To avoid modifying q_shared.c as much as possible, I've created this.

Author: Nate 'L0
Created: 11.Sept/13
Updated:
===========================================================================
*/
#include "g_local.h"

extern char *stats_chars[];

/*
==================
L0 - Ported from et: NQ
DecolorString

Remove color characters
==================
*/
void DecolorString( char *in, char *out)
{
	while(*in) {
		if(*in == 27 || *in == '^') {
			in++;		// skip color code
			if(*in) in++;
			continue;
		}
		*out++ = *in++;
	}
	*out = 0;
}

/*
==================
L0 - Wish it would be like in php and i wouldn't need to bother with this..
==================
*/
int is_numeric(const char *p) {
	if (*p) {
		char c;
		while ((c=*p++)) {
			if (!isdigit(c)) return 0;
		}
		return 1;
	}
	return 0;
}

/*
===================
L0 - Str replacer 

Ported from etPub
===================
*/
char *Q_StrReplace(char *haystack, char *needle, char *newp)
{
	static char final[MAX_STRING_CHARS] = {""};
	char dest[MAX_STRING_CHARS] = {""};
	char newStr[MAX_STRING_CHARS] = {""};
	char *destp;
	int needle_len = 0;
	int new_len = 0;

	if(!*haystack) {
		return final;
	}
	if(!*needle) {
		Q_strncpyz(final, haystack, sizeof(final));
		return final;
	}
	if(*newp) {
		Q_strncpyz(newStr, newp, sizeof(newStr));	
	}

	dest[0] = '\0';
	needle_len = strlen(needle);
	new_len = strlen(newStr);
	destp = &dest[0];
	while(*haystack) {
		if(!Q_stricmpn(haystack, needle, needle_len)) {
			Q_strcat(dest, sizeof(dest), newStr);
			haystack += needle_len;
			destp += new_len;
			continue;
		}
		if(MAX_STRING_CHARS > (strlen(dest) + 1)) {
			*destp = *haystack;
			*++destp = '\0';
		}
		haystack++;
	}
	// tjw: don't work with final return value in case haystack 
	//      was pointing at it.
	Q_strncpyz(final, dest, sizeof(final));

	return final;
}

/*
==================
L0 - Strip the chars when need it
==================
*/
void stripChars( char *input, char *output, int cutSize ) {
	int lenght = strlen( input );
	int i = 0, k = 0;

	for ( i = lenght - cutSize; i < lenght; i++ )
		output[k++] = input[i];

	output[k++] = '\0';
}

/*
===========
Sorts names so they're file friendly.

Quite sure this bit is from S4NDMoD or at least derived from.
===========
*/
char *parseNames( char *name )
{
	int i=0, j=0, k=0;
	char parse[MAX_NETNAME + 1];

	Q_strncpyz (parse, "", sizeof( parse ) );
	for (i=0 ; i < MAX_NETNAME + 1; i++)
	{
		for (k=0 ; k < 91 ; k++)
		{
			if(name[i] == stats_chars[k][0])
			{
				parse[j] = name[i];
				j++;
			}
		}
	}
	return va("%s",parse);
}

/*
==================
L0 - Splits string into tokens
==================
*/
void Q_Tokenize(char *str, char **splitstr, char *delim) {
	char *p;
	int i = 0;

	p = strtok(str, delim);
	while (p != NULL)
	{
		printf("%s", p);

		splitstr[i] = G_Alloc(strlen(p) + 1);

		if (splitstr[i])
			strcpy(splitstr[i], p);
		i++;

		p = strtok(NULL, delim);
	}
}

/*
==================
Parse Strings

From S4NDMoD
==================
*/
void ParseStr(const char *strInput, char *strCmd, char *strArgs)
{
	int i = 0, j=0;
	int foundspace=0;

	while(strInput[i] != 0) {
		if (!foundspace) {
			if (strInput[i] == ' ') {
				foundspace = 1;
				strCmd[i]=0;
			} else
				strCmd[i]=strInput[i];
			i++;
		} else { 
			strArgs[j++]=strInput[i++];
		}
	}
	if (!foundspace)
		strCmd[i]=0;

	strArgs[j]=0;
}

/*
==================
TokenList

See if there's a match
==================
*/
qboolean Q_FindToken(char *haystack, char *needle) {

	if (strlen(haystack) && strlen(needle)) {
		char *token;

		while (1)
		{
			token = COM_Parse(&haystack);
			if (!token || !token[0])
				break;

			if (!Q_stricmp(needle, token))
				return qtrue;
		}
	}
	return qfalse;
}
