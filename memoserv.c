/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module:  MemoServ
** Version: 1.0
*/


/* TODO STUFF
** =======================================
** - Put in standard headers in all files!
** - Make patch-o-matic, so we can distribute without Neo
** =======================================
** - private should be able to set what u want to hide (after 1.0)
** - link nicks (after 1.0)
** - set language option
** - stress test it !!! 
** - redo recover... doesn't work the way it should
** - forbid a active name starts to crash neo
** - whois on identify with info string (settable)
** - info is crashing neostats... I think its a malloc problem... have to look into it
** - - it was lookup_regnick.. free'ing anything from that function crashes Neo, hu?
** - - anyway, I kinda fixed it, but I think its a kludge, and its not to good... 
** - check code to make sure any changes we send out to the net also get noticed here (by calling Usr_* in ircd.c in core code)
** - - - - Some of these shouldn't/arenot needed, as the IRCD sents the changes back to us - like SVSNICK.
** - - - - it would actually be easier if we use functions to send out, eh, Send_Svsnick(u, target) then we can make sure:
** - - - - 1) that the correct protocol is used (ie, can use Tokens or full cmds, based on what the uplink suppports)
** - - - - 2) make cross IRCD compatibility a lot easier
** - - - - but its better done in the core NeoStats
** - - (before 1.0)
**
** - Handle Squits (ie, if the user squits, then start a time to keep them ident'd in case the rejoin in a time period, then delete from regnicks)
**   Delete or add as u do stuff (after 1.0)
**
** - Plus all the TODO: in the code!!!!!
** - Any in the Readme for 0.2 release
*/




#include <stdio.h>
#include <db.h>
#include <dl.h>
#include <stats.h>
#include "services.h"
#include "language.h"

#define MS_Version "1.0 - 25/10/2001"


int MS_Bot_Message(char *origin, char **av, int ac)
{
	char *buf;
	User *u;
	u = finduser(origin);
	if (!u) {
		log("Unable to find user %s (nickserv)", origin);
		return -1;
	}
	log("%s", av[1]);
	if (!strcasecmp(av[1], "HELP")) {
		if (ac > 2) {
			send_mshelp(u, av[2]);
		} else {
			send_mshelp(u, NULL);
		}
		return 1;
	}
	return 1;
}
