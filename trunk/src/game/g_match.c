/*
===========================================================================
g_match.c

Handle match related stuff, much like in et..

Author: Nate 'L0
Created: 8.Sept/13
Updated:
===========================================================================
*/
#include "g_local.h"

/*
=================
FlagBalance

Checks if teams are uneven and favours one with less to claim last take-retake 
till teams even up again, then it unlocks so it can be (re)claimed again.
=================
*/
int FlagBalance( void ) {

	if (!g_balanceFlagRetake.integer)
		return 0;

	sortedActivePlayers();

	// Avoid dealing with check if there's less than 2 players..
	if (level.axisPlayers < 2 && level.alliedPlayers < 2)
		return 0;

	// Even
	if (level.axisPlayers == level.alliedPlayers)
		return 0;
	// Axis (less)
	else if (level.axisPlayers < level.alliedPlayers)
		return 1;
	// Allied (less)
	else if (level.axisPlayers > level.alliedPlayers)
		return 2;
	// We don't know what happen...
	else
		return 0;
}

