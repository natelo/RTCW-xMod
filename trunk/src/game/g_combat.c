/*
 * name:		g_combat.c
 *
 * desc:		
 *
*/

#include "g_local.h"

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if ( level.warmupTime ) {
		return;
	}

	// Ridah, no scoring during single player
	// DHM - Nerve :: fix typo
	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		return;
	}
	// done.


	ent->client->ps.persistant[PERS_SCORE] += score;
	if (g_gametype.integer >= GT_TEAM) // JPW NERVE changed to >=
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	int			weapon;
	gentity_t	*drop = 0;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.

	// (SA) always drop what you were switching to
	if(1) {
		if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
			weapon = self->client->pers.cmd.weapon;
		}
		if ( !( COM_BitCheck( self->client->ps.weapons, weapon ) ) ) {
			weapon = WP_NONE;
		}
	}

	// JPW NERVE don't drop these weapon types
	if ((weapon == WP_FLAMETHROWER) || (weapon == WP_GARAND) || (weapon == WP_MAUSER) || (weapon == WP_VENOM))
		weapon = WP_NONE;
	// jpw

	if ( weapon > WP_NONE && weapon < WP_MONSTER_ATTACK1&& self->client->ps.ammo[ BG_FindAmmoForWeapon(weapon)] ) {
		// find the item type for this weapon
		item = BG_FindItemForWeapon( weapon );
		// spawn the item
		
		// Rafael
		if (!(self->client->ps.persistant[PERS_HWEAPON_USE])) {
			drop = Drop_Item( self, item, 0, qfalse );
			// JPW NERVE -- fix ammo counts
			drop->count = self->client->ps.ammoclip[BG_FindClipForWeapon(weapon)];					
			drop->item->quantity = self->client->ps.ammoclip[BG_FindClipForWeapon(weapon)];					
			// jpw
		}
	}

	// L0 - dropping stuff when going to limbo

	// Binocs
	if (self->client->ps.stats[STAT_KEYS] & (1 << INV_BINOCS) && g_dropBinocs.integer){
		item = &bg_itemlist[82];
		drop = Drop_Item(self, item, 0, qfalse);
		drop->think = MagicSink;
		drop->timestamp = level.time + 31200;
		drop->parent = NULL;
	}
		
	// drop medpacks
	if (g_dropHealth.integer) {
		if (self->client->ps.stats[STAT_PLAYER_CLASS] == PC_MEDIC)
		{
			int i;
			item = BG_FindItem("Med Health");
			
			for (i = 0; i < g_dropHealth.integer; i++)
			{
				drop = Drop_Item(self, item, 0, qfalse);
				drop->think = MagicSink;
				drop->timestamp = level.time + 31200;
				drop->parent = NULL;
			}
		}
	}

	// drop ammopacks
	if (g_dropAmmo.integer)	{
		if (self->client->ps.stats[STAT_PLAYER_CLASS] == PC_LT)
		{
			int i;
			item = BG_FindItem("Ammo Pack");
			
			for (i = 0; i < g_dropAmmo.integer; i++)
			{
				drop = Drop_Item(self, item, 0, qfalse);
				drop->think = MagicSink;
				drop->timestamp = level.time + 31200;
				drop->parent = NULL;
			}
		}
	}	

	// drop nades
	if (g_dropNades.integer) {
		if (self->client->ps.stats[STAT_PLAYER_CLASS] == PC_ENGINEER){
			int i;
			if (self->client->sess.sessionTeam == TEAM_RED)
				item = BG_FindItem("Grenades");
			else
				item = BG_FindItem("Pineapples");

			for (i = 0; i < g_dropNades.integer; i++){
				drop = Drop_Item(self, item, 0, qfalse);
				drop->think = MagicSink;
				drop->timestamp = level.time + 31200;
				drop->parent = NULL;
			}
		}
	}  // L0 - end
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );
}


/*
==============
GibHead
==============
*/
void GibHead(gentity_t *self, int killer)
{
	G_AddEvent( self, EV_GIB_HEAD, killer );
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
	gentity_t *other=&g_entities[killer];
	vec3_t dir;

	VectorClear( dir );
	if (other->inuse) {
		if (other->client) {
			VectorSubtract( self->r.currentOrigin, other->r.currentOrigin, dir );
			VectorNormalize( dir );
		} else if (!VectorCompare(other->s.pos.trDelta, vec3_origin)) {
			VectorNormalize2( other->s.pos.trDelta, dir );
		}
	}

	G_AddEvent( self, EV_GIB_PLAYER, DirToByte(dir) );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}

	GibEntity( self, 0 );
}


// these are just for logging, the client prints its own messages
char	*modNames[] = {
	"MOD_UNKNOWN",
	"MOD_SHOTGUN",
	"MOD_GAUNTLET",
	"MOD_MACHINEGUN",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_RAILGUN",
	"MOD_LIGHTNING",
	"MOD_BFG",
	"MOD_BFG_SPLASH",
	"MOD_KNIFE",
	"MOD_KNIFE2",
	"MOD_KNIFE_STEALTH",
	"MOD_LUGER",
	"MOD_COLT",
	"MOD_MP40",
	"MOD_THOMPSON",
	"MOD_STEN",
	"MOD_MAUSER",
	"MOD_SNIPERRIFLE",
	"MOD_GARAND",
	"MOD_SNOOPERSCOPE",
	"MOD_SILENCER",//----(SA)	
	"MOD_AKIMBO",	//----(SA)	
	"MOD_BAR",	//----(SA)	
	"MOD_FG42",
	"MOD_FG42SCOPE",
	"MOD_PANZERFAUST",
	"MOD_ROCKET_LAUNCHER",
	"MOD_GRENADE_LAUNCHER",
	"MOD_VENOM",
	"MOD_VENOM_FULL",
	"MOD_FLAMETHROWER",
	"MOD_TESLA",
	"MOD_SPEARGUN",
	"MOD_SPEARGUN_CO2",
	"MOD_GRENADE_PINEAPPLE",
	"MOD_CROSS",
	"MOD_MORTAR",
	"MOD_MORTAR_SPLASH",
	"MOD_KICKED",
	"MOD_GRABBER",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_GRAPPLE",
	"MOD_EXPLOSIVE",
	"MOD_POISONGAS",
	"MOD_ZOMBIESPIT",
	"MOD_ZOMBIESPIT_SPLASH",
	"MOD_ZOMBIESPIRIT",
	"MOD_ZOMBIESPIRIT_SPLASH",
	"MOD_LOPER_LEAP",
	"MOD_LOPER_GROUND",
	"MOD_LOPER_HIT",
// JPW NERVE
	"MOD_LT_ARTILLERY",
	"MOD_LT_AIRSTRIKE",
	"MOD_ENGINEER",	// not sure if we'll use
	"MOD_MEDIC",		// these like this or not
// jpw
// L0 - Hacks for MODs
	"MOD_ADMKILL",		// Slapped to death || killed by admin
	"MOD_SELFKILL",		// Suicide (not gib!)
	"MOD_THROWKNIFE",	// Killed by knife throw
	"MOD_CHICKEN",		// Funny print when player self kills to avoid being killed
	"MOD_POISONDMED",	// Killed by poison
	"MOD_GOOMBA",		// Killed by some1 landing on his head..
// End
	"MOD_BAT"
};

