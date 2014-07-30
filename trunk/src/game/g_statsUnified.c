/*
===========================================================================
L0 - g_http_stats.c
All essential WebStats functionality resides here.

Created: 29.07 / 14
Last Updated: /
===========================================================================
*/
#include "g_local.h"

/*
==============================
stats_addEntry

Linked list
==============================
*/
void stats_addEntry(char* entry) {
	struct statEntry *node;

	//allocate memory space for this
	node = (struct statEntry *)malloc(sizeof(struct statEntry));

	//strcpy(node->info, entry);
	Q_strncpyz(node->info, entry, sizeof(node->info));

	if (statHead == NULL) {
		statHead = node;
	}
	else {
		statTail->next = node;
	}
	statTail = node;
	node->next = NULL;
}

/*
==========================
stats_shutdown

Clears/frees linked lists
==========================
*/
void stats_shutdown() {
	struct statEntry *node;

	while (statHead != NULL) {
		node = statHead;
		statHead = statHead->next;
		free(node);
	}
}

/*
===========================================
stats_init

for any init. stuff thats needs to be done
===========================================
*/
void stats_init() {
	statNumber = 0; //init statNumber to 0

	//will add begin round to the link list.
	//stats_event_roundBegin();
}