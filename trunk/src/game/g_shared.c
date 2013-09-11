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
==================
L0 - Splits string into tokens
==================
*/
void Q_Tokenize(char *str, char **splitstr, char *delim) {      
	char *p;      
	int i=0;      

	p = strtok(str,delim);      
	while(p!= NULL)      
	{                
		printf("%s", p);

		//splitstr[i] = malloc(strlen(p) + 1);
		splitstr[i] = G_Alloc(strlen(p) + 1);

		if (splitstr[i])
			strcpy(splitstr[i], p);
		i++;

		p = strtok (NULL, delim);       
	} 
}