/*
==================
player_die
==================
*/
void limbo( gentity_t *ent, qboolean makeCorpse ); // JPW NERVE

void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
  // TTimo might be used uninitialized
	int			contents = 0;
	int			killer;
	int			i;
	char		*killerName, *obit;
	qboolean	nogib = qtrue;
	gitem_t		*item=NULL; // JPW NERVE for flag drop
	vec3_t		launchvel,launchspot; // JPW NERVE
	gentity_t	*flag; // JPW NERVE

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}

	// L0 - Shortcuts	
	if(attacker && attacker->client) {
		self->client->pers.lastkiller_client = attacker->s.clientNum;
		attacker->client->pers.lastkilled_client = self->s.clientNum;
	} // End

	self->client->ps.pm_type = PM_DEAD;

	G_AddEvent( self, EV_STOPSTREAMINGSOUND, 0);

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

// L0 - Hacks for custom MOD's
	if ( meansOfDeath == MOD_ADMKILL && g_gamestate.integer == GS_PLAYING) {
		AP(va("print \"%s ^7was killed by Admin.\n\"", self->client->pers.netname));
	}

	if ( meansOfDeath == MOD_SELFKILL && g_gamestate.integer == GS_PLAYING) {
		int r = rand() %2; // randomize messages
			
		if (r == 0)			
			AP(va("print \"%s ^7slit his throat.\n\"", self->client->pers.netname));
		else if (r == 1)
			AP(va("print \"%s ^7commited suicide.\n\"", self->client->pers.netname));
	}

	if ( meansOfDeath == MOD_THROWKNIFE && g_gamestate.integer == GS_PLAYING) {
		AP(va("print \"%s ^7was impaled by %s^7s throwing knife.\n\"", self->client->pers.netname, attacker->client->pers.netname));
		attacker->client->pers.knifeKills++;	
		write_RoundStats(attacker->client->pers.netname, attacker->client->pers.knifeKills, ROUND_KNIFETHROW);
	}

	if ( meansOfDeath == MOD_CHICKEN && g_gamestate.integer == GS_PLAYING) {
		AP(va("print \"%s ^6was scared to death by ^7%s^7.\n\"", self->client->pers.netname, attacker->client->pers.netname));
		self->client->pers.chicken++;
		write_RoundStats(self->client->pers.netname, self->client->pers.chicken, ROUND_CHICKEN);

		// Give props to attacker
		attacker->client->pers.kills++;
		attacker->client->pers.lifeKills++;
		write_RoundStats(attacker->client->pers.netname, attacker->client->pers.kills, ROUND_KILLS);
		write_RoundStats(attacker->client->pers.netname, attacker->client->pers.lifeKills, ROUND_KILLPEAK);
	}

	// If person gets stabbed use custom sound from soundpack
	// it's broadcasted to victim and heard only if standing near victim...
	if ( meansOfDeath == MOD_KNIFE_STEALTH && !OnSameTeam(self, attacker) && g_fastStabSound.integer) {
		int r = rand() %2; 
		char *snd;

		if (r == 0)
			snd = "stab.wav";
		else
			snd = "stab_alt.wav";

		APRS(self, va("xmod/sound/game/events/%s", ((g_fastStabSound.integer == 1) ? "stab.wav" : 
			((g_fastStabSound.integer == 2) ? "stab_alt.wav" : snd)	)));

		attacker->client->pers.fastStabs++;
		write_RoundStats(attacker->client->pers.netname, attacker->client->pers.fastStabs, ROUND_FASTSTABS);
	}  

	if ( meansOfDeath == MOD_POISONDMED && g_gamestate.integer == GS_PLAYING)  {
		int r = rand() %2; 
		
		if (r == 0)
			AP(va( "print \"%s ^7was poisoned by %s^7.\n\"", self->client->pers.netname, attacker->client->pers.netname)); 
		else if (r == 1)
			AP(va( "print \"%s ^7tasted %s^7's poison.\n\"", self->client->pers.netname, attacker->client->pers.netname)); 

		// Stats
		attacker->client->pers.poison++; 

		write_RoundStats(attacker->client->pers.netname, attacker->client->pers.poison, ROUND_POISON);
	}

	if (meansOfDeath == MOD_GOOMBA && g_gamestate.integer == GS_PLAYING)  {
		AP(va("print \"%s ^7experienced death from above by %s^7.\n\"", self->client->pers.netname, attacker->client->pers.netname));
		attacker->client->pers.goomba++;
		write_RoundStats(attacker->client->pers.netname, attacker->client->pers.goomba, ROUND_GOOMBAS);
	}
// Mod hacks ends here

	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[ meansOfDeath ];
	}

	// L0 - Don't bother in warmup etc..
	if (g_gamestate.integer == GS_PLAYING)
	{
		G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", 
			killer, self->s.number, meansOfDeath, killerName, 
			self->client->pers.netname, obit );
	}

	// L0 - Death Sprees
	stats_DeathSpree( self ); 		

	// L0 - Stats
	if (attacker && attacker->client && g_gamestate.integer == GS_PLAYING) {
		// Life kills & death spress
		if (!OnSameTeam(attacker, self)) {		
			attacker->client->pers.kills++;	
			attacker->client->pers.lifeKills++; 

			write_RoundStats(attacker->client->pers.netname, attacker->client->pers.kills, ROUND_KILLS);
			write_RoundStats(attacker->client->pers.netname, attacker->client->pers.lifeKills, ROUND_KILLPEAK);

			if (g_mapStats.integer == 1)
				write_MapStats(attacker, attacker->client->pers.kills, MAP_KILLER);
			else if (g_mapStats.integer == 2)
				write_MapStats(attacker, attacker->client->pers.kills, MAP_KILLING_SPREE);

		// Count teamkill
		} else {
			// Admin bot - teamKills
			if (attacker != self) 
			{
				SB_maxTeamKill(attacker);

				write_RoundStats(attacker->client->pers.netname, attacker->client->pers.teamKills, ROUND_TEAMKILLS);
			}
		}
	} 

	// L0 - Stats
	if (attacker && attacker->client)
	{
		if (!OnSameTeam(attacker, self) && (attacker->client->pers.spreeDeaths > attacker->client->pers.lifeDeathsPeak))
			attacker->client->pers.lifeDeathsPeak = attacker->client->pers.spreeDeaths;

		if (!OnSameTeam(attacker, self))
			attacker->client->pers.spreeDeaths = 0;
	}

	// L0 - (a nasty hack) Don't send this if there's a custom MOD
	if (meansOfDeath != MOD_ADMKILL) { 
	if (meansOfDeath != MOD_SELFKILL ) {
	if (meansOfDeath != MOD_THROWKNIFE) {
	if (meansOfDeath != MOD_CHICKEN) {
	if (meansOfDeath != MOD_POISONDMED) {

		// broadcast the death event to everyone
		// L0 - Don't bother in warmup..
		if (g_gamestate.integer == GS_PLAYING)
		{
			ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
			ent->s.eventParm = meansOfDeath;
			ent->s.otherEntityNum = self->s.number;
			ent->s.otherEntityNum2 = killer;
			ent->r.svFlags = SVF_BROADCAST;	// send to everyone
		}

	}}}}}

	self->enemy = attacker;

	self->client->ps.persistant[PERS_KILLED]++;

	// L0 - Stats
	self->client->pers.deaths++;
	self->client->pers.spreeDeaths++;
	write_RoundStats(self->client->pers.netname, self->client->pers.deaths, ROUND_DEATHS);	
	write_RoundStats(self->client->pers.netname, self->client->pers.spreeDeaths, ROUND_DEATHPEAK);	

	if (g_mapStats.integer == 3)
		write_MapStats(self, self->client->pers.deaths, MAP_VICTIM);
	else if (g_mapStats.integer == 4)
		write_MapStats(self, self->client->pers.deaths, MAP_DEATH_SPREE);
	// End

	// L0 - spies
	if ( self->client->ps.isSpy )
	{
		self->client->ps.isSpy = qfalse;
		ClientUserinfoChanged(self->client->ps.clientNum);
	} 

// JPW NERVE -- if player is holding ticking grenade, drop it
	if (g_gametype.integer != GT_SINGLE_PLAYER)
		if ((self->client->ps.grenadeTimeLeft) && (self->s.weapon != WP_DYNAMITE)) {
			launchvel[0] = crandom();
			launchvel[1] = crandom();
			launchvel[2] = random();
			VectorScale( launchvel, 160, launchvel );
			VectorCopy(self->r.currentOrigin, launchspot);
			launchspot[2] += 40;
			fire_grenade(self, launchspot, launchvel, self->s.weapon);
		}
// jpw

	if (attacker && attacker->client) {
		if ( attacker == self || OnSameTeam (self, attacker ) ) {

			// DHM - Nerve :: Complaint lodging
			if ( attacker != self && level.warmupTime <= 0 ) {
				if ( attacker->client->pers.localClient ) {
					trap_SendServerCommand( self-g_entities, "complaint -4" );
				}
				else {
					trap_SendServerCommand( self-g_entities, va( "complaint %i", attacker->s.number ) );
					self->client->pers.complaintClient = attacker->s.clientNum;
					self->client->pers.complaintEndTime = level.time + 20500;
				}
			}
			// dhm

			// JPW NERVE
			if (g_gametype.integer >= GT_WOLF) // high penalty to offset medic heal
				AddScore(attacker, WOLF_FRIENDLY_PENALTY); 
			else
			// jpw
				AddScore( attacker, -1 );

			// L0 - Life stats
			if (g_showLifeStats.integer) {
				float acc = 0.00f; 
				
				acc = (self->client->pers.lifeAcc_shots == 0) ? 
					0.00 : ((float)self->client->pers.lifeAcc_hits / (float)self->client->pers.lifeAcc_shots ) * 100.00f ;	
					
				// Class based..
				if (self->client->ps.stats[PC_MEDIC])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Rev:^3%d ^7Acc:^3%2.2f\n\"",
						self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.lifeRevives, acc));
				else if (self->client->ps.stats[PC_LT])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7AmmoGiv:^3%d ^7Acc:^3%2.2f\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.ammoPacks, acc));
				else if (self->client->ps.stats[PC_ENGINEER])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Gibs: ^3%d ^7Acc:^3%2.2f\n\"",
						self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.gibs, acc));
				else
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Gibs: ^3%d ^7Acc:^3%2.2f\n\"",
						self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.gibs, acc));
			} // End

		} else {
			// JPW NERVE -- mostly added as conveneience so we can tweak from the #defines all in one place
			if (g_gametype.integer >= GT_WOLF)
			{
				AddScore(attacker, WOLF_FRAG_BONUS);

				// L0 - Stats
				stats_DoubleKill( attacker, meansOfDeath );		// Double kills
				stats_KillingSprees(attacker, 0);				// Overall killing sprees
				stats_KillerSpree(attacker, 0);					// Most Kills per life
			}
			else
			{
			// jpw
				AddScore( attacker, 1 );

				// L0 - Stats
				stats_DoubleKill( attacker, meansOfDeath );		// Double kills
				stats_KillingSprees(attacker, 0);				// Overall killing sprees
				stats_KillerSpree(attacker, 0);					// Most Kills per life
			}

			attacker->client->lastKillTime = level.time;
			// L0 - Last blood
			if (g_showLastBlood.integer)
			{
				Q_strncpyz ( level.lastKiller, attacker->client->pers.netname, sizeof( level.lastKiller ) );
				Q_strncpyz ( level.lastVictim, self->client->pers.netname, sizeof( level.lastVictim ) );
			}

			// L0 - Life stats
			if (g_showLifeStats.integer) {
				float acc = 0.00f; 
				
				acc = (self->client->pers.lifeAcc_shots == 0) ? 
					0.00 : ((float)self->client->pers.lifeAcc_hits / (float)self->client->pers.lifeAcc_shots ) * 100.00f ;	

				// Class based..
				if (self->client->ps.stats[PC_MEDIC])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Rev:^3%d ^7Acc:^3%2.2f ^7Killer: %s^3(%ihp)\n\"",
						self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.lifeRevives, acc, attacker->client->pers.netname, attacker->health));
				else if (self->client->ps.stats[PC_LT])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7AmmoGiv:^3%d ^7Acc:^3%2.2f ^7Killer: %s^3(%ihp)\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.ammoPacks, acc, attacker->client->pers.netname, attacker->health));
				else if (self->client->ps.stats[PC_ENGINEER])
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Gibs: ^3%d ^7Acc:^3%2.2f ^7Killer: %s^3(%ihp)\n\"",
						self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.gibs, acc, attacker->client->pers.netname, attacker->health));
				else
					CPx(self-g_entities, va("chat \"^3Last life: ^7Kills:^3%d ^7Hs:^3%d ^7Gibs: ^3%d ^7Acc:^3%2.2f ^7Killer: %s^3(%ihp)\n\"",
					self->client->pers.lifeKills, self->client->pers.lifeHeadshots, self->client->pers.gibs, acc, attacker->client->pers.netname, attacker->health));
			} // End
		}
	} else {
		AddScore( self, -1 );
	}

	// Add team bonuses
	Team_FragBonuses(self, inflictor, attacker);

	// L0 - AB low score
	// TODO: Add mod_switchteam check..
	SB_minLowScore(self);

	// if client is in a nodrop area, don't drop anything
// JPW NERVE new drop behavior
	if (g_gametype.integer == GT_SINGLE_PLAYER) {	// only drop here in single player; in multiplayer, drop @ limbo
		contents = trap_PointContents( self->r.currentOrigin, -1 );
		if ( !( contents & CONTENTS_NODROP ) ) {
			TossClientItems( self );
		}
	}

	// drop flag regardless
	if (g_gametype.integer != GT_SINGLE_PLAYER) {
		if (self->client->ps.powerups[PW_REDFLAG]) {
			item = BG_FindItem("Red Flag");
			if (!item)
				item = BG_FindItem("Objective");

			self->client->ps.powerups[PW_REDFLAG] = 0;
		}
		if (self->client->ps.powerups[PW_BLUEFLAG]) {
			item = BG_FindItem("Blue Flag");
			if (!item)
				item = BG_FindItem("Objective");

			self->client->ps.powerups[PW_BLUEFLAG] = 0;
		}
  
		if (item) {
			launchvel[0] = crandom()*20;
			launchvel[1] = crandom()*20;
			launchvel[2] = 10+random()*10;

			flag = LaunchItem(item,self->r.currentOrigin,launchvel,self->s.number);
			flag->s.modelindex2 = self->s.otherEntityNum2;// JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
			flag->message = self->message;	// DHM - Nerve :: also restore item name
			// Clear out player's temp copies
			self->s.otherEntityNum2 = 0;
			self->message = NULL;
		}

		// send a fancy "MEDIC!" scream.  Sissies, ain' they?
		if (self->client != NULL) {
			if (self->health > GIB_HEALTH && meansOfDeath != MOD_SUICIDE ) {

				if (self->client->sess.sessionTeam == TEAM_RED) {
					if (random() > 0.5)
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/axis/g-medic2.wav" ));
					else
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/axis/g-medic3.wav" ));
				}
				else {
					if (random() > 0.5)
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/allies/a-medic3.wav" ));
					else
						G_AddEvent( self, EV_GENERAL_SOUND, G_SoundIndex( "sound/multiplayer/allies/a-medic2.wav" ));
				}
			}
		}
	}
// jpw

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed
	self->r.contents = CONTENTS_CORPSE;

	self->s.powerups = 0;
// JPW NERVE -- only corpse in SP; in MP, need CONTENTS_BODY so medic can operate
	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		self->s.weapon = WP_NONE;
		self->s.angles[0] = 0;
	}
	else {
		self->client->limboDropWeapon = self->s.weapon; // store this so it can be dropped in limbo
	}
// jpw
	self->s.angles[2] = 0;
	LookAtKiller (self, inflictor, attacker);

	VectorCopy( self->s.angles, self->client->ps.viewangles );
	self->s.loopSound = 0;

	trap_UnlinkEntity( self );
	self->r.maxs[2] = 0;
	self->client->ps.maxs[2] = 0;
	trap_LinkEntity( self );

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 800;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// never gib in a nodrop
	if ( self->health <= GIB_HEALTH && !(contents & CONTENTS_NODROP) ) {
			GibEntity( self, killer );
			nogib = qfalse;
	}

	if(nogib){
		// normal death
		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH+1;
		}

// JPW NERVE for medic
		self->client->medicHealAmt = 0;
// jpw

		// DHM - Play death animation, and set pm_time to delay 'fallen' animation
		self->client->ps.pm_time = BG_AnimScriptEvent( &self->client->ps, ANIM_ET_DEATH, qfalse, qtrue );

		G_AddEvent( self, EV_DEATH1 + 1, killer );

		// the body can still be gibbed
		self->die = body_die;
	}

	trap_LinkEntity (self);

	// L0 - show First Blood
	stats_FirstBlood (self, attacker);

	if ( g_gametype.integer >= GT_WOLF && meansOfDeath == MOD_SUICIDE ) {
		limbo( self, qtrue );
	}

	// L0 - reset chicken test so it doesn't spam when not need it
	self->client->lasthurt_time=0;
	self->client->lasthurt_client=ENTITYNUM_NONE;
	self->client->lasthurt_mod=0;
	// End
}


/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			count;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil( damage * ARMOR_PROTECTION );
	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

qboolean IsHeadShotWeapon (int mod, qboolean aicharacter)
{
	if(aicharacter) {		// ai's are allowed headshots from these weapons
		if(	mod == MOD_SNIPERRIFLE ||
			mod == MOD_SNOOPERSCOPE)
			return qtrue;

		return qfalse;
	}

							// players are allowed headshots from these weapons
	if (	mod == MOD_LUGER ||
			mod == MOD_COLT ||
			mod == MOD_AKIMBO ||	//----(SA)	added
			mod == MOD_MP40 ||
			mod == MOD_THOMPSON ||
			mod == MOD_STEN ||
			mod == MOD_BAR ||
			mod == MOD_FG42 ||
			mod == MOD_FG42SCOPE ||
			mod == MOD_MAUSER ||
			mod == MOD_GARAND || // JPW NERVE this was left out
			mod == MOD_SNIPERRIFLE ||
			mod == MOD_SNOOPERSCOPE ||
			mod == MOD_SILENCER ||	//----(SA)	modified
			mod == MOD_SNIPERRIFLE)
		return qtrue;

	return qfalse;
}

qboolean IsHeadShot (gentity_t *targ, qboolean isAICharacter, vec3_t dir, vec3_t point, int mod ) 
{
	gentity_t	*head;
	trace_t		tr;
	vec3_t		start, end;
	gentity_t	*traceEnt;
	orientation_t or;			// DHM - Nerve

	qboolean head_shot_weapon = qfalse;

	// not a player or critter so bail
	if (!(targ->client))
		return qfalse;

	if (targ->health <= 0)
		return qfalse;

	head_shot_weapon = IsHeadShotWeapon (mod, isAICharacter);

	if (head_shot_weapon)
	{
		head = G_Spawn ();

		if (trap_GetTag( targ->s.number, "tag_head", &or )) {
			G_SetOrigin( head, or.origin );
		}
		else {
			float height, dest;
			vec3_t v, angles, forward, up, right;
			VectorClear(v);	// L0 - Clear stuff

			G_SetOrigin (head, targ->r.currentOrigin); 

			if (targ->client->ps.pm_flags & PMF_DUCKED)	// closer fake offset for 'head' box when crouching
				height = targ->client->ps.crouchViewHeight - 12;
			else
				height = targ->client->ps.viewheight;

			// NERVE - SMF - this matches more closely with WolfMP models
			VectorCopy( targ->client->ps.viewangles, angles );
			if ( angles[PITCH] > 180 ) {
				dest = (-360 + angles[PITCH]) * 0.75;
			} else {
				dest = angles[PITCH] * 0.75;
			}
			angles[PITCH] = dest;

			AngleVectors( angles, forward, right, up );
			VectorScale( forward, 5, v );			
			VectorMA( v, 18, up, v );
			//VectorMA(v, 5, right, v); // L0 - Align better

			VectorAdd( v, head->r.currentOrigin, head->r.currentOrigin );
			head->r.currentOrigin[2] += height / 2;
			// -NERVE - SMF
		}

		VectorCopy (head->r.currentOrigin, head->s.origin);
		VectorCopy (targ->r.currentAngles, head->s.angles); 
		VectorCopy (head->s.angles, head->s.apos.trBase);
		VectorCopy (head->s.angles, head->s.apos.trDelta);	
		VectorSet(head->r.mins, -6, -6, -2);
		VectorSet(head->r.maxs, 6, 6, 10); // changed this z from 0 to 6
		head->clipmask = CONTENTS_SOLID;
		head->r.contents = CONTENTS_SOLID;

		trap_LinkEntity (head);
	
		// trace another shot see if we hit the head
		VectorCopy (point, start);
		VectorMA (start, 64, dir, end);
		trap_Trace (&tr, start, NULL, NULL, end, targ->s.number, MASK_SHOT);
			
		traceEnt = &g_entities[ tr.entityNum ];

		if(g_debugBullets.integer >= 3) {	// show hit player head bb
			gentity_t *tent;
			vec3_t b1, b2;
			VectorCopy(head->r.currentOrigin, b1);
			VectorCopy(head->r.currentOrigin, b2);
			VectorAdd(b1, head->r.mins, b1);
			VectorAdd(b2, head->r.maxs, b2);
			tent = G_TempEntity( b1, EV_RAILTRAIL );
			VectorCopy(b2, tent->s.origin2);
			tent->s.dmgFlags = 1;

											// show headshot trace
			// end the headshot trace at the head box if it hits
			if(tr.fraction != 1) {
				VectorMA(start, (tr.fraction * 64), dir, end);
			}
			tent = G_TempEntity( start, EV_RAILTRAIL );
			VectorCopy(end, tent->s.origin2);
			tent->s.dmgFlags = 0;
		}

		G_FreeEntity (head);

		if (traceEnt == head) {
			level.totalHeadshots++;			// NERVE - SMF
			return qtrue;
		}
		else
			level.missedHeadshots++;	// NERVE - SMF
	}

	return qfalse;
}

gentity_t* G_BuildHead(gentity_t *ent) {
	gentity_t* head;
	orientation_t or;			// DHM - Nerve

	head = G_Spawn ();

	if (trap_GetTag( ent->s.number, "tag_head", &or )) {
		G_SetOrigin( head, or.origin );
	} else {
		float height, dest;
		vec3_t v, angles, forward, up, right;
		VectorClear(v);	// L0 - Clear stuff

		G_SetOrigin (head, ent->r.currentOrigin); 

		if (ent->client->ps.pm_flags & PMF_DUCKED)	// closer fake offset for 'head' box when crouching
			height = ent->client->ps.crouchViewHeight - 12;
		else
			height = ent->client->ps.viewheight;

		// NERVE - SMF - this matches more closely with WolfMP models
		VectorCopy( ent->client->ps.viewangles, angles );
		if ( angles[PITCH] > 180 ) {
			dest = (-360 + angles[PITCH]) * 0.75;
		} else {
			dest = angles[PITCH] * 0.75;
		}
		angles[PITCH] = dest;

		AngleVectors( angles, forward, right, up );
		VectorScale( forward, 5, v );
		VectorMA( v, 18, up, v );
		//VectorMA(v, 5, right, v); // L0 - Align better

		VectorAdd( v, head->r.currentOrigin, head->r.currentOrigin );
		head->r.currentOrigin[2] += height / 2;
		// -NERVE - SMF
	}

	VectorCopy (head->r.currentOrigin, head->s.origin);
	VectorCopy (ent->r.currentAngles, head->s.angles); 
	VectorCopy (head->s.angles, head->s.apos.trBase);
	VectorCopy (head->s.angles, head->s.apos.trDelta);
	VectorSet(head->r.mins, -6, -6, -2);
	VectorSet(head->r.maxs, 6, 6, 10); // changed this z from 0 to 6
	head->clipmask = CONTENTS_SOLID;
	head->r.contents = CONTENTS_SOLID;
	head->parent = ent;
	head->s.eType = ET_TEMPHEAD;

	trap_LinkEntity (head);
	
	return head;
}

/*
==============
G_ArmorDamage
	brokeparts is how many should be broken off now
	curbroke is how many are broken
	the difference is how many to pop off this time
==============
*/
void G_ArmorDamage(gentity_t *targ) {
	int brokeparts, curbroke;
	int numParts;
	int dmgbits = 16;	// 32/2;
	int i;

	if(!targ->client)
		return;

	if(targ->s.aiChar == AICHAR_PROTOSOLDIER ) {
		numParts = 9;
	} else if(targ->s.aiChar == AICHAR_SUPERSOLDIER ) {
		numParts = 14;
	} else if(targ->s.aiChar == AICHAR_HEINRICH ) {
		numParts = 20;
	} else {
		return;
	}

	if(numParts > dmgbits)
		numParts = dmgbits;	// lock this down so it doesn't overwrite any bits that it shouldn't.  TODO: fix this


	// determined here (on server) by location of hit and existing armor, you're updating here so 
	// the client knows which pieces are still in place, and by difference with previous state, which
	// pieces to play an effect where the part is blown off.
	// Need to do it here so we have info on where the hit registered (head, torso, legs or if we go with more detail; arm, leg, chest, codpiece, etc)

	// ... Ick, just discovered that the refined hit detection ("hit nearest to which tag") is clientside...

	// For now, I'll randomly pick a part that hasn't been cleared.  This might end up looking okay, and we won't need the refined hits.
	//	however, we still have control on the server-side of which parts come off, regardless of what shceme is used.

	brokeparts = (int)( (1 - ((float)(targ->health) / (float)(targ->client->ps.stats[STAT_MAX_HEALTH])) ) * numParts);

	if(brokeparts && ( (targ->s.dmgFlags & ((1<<numParts)-1)) != (1<<numParts)-1) ) {	// there are still parts left to clear

		// how many are removed already?
		curbroke = 0;
		for(i=0;i<numParts;i++) {
			if(targ->s.dmgFlags & (1<<i))
				curbroke++;
		}

		// need to remove more
		if(brokeparts-curbroke >= 1 && curbroke < numParts) {
			for(i=0;i<(brokeparts-curbroke);i++) {
				int remove = rand()%(numParts);

				if(!( (targ->s.dmgFlags & ((1<<numParts)-1)) != (1<<numParts)-1))	// no parts are available any more
					break;

				// FIXME: lose the 'while' loop.  Still should be safe though, since the check above verifies that it will eventually find a valid part
				while(targ->s.dmgFlags & (1<<remove)){
					remove = rand()%(numParts);
				}

				targ->s.dmgFlags |= (1<<remove);	// turn off 'undamaged' part
				if((int)(random()+0.5))							// choose one of two possible replacements
					targ->s.dmgFlags |= (1<<(numParts + remove));
			}
		}
	}
}

/*
==============
L0 

Hitsounds
Note that it requires pack for it.
TODO: Hook this under colors?
==============
*/
void Hitsounds( gentity_t *targ, gentity_t *attacker, qboolean body ) {

	qboolean 	onSameTeam = OnSameTeam( targ, attacker); 
	gentity_t	*te;

	if (g_hitsounds.integer) {

		// if player is hurting him self don't give any sounds
		if (targ->client == attacker->client) {
			return;  // this happens at flaming your self... just return silence...			
		}

		// if team mate
		if (targ->client && attacker->client && onSameTeam ) {

			if (!(attacker->client->sess.clientFlags & CFLAGS_HITSOUNDS)) {
				return;
			}

			te = G_TempEntity( attacker->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND );			
			te->s.eventParm = G_SoundIndex("xmod/sound/game/hitTeam.wav");			
			te->s.teamNum = attacker->s.clientNum;
		}

		// If enemy
		else if ( targ &&
				targ->client &&
				attacker &&
				attacker->client &&
				attacker->s.number != ENTITYNUM_NONE &&
				attacker->s.number != ENTITYNUM_WORLD &&
				attacker != targ &&
				g_gamestate.integer == GS_PLAYING &&
				!onSameTeam )	
		{   
			if (!(attacker->client->sess.clientFlags & CFLAGS_HITSOUNDS)) {
				return;
			}

			te = G_TempEntity( attacker->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND );
			if (body)
				te->s.eventParm = G_SoundIndex("xmod/sound/game/hit.wav");
			else
				te->s.eventParm = G_SoundIndex("xmod/sound/game/hitH.wav");
			te->s.teamNum = attacker->s.clientNum;
		}
	}
}

/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	int			take;
//	int			asave; // L0 - Armor is out..
	int			knockback;

	if (!targ->takedamage) {
		return;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued || g_gamestate.integer != GS_PLAYING ) {
		// L0 - warmup damage
		if (!g_warmupDamage.integer)
			return;
	}

	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

// JPW NERVE
	if ((targ->waterlevel >= 3) && (mod == MOD_FLAMETHROWER))
		return;
// jpw

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER && !(targ->aiName) && !(targ->isProp) && !targ->scriptName) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			targ->use( targ, inflictor, attacker );
		}
		return;
	}

	if ( targ->s.eType == ET_MOVER && targ->aiName && !(targ->isProp) && !targ->scriptName)
	{
		switch (mod) {
		case MOD_GRENADE:
		case MOD_GRENADE_SPLASH:
		case MOD_ROCKET:
		case MOD_ROCKET_SPLASH:
			break;
		default:
			return;	// no damage from other weapons
		}
	}
	else if ( targ->s.eType == ET_EXPLOSIVE ) 
	{
		// 32 Explosive
		// 64 Dynamite only
		if ((targ->spawnflags & 32) || (targ->spawnflags & 64))
		{
			switch (mod) {
			case MOD_GRENADE:
			case MOD_GRENADE_SPLASH:
			case MOD_ROCKET:
			case MOD_ROCKET_SPLASH:
			case MOD_AIRSTRIKE:
			case MOD_GRENADE_PINEAPPLE:
			case MOD_MORTAR:
			case MOD_MORTAR_SPLASH:
			case MOD_EXPLOSIVE:
				if (targ->spawnflags & 64)
					return;
				
				break;

			case MOD_DYNAMITE:
			case MOD_DYNAMITE_SPLASH:
				break;
			
			default: 
				return;
			}
		}
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	// L0 - Now by default knockback is set to 100 (was 1000) so if it's not touched
	// multiply nade and AS to 1000 so it acts and feels like default
	if ( dflags & DAMAGE_RADIUS ) {
		if (g_knockback.integer == 100 )
			knockback *= 10; 
	} // End
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client && (g_friendlyFire.integer || !OnSameTeam(targ, attacker)) ) {
		vec3_t	kvel;
		float	mass;

		mass = 200;

		if (mod == MOD_LIGHTNING && !((level.time+targ->s.number*50)%400)) {
			knockback = 60;
			dir[2] = 0.3;
		}

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		if (targ == attacker && !(	mod != MOD_ROCKET &&
									mod != MOD_ROCKET_SPLASH &&
									mod != MOD_GRENADE &&
									mod != MOD_GRENADE_SPLASH &&
									mod != MOD_DYNAMITE))
		{
			targ->client->ps.velocity[2] *= 0.25;
		}

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( targ != attacker && OnSameTeam (targ, attacker)  ) {
			if ( !g_friendlyFire.integer ) {
				return;
			}
		}

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}

		// RF, warzombie defense position is basically godmode for the time being
		if ( targ->flags & FL_DEFENSE_GUARD ) {
			return;
		}

		// check for invulnerability // (SA) moved from below so DAMAGE_NO_PROTECTION will still work
		if ( client && client->ps.powerups[PW_INVULNERABLE] ) {	//----(SA)	added
			return;
		}
		
	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	if ( client && client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( dflags & DAMAGE_RADIUS ) {
			return;
		}
		damage *= 0.5;
	}

	// add to the attacker's hit counter
	if ( attacker->client && targ != attacker && targ->health > 0 ) {
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS] -= damage;
			// L0 - Admin bot, Team bleed
			attacker->client->pers.sb_teamBleed++;
			SB_maxTeamBleed(attacker);			
		} else {
			attacker->client->ps.persistant[PERS_HITS] += damage;
		}
	}

	if ( damage < 1 ) {
		damage = 1;
	}
	take = damage;

	// L0 - Hitsounds (body)
	Hitsounds( targ, attacker, qtrue);

	// save some from armor
	// L0 - Not for MP..
	/*
	asave = CheckArmor (targ, take, dflags);
	take -= asave;
	*/

	if ( IsHeadShot(targ, qfalse, dir, point, mod) ) {

		if (take*2 < 50) // head shots, all weapons, do minimum 50 points damage
			take = 50;
		else
			take *= 2; // sniper rifles can do full-kill (and knock into limbo)

		if(!(targ->client->ps.eFlags & EF_HEADSHOT))	// only toss hat on first headshot
			G_AddEvent( targ, EV_LOSE_HAT, DirToByte(dir) );

		targ->client->ps.eFlags |= EF_HEADSHOT;

		// L0 - Hitsounds (head)
		Hitsounds( targ, attacker, qfalse);

		// L0 - check for First Headshot
		stats_FirstHeadshot (attacker, targ); 

		// L0 - Stats
		if (!OnSameTeam(attacker, targ))
		{
			attacker->client->pers.headshots++;
			attacker->client->pers.lifeHeadshots++;

			write_RoundStats(attacker->client->pers.netname, attacker->client->pers.headshots, ROUND_HEADSHOTS);

			if (g_mapStats.integer == 6)
				write_MapStats(attacker, attacker->client->pers.deaths, MAP_HEADSHOTS);
		}
	}
	// L0 - Headshots only mode..
	else if ( (targ && targ->client) && 
		(g_headshotsOnly.integer) && 
		(targ->client->ps.stats[STAT_HEALTH] > 0) ) {
		return;
	}

	// L0 - Not used anymore..
/*
	if ( g_debugDamage.integer ) {
		G_Printf( "client:%i health:%i damage:%i armor:%i\n", targ->s.number,
			targ->health, take, asave );
	}
*/
	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		//client->damage_armor += asave; // L0 - Removed..
		client->damage_blood += take;
		client->damage_knockback += knockback;

		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
	Team_CheckHurtCarrier(targ, attacker);

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
		targ->client->lasthurt_time = level.time;	// L0 - chicken test
	}

	// L0 - Stats..
	if ( (attacker && attacker->client) && (targ && targ->client) ){
		if ( !OnSameTeam( attacker, targ ) && targ->client->ps.stats[STAT_HEALTH] > 0) {
			attacker->client->pers.dmgGiven += take;
			targ->client->pers.dmgReceived += take;
		// Count team damage but only if victim is alive..
		} else if (OnSameTeam( attacker, targ ) && targ->client->ps.stats[STAT_HEALTH] > 0) {
			attacker->client->pers.dmgTeam += take;
			write_RoundStats(attacker->client->pers.netname, attacker->client->pers.dmgTeam, ROUND_TEAMBLEED);
		}
	}
	// End

	// do the damage
	if (take) {
		targ->health = targ->health - take;

		// Ridah, can't gib with bullet weapons (except VENOM)
		if ( mod != MOD_VENOM && attacker == inflictor && targ->health <= GIB_HEALTH) {
			if (targ->aiCharacter != AICHAR_ZOMBIE)	// zombie needs to be able to gib so we can kill him (although he doesn't actually GIB, he just dies)
				targ->health= GIB_HEALTH+1;
		}

// JPW NERVE overcome previous chunk of code for making grenades work again
		if ((g_gametype.integer != GT_SINGLE_PLAYER) && (take > 190)) // 190 is greater than 2x mauser headshot, so headshots don't gib
				targ->health = GIB_HEALTH - 1;
// jpw
		//G_Printf("health at: %d\n", targ->health);
		if ( targ->health <= 0 ) {
			if ( client ) {
				targ->flags |= FL_NO_KNOCKBACK;
// JPW NERVE -- repeated shooting sends to limbo
					if (g_gametype.integer >= GT_WOLF)
						if ((targ->health < FORCE_LIMBO_HEALTH) && (targ->health > GIB_HEALTH) && (!(targ->client->ps.pm_flags & PMF_LIMBO)))
						{
							// L0 - Stats
							if (!OnSameTeam(attacker, targ) && attacker->client)
							{
								attacker->client->pers.gibs++;
								write_RoundStats(attacker->client->pers.netname, attacker->client->pers.gibs, ROUND_GIBS);
							}

							// L0 - Gib reports
							if (!attacker->client && g_gibReports.integer) 							
								AP(va("print \"%s ^7was gibbed.\n\"", targ->client->pers.netname));
							else if (!OnSameTeam(attacker, targ) && g_gibReports.integer)							 
								AP(va("print \"%s ^7was gibbed by %s^7.\n\"", targ->client->pers.netname, attacker->client->pers.netname));
							 else if (OnSameTeam(attacker, targ) && (attacker != targ) && g_gibReports.integer)
								 AP(va("print \"%s ^7was gibbed by ^3teammate ^7%s^7.\n\"", targ->client->pers.netname, attacker->client->pers.netname));							
							 else if (OnSameTeam(attacker, targ) && (attacker == targ) && g_gibReports.integer)
								 AP(va("print \"%s ^7has gibbed himself.\n\"", targ->client->pers.netname));							 
							// End

							limbo(targ, qtrue);
						}
// jpw
			}

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			if (targ->die)	// Ridah, mg42 doesn't have die func (FIXME)
				targ->die (targ, inflictor, attacker, take, mod);

			// if we freed ourselves in death function
			if (!targ->inuse)
				return;

			// RF, entity scripting
			if (targ->s.number >= MAX_CLIENTS && targ->health <= 0)	// might have revived itself in death function
				G_Script_ScriptEvent( targ, "death", "" );

		} else if ( targ->pain ) {
			if (dir) {	// Ridah, had to add this to fix NULL dir crash
				VectorCopy (dir, targ->rotate);
				VectorCopy (point, targ->pos3); // this will pass loc of hit
			} else {
				VectorClear( targ->rotate );
				VectorClear( targ->pos3 );
			}

			targ->pain (targ, attacker, take, point);

			// RF, entity scripting
			if (targ->s.number >= MAX_CLIENTS)
				G_Script_ScriptEvent( targ, "pain", va("%d %d", targ->health, targ->health+take) );
		}

		//G_ArmorDamage(targ);	//----(SA)	moved out to separate routine

		// Ridah, this needs to be done last, incase the health is altered in one of the event calls
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}
	}

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;



	if(&g_entities[tr.entityNum] == targ)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;


	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;
// JPW NERVE
	float		boxradius;
	vec3_t		dest; 
	trace_t		tr;
	vec3_t		midpoint;
// jpw


	if ( radius < 1 ) {
		radius = 1;
	}

	boxradius = 1.41421356 * radius; // radius * sqrt(2) for bounding box enlargement -- 
	// bounding box was checking against radius / sqrt(2) if collision is along box plane
	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - boxradius; // JPW NERVE
		maxs[i] = origin[i] + boxradius; // JPW NERVE
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

/* JPW NERVE -- we can put this back if we need to, but it kinna sucks for human-sized bboxes
		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}
*/
// JPW NERVE
		if (!ent->r.bmodel)
			VectorSubtract(ent->r.currentOrigin,origin,v); // JPW NERVE simpler centroid check that doesn't have box alignment weirdness
		else {
			for ( i = 0 ; i < 3 ; i++ ) {
				if ( origin[i] < ent->r.absmin[i] ) {
					v[i] = ent->r.absmin[i] - origin[i];
				} else if ( origin[i] > ent->r.absmax[i] ) {
					v[i] = origin[i] - ent->r.absmax[i];
				} else {
					v[i] = 0;
				}
			}
		}
// jpw
		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

// JPW NERVE -- different radiusdmg behavior for MP -- big explosions should do less damage (over less distance) through failed traces
		if( CanDamage (ent, origin) ) {
			if( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
// JPW NERVE --  MP weapons should do 1/8 damage through walls over 1/8th distance
		else {
			if (g_gametype.integer != GT_SINGLE_PLAYER) { 
				VectorAdd (ent->r.absmin, ent->r.absmax, midpoint);
				VectorScale (midpoint, 0.5, midpoint);
				VectorCopy (midpoint, dest);
			
				trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
				if (tr.fraction < 1.0) {
					VectorSubtract(dest,origin,dest);
					dist = VectorLength(dest);
					if (dist < radius*0.2f) { // closer than 1/4 dist
						if( LogAccuracyHit( ent, attacker ) ) {
							hitClient = qtrue;
						}
						VectorSubtract (ent->r.currentOrigin, origin, dir);
						dir[2] += 24;
						G_Damage (ent, NULL, attacker, dir, origin, (int)(points*0.1f), DAMAGE_RADIUS, mod);
					}
				}
			}
		}
// jpw
	}
	return hitClient;
}
