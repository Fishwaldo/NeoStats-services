/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module:  NickServ
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
#include "dl.h"
#include "stats.h"
#include "services.h"
#include "language.h"

extern const char *ls_help[];
static void ns_register(User *u, char *email, char *pass);
static void ns_identify(User *u, char *pass);
static void ns_info(User *u, char *nick);
static void ns_ghost(User *, char *, char *);
static void ns_set(User *, char *);
static void ns_recover(User*, char*, char*);
static void ns_logout(User *);
static void ns_opercomment(User *, char*);
static void ns_drop(User *, char *);
static void ns_accesslist(User *, char *);
static void ns_suspend(User *, char *);
static void ns_forbid(User *, char *);
static void ns_getpass(User *, char *);
static void ns_setpass(User *, char *);
static void ns_setvhost(User *, char *);
void lslog(char *, ...);



#define NS_Version "1.0 - 25/10/2001"

int guest_num = 1000;


int __Bot_Message(char *origin, char *coreLine, int type)
{
	User *u;
	char *cmd, *tok1, *tok2;
	u = finduser(origin);
	if (!u) {
		log("Unable to find user %s (nickserv)", origin);
		return -1;
	}

	if (coreLine == NULL) return -1;
	cmd = strtok(coreLine, " ");

	if (!strcasecmp(cmd, "HELP")) {
		send_help(u, strtok(NULL,""));
		return 1;
	} else if (!strcasecmp(cmd, "REGISTER")) {
		tok1 = strtok(NULL, " ");
		if (tok1 == NULL) {
			privmsg(u->nick, s_NickServ, "Error, Incorrect Syntax, please type /msg nickserv help register");
			return -1;
		}
		if (strpbrk("@", tok1) == NULL) {
			privmsg(u->nick, s_NickServ, "Your Email address is invalid (No @)");
			return -1;
		}
		if (strpbrk(".", tok1) == NULL) {
			privmsg(u->nick, s_NickServ, "Your Email address is invalid (No .)");
			return -1;
		}
		tok2 = strtok(NULL, " ");
		if (tok2 == NULL) {
			privmsg(u->nick, s_NickServ, "Error, Incorrect Syntax, please type /msg nickserv help register");
			return -1;
		}
		ns_register(u, tok1, tok2);
		return 1;
	} else if (!strcasecmp(cmd, "IDENTIFY")) {
		tok1 = strtok(NULL, " ");
		if (tok1 == NULL) {
			privmsg(u->nick, s_NickServ, "Error, You must Supply a Password");
			return -1;
		}
		ns_identify(u, tok1);	
		return 1;
	} else if (!strcasecmp(cmd, "INFO")) {
		tok1 = strtok(NULL, " ");
		if (tok1 == NULL) {
			privmsg(u->nick, s_NickServ, "Error, you Must supply a Nickname");
			return -1;
		}
		ns_info(u, tok1);
		return 1;
	} else if (!strcasecmp(cmd, "GHOST")) {
		tok1 = strtok(NULL, " ");
		if (tok1 == NULL) {
			privmsg(u->nick, s_NickServ, "Error, You must supply a nickname");
			return -1;
		}
		tok2 = strtok(NULL, " ");
		if (tok2 == NULL) {
			privmsg(u->nick, s_NickServ, "Error, You must supply a Password");
			return -1;
		}
		ns_ghost(u, tok1, tok2);
		return 1;
	} else if (!strcasecmp(cmd, "RECOVER")) {
		tok1 = strtok(NULL, " ");
		if (tok1 == NULL) {
			privmsg(u->nick, s_NickServ, "Error, You must supply a nickname");
			return -1;
		}
		tok2 = strtok(NULL, " ");
		if (tok2 == NULL) {
			privmsg(u->nick, s_NickServ, "Error, you must supply your password");
			return -1;
		}
		ns_recover(u, tok1, tok2);
		return 1;
	} else if (!strcasecmp(cmd, "DROP")) {
		ns_drop(u, strtok(NULL, " "));
		return 1;
	} else if (!strcasecmp(cmd, "LOGOUT")) {
		ns_logout(u);
		return 1;
	} else if (!strcasecmp(cmd, "\001VERSION\001")) {
		sts(":%s NOTICE %s :\001VERSION NickServ Version %s", s_NickServ, u->nick, NS_Version);
		return 1;
	} else if (!strcasecmp(cmd, "SET")) {
		ns_set(u, strtok(NULL, ""));
		return 1;
	} else if (!strcasecmp(cmd, "OPERCOMMENT")) {
		/* if the user doesn't have the access rights, just return the normal what? message! */
		if (UserLevel(u)  < OPERCOMMENT_SET_ACCESS_LEVEL) {
			goto unknown;
		}
		ns_opercomment(u, strtok(NULL, ""));
		return 1;
	} else if (!strcasecmp(cmd, "SUSPEND")) {
		/* if the user doesn't have access, then return the normal unknown message */
		if (UserLevel(u) < OPERSUSPEND_SET_ACCESS_LEVEL) {
			goto unknown;
		}
		ns_suspend(u, strtok(NULL, ""));
		return 1;
	} else if (!strcasecmp(cmd, "FORBID")) {
		/* if the user doesn't have the access, return the normal unknown list */
		if (UserLevel(u) < OPERFORBID_SET_ACCESS_LEVEL) {
			goto unknown;
		}
		ns_forbid(u, strtok(NULL, ""));
		return 1;
	} else if (!strcasecmp(cmd, "GETPASS")) {
		/* check its enabled */
		if (!ENABLE_GETPASS) {
			goto unknown;
		}
		if (UserLevel(u) < GETPASS_ACCESS_LEVEL) {
			goto unknown;
		}
		ns_getpass(u, strtok(NULL, " "));
		return 1;
	} else if (!strcasecmp(cmd, "SETPASS")) {
		/* check its enabled */
		if (!ENABLE_SETPASS) {
			goto unknown;
		}
		if (UserLevel(u) < SETPASS_ACCESS_LEVEL) {
			goto unknown;
		}
		ns_setpass(u, strtok(NULL, ""));
		return 1;	
	} else if (!strcasecmp(cmd, "VHOST")) {
		if (!((ENABLE_VHOST >= 1) && (UserLevel(u) >= SETVHOST_ACCESS_LEVEL))) {
			privmsg(u->nick, s_NickServ, "Access Denied");
			return 1;
		}
		ns_setvhost(u, strtok(NULL, ""));
		return 1;
	} else if (!strcasecmp(cmd, "ACCESS")) {
		ns_accesslist(u, strtok(NULL, ""));
		return 1;
	}


	unknown:
	privmsg(u->nick, s_NickServ, "Unknown Command: \2%s\2, maybe you need help?", cmd);
	return -1;


}
static void ns_setvhost(User *u, char *line) {
	NS_User *ns;
	char *nick, *vhost;
	int save = 0;
	
	nick = strtok(line, " ");
	vhost = strtok(NULL, " ");
	if ((!nick) || (!vhost)) {
		privmsg(u->nick, s_NickServ, "Syntax Error: \002/%s help vhost\002 for help", s_NickServ);
		return;
	}
	ns = findregnick(nick);
	if (!ns) {
		ns = lookup_regnick(nick);
		save = 1;
	}
	if (!ns) {
		privmsg(u->nick, s_NickServ, "Error, that Nickname does not exist");
		return;
	}
	if (!strcasecmp(vhost, "NONE")) {
		strcpy(ns->vhost, "");
		if (save == 0) {
			ns->lastchange = time(NULL);
		} else {
			sync_nick_to_db(ns);
		}
		privmsg(u->nick, s_NickServ, "Vhost has been removed");
		notice(s_NickServ, "\002VHOST\002 was revoved from %s by %s", ns->nick, u->nick);
		return;
	}
	strcpy(ns->vhost, vhost);
	privmsg(u->nick, s_NickServ, "Vhost for %s now set to \002%s\002", nick, vhost);
	notice(s_NickServ, "\002VHOST\002 was set for %s by %s: %s", ns->nick, u->nick, vhost); 
	if (save == 0) ns->lastchange = time(NULL);
	if (save == 1) sync_nick_to_db(ns);
}

static void ns_setpass(User *u, char *line) {
	NS_User *ns;
	char *nick, *pass;
	int save = 0;
	
	nick = strtok(line, " ");
	pass = strtok(NULL, " ");
	if ((!nick) || (!pass)) {
		privmsg(u->nick, s_NickServ, "Syntax Error: \002/%s help setpass\002 for help", s_NickServ);
		return;
	}
	ns = findregnick(nick);
	if (!ns) {
		ns = lookup_regnick(nick);
		save = 1;
	}
	if (!ns) {
		privmsg(u->nick, s_NickServ, "Error, that Nickname does not exist");
		return;
	}
	strcpy(ns->pass, pass);
	if (save == 0) ns->lastchange = time(NULL);
	privmsg(u->nick, s_NickServ, "Password for %s has been set to \002%s\002", nick, pass);
	notice(s_NickServ, "\002SETPASS\002 was used by %s on %s", u->nick, nick);
	globops(s_NickServ, "\002SETPASS\002 was used by %s on %s", u->nick, nick);
	if (save == 0) privmsg(nick, s_NickServ, "Your Password has been reset to \002%s\002 by %s", pass, u->nick);
	if (save == 1) sync_nick_to_db(ns);
	return; 	
}
static void ns_getpass(User *u, char *line) {
	NS_User *ns;
	
	if (!line) {
		privmsg(u->nick, s_NickServ, "Syntax Error: \002/%s help getpass\002 for help", s_NickServ);
		return;
	}
	log("%s", line);
	ns = findregnick(line);
	if (!ns) 
		ns = lookup_regnick(line);
	if (!ns) {
		privmsg(u->nick, s_NickServ, "Error: That nickname does not exist");
		return;
	}
	privmsg(u->nick, s_NickServ, "Password for %s is \002%s\002", line, ns->pass);
	notice(s_NickServ, "\002GetPass\002 was used by %s on %s", u->nick, line);
	globops(s_NickServ, "\002GetPass\002 was used by %s on %s", u->nick, line);
	return;
}



static void ns_forbid(User *u, char *line) {
	NS_User *ns;
	int count = 0;
	char *cmd, *tmp;

	log("%s", line);
	cmd = strtok(line, " ");
	if (!cmd) {
		privmsg(u->nick, s_NickServ, "Syntax Error, \002/%s help forbid\002 for more information", s_NickServ);
		return;
	}
	if (!strcasecmp(cmd, "LIST")) {
		if (strlen(ns_forbid_list) == 0) {
			privmsg(u->nick, s_NickServ, "Forbid List is empty");
			return;
		}
		strcpy(cmd, ns_forbid_list);
		tmp  = strtok(cmd, " ");
		privmsg(u->nick, s_NickServ, "Forbidden Nick List:");
		while (tmp) {
			count++;
			privmsg(u->nick, s_NickServ, "\002%d\002 - %s", count, tmp);
			tmp = strtok(NULL, " ");
		}			
		privmsg(u->nick, s_NickServ, "End of List.");
	} else if (!strcasecmp(cmd, "ADD")) {
		tmp = strtok(NULL, " ");
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Syntax Error, \002/%s help forbid\002 for more information", s_NickServ);
			return;
		}
		sprintf(ns_forbid_list, "%s %s", ns_forbid_list, tmp);
		privmsg(u->nick, s_NickServ, "Success: Added the Following Nick to the Forbidden List: %s", tmp);
		ns = findregnick(tmp);
		if (ns) {
			privmsg(u->nick, s_NickServ, "A Registered user with the nick is online now, Informing them of the Forbidden Nickname, and dropping their registration", tmp);
			privmsg(tmp, s_NickServ, "Warning, Your nickname has been Forbidden for Registration with Services, it has been dropped");
			ns_drop(u, tmp);			
		}
		ns = lookup_regnick(tmp);
		if (ns) {
			privmsg(u->nick, s_NickServ, "That Nickname has already been registered, Dropping it from the database");
			ns_drop(u, tmp);
		}
		save_nick_forbid_list();
		return;
	} else if (!strcasecmp(cmd, "DEL")) {
		char *newlist, *tmp2;
		int ok = 0;

		cmd = strtok(NULL, " ");
		if ((!cmd) || (!strspn(cmd, "0123456789"))) {
			privmsg(u->nick, s_NickServ, "Syntax Error: \002/%s help forbid del\002 for help", s_NickServ);
			return;
		}			
		newlist = smalloc(1024);
		tmp2 = smalloc(1024);
		strcpy(tmp2, ns_forbid_list);
		tmp = strtok(tmp2, " ");
		while (tmp) {
			count++;
			if (count != atoi(cmd)) {
				sprintf(newlist, "%s %s", newlist, tmp);
			} else if (count == atoi(cmd)) {
				privmsg(u->nick, s_NickServ, "Removed %s from the forbidden list", tmp);
				ok = 1;
			}						
			tmp = strtok(NULL, " ");
		}
		if (ok == 0) {
			privmsg(u->nick, s_NickServ, "Error, Entry Number %s does not exist", cmd);
		}
		strcpy(ns_forbid_list, newlist);
		free(tmp2);
		free(newlist);
		save_nick_forbid_list();
		return;
	}




}
static void ns_suspend(User *u, char *line) {
	NS_User *ns;
	char *nick;
	int dosave = 0;

	nick = strtok(line, " ");
	ns=findregnick(nick);
	if (!ns) {
		ns = lookup_regnick(nick);
		dosave = 1;
	}
	if (!ns) {
		privmsg(u->nick, s_NickServ, "Error, the nickname %s is not registered", nick);
		if (dosave == 1) free(ns);
		return;
	}
	nick = strtok(NULL, "");
	if (!nick) {
		privmsg(u->nick, s_NickServ, "Error, you must supply a Comment for the suspension");
		return;
	}
	if (!strcasecmp(nick, "OFF")) {
		if (!(ns->flags &= NSFL_SUSPEND)) {
			privmsg(u->nick, s_NickServ, "Error, \002%s\002 is not suspended", ns->nick);
			return;
		}
		ns->flags &= ~NSFL_SUSPEND;
		privmsg(u->nick, s_NickServ, "Success, \002%s\002 has been un-suspended", ns->nick);
		strcpy(ns->opercomment, "");
		ns->lastchange = time(NULL);
		if (dosave == 1) sync_nick_to_db(ns);
		return;
	}
	ns->flags |= NSFL_SUSPEND;
	log("%d", ns->flags);
	sprintf(ns->opercomment, "(%s) - %s", u->nick, nick);
	ns->lastchange = time(NULL);	
	privmsg(u->nick, s_NickServ, "Success, \002%s\002 has been suspended", ns->nick);
	if (dosave == 1) sync_nick_to_db(ns);
/*	if (dosave == 1) free(ns); */

}
static void ns_accesslist(User *u, char *line) {
	NS_User *ns;
	char *cmd, *target;
	int count = 0;
	
	ns = findregnick(u->nick);
	if (!ns) {
		privmsg(u->nick, s_NickServ, "Error, your Nickname is not registered");
		return;
	}
	if ((ns->secure == 1) && !(ns->onlineflags &= NSFL_IDENTIFED)) {
		privmsg(u->nick, s_NickServ, "Access Denied");
		return;
	}
	if (ns->flags &= NSFL_SUSPEND) {
		privmsg(u->nick, s_NickServ, "Error, Your Nickname has been Suspended");
		return;
	}
	cmd = strtok(line, " ");
	if (!cmd) {
		privmsg(u->nick, s_NickServ, "Syntax Error: \002/%s help access\002 for help", s_NickServ);
		return;
	}
	if (!strcasecmp(cmd, "LIST")) {
		strcpy(cmd, ns->acl);
		target = strtok(cmd, " ");
		privmsg(u->nick, s_NickServ, "Your Access list is:");
		while (target) {
			count++;
			privmsg(u->nick, s_NickServ, "\002%d\002 %s", count, target);
			target = strtok(NULL, " ");
		}
		privmsg(u->nick, s_NickServ, "End of Access List");
		return;
	} else if (!strcasecmp(cmd, "ADD")) {
/* TODO: have to check the format (*@*) to make sure its valid */
/* TODO: have to make sure that no other acl is the same as this */
		target = strtok(NULL, " ");
		if (!target) {
			privmsg(u->nick, s_NickServ, "Syntax Error: \002/%s help access add\002 for help", s_NickServ);
			return;
		}
		sprintf(ns->acl, "%s %s", ns->acl, target);
		privmsg(u->nick, s_NickServ, "Success, Added %s to your access list", target);
		ns->lastchange = time(NULL);
		return;		
	} else if (!strcasecmp(cmd, "CURRENT")) {
		/* first check there isn't a match already */
		if (ns->acl != NULL) {
			cmd = strtok(ns->acl, " ");
			target = makemask(u, 0);
			if (!match2(cmd, target)) {
#ifdef DEBUG						
				log("access list match  for usrmask %s", target);
#endif
				count = 1;
				goto gotcurrent;

			} else {
				while ((cmd = strtok(NULL, " ")) != NULL) {
					if (!match2(cmd, target)) {
#ifdef DEBUG						
						log("access list match  for usrmask %s", target);
#endif
						count = 1;
						goto gotcurrent;
					}
				}				
			}
		}
		gotcurrent:
		if (count == 1) {
			privmsg(u->nick, s_NickServ, "Error: One of your Access Lists (%s) already Matches your Host. Not adding another", cmd);
			return;
		}
		target = makemask(u, 0);
		sprintf(ns->acl, "%s %s", ns->acl, target);
		privmsg(u->nick, s_NickServ, "Success, Added your Current Host (%s) to the Access list", target);
		ns->lastchange = time(NULL);
		return;
	} else if (!strcasecmp(cmd, "DEL")) { 
		char *newacl, *tmp;
		int ok = 0;

		cmd = strtok(NULL, " ");
		if ((!cmd) || (!strspn(cmd, "0123456789"))) {
			privmsg(u->nick, s_NickServ, "Syntax Error: \002/%s help access del\002 for help", s_NickServ);
			return;
		}			
		newacl = smalloc(1024);
		tmp = smalloc(1024);
		strcpy(tmp, ns->acl);
		target = strtok(tmp, " ");
		while (target) {
			count++;
			if (count != atoi(cmd)) {
				sprintf(newacl, "%s %s", newacl, target);
			} else if (count == atoi(cmd)) {
				privmsg(u->nick, s_NickServ, "Removed %s from your access list", target);
				ok = 1;
			}						
			target = strtok(NULL, " ");
		}
		if (ok == 0) {
			privmsg(u->nick, s_NickServ, "Error, Entry Number %s does not exist", cmd);
		}
		strcpy(ns->acl, newacl);
		ns->lastchange = time(NULL);
		free(tmp);
		free(newacl);
		return;
	} else {
		privmsg(u->nick, s_NickServ, "Syntax Error: \002/%s help access\002 for help", s_NickServ);
		return;
	}
			 
}



static void ns_drop(User *u, char *targetnick) {
	NS_User *ns;
	char *tmp;
	
	if (targetnick) { 
		if (UserLevel(u) >= CAN_DROP_NICK_LEVEL) {
			tmp = targetnick;
		} else {
			privmsg(u->nick, s_NickServ, "Syntax Error. \002/%s help drop\002 for more information", s_NickServ);
			return;
		}
	} else {
		tmp = u->nick;
	}

	ns=findregnick(tmp);
	if (!ns) {
		if (targetnick) {
			ns = lookup_regnick(targetnick);
		}
	}
	if (!ns) {
		privmsg(u->nick, s_NickServ, "Error, the nickname %s is not registered", tmp);
		return;
	}
	if (!targetnick) {
		if ((ns->secure == 1) && !(ns->onlineflags &= NSFL_IDENTIFED)) {
			privmsg(u->nick, s_NickServ, "Access Denied");
			return;
		}
		if (ns->flags &= NSFL_SUSPEND) {
			privmsg(u->nick, s_NickServ, "Error, Your Nickname has been Suspended");
			return;
		}

	}

	del_nick_from_db(tmp);
	/* only delete the nickname from the list if they are not online */
	if (finduser(tmp)) del_regnick(tmp);
	privmsg(u->nick, s_NickServ, "The Nickname %s was dropped", tmp);
	notice(s_NickServ, "%s(%s@%s) Dropped nickname %s", u->nick, u->username, u->vhost, tmp);

}

static void ns_opercomment(User *u, char *comment) {
	NS_User *ns;
	int dosave = 0;
	char *tmp;
	
	tmp = strtok(comment, " ");
	ns = findregnick(tmp);
	if (!ns) {
		/* they are not online at the moment */
		ns = lookup_regnick(tmp);
		dosave = 1;
	}
	if (!ns) {
		/* the nick doesn't exist */
		privmsg(u->nick, s_NickServ, "Error, that Nickname doesn't Exist");
		return;
	}
	if (!comment) {
		privmsg(u->nick, s_NickServ, "Error, you Must set a comment");
		return;
	}
	if (strlen(comment) > 255) {
		privmsg(u->nick, s_NickServ, "Error, you Comment can only be a Maxiumn of 255 Characters");
		return;
	}
	tmp = strtok(NULL, "");
	sprintf(ns->opercomment, "(%s) - %s", u->nick, tmp);
	if (dosave) sync_nick_to_db(ns);
/*	if (dosave) free(ns); */
	privmsg(u->nick, s_NickServ, "You OperComment has been saved");
	
}


static void ns_logout(User *u) {
	NS_User *ns;
	
	ns = findregnick(u->nick);
	if (!ns) {
		privmsg(u->nick, s_NickServ, "Error: Your nick is not registered");
		return;
	}
	if ((ns->secure == 1) && !(ns->onlineflags &= NSFL_IDENTIFED)) {
		privmsg(u->nick, s_NickServ, "Access Denied");
		return;
	}
	if (ns->flags &= NSFL_SUSPEND) {
		privmsg(u->nick, s_NickServ, "Error, Your Nickname has been Suspended");
		return;
	}

	/* check if they are secure, and if so, remove all online flags or just identified flag */
	if (ns->secure == 1) {
		ns->onlineflags = 0;
	} else {
		ns->onlineflags &= ~NSFL_IDENTIFED;
	}
	privmsg(u->nick, s_NickServ, "You have been Logged out of Services");
	if (ns->secure > 0) privmsg(u->nick, s_NickServ, "You will have to identify again to gain access to Services");	
}
static void ns_set(User *u, char *args) {
	char *cmd, *tmp;
	NS_User *ns;
	
	cmd = strtok(args, " ");
	ns = findregnick(u->nick);
	
	/* check its a registered nick, and that its been identified for if secure is set */
	/* notice I can't spell? */
	
	if ((!ns) || (!(ns->onlineflags &= NSFL_IDENTIFED) && (ns->secure == 1))) {
		privmsg(u->nick, s_NickServ, "Access Denied");
		return;
	}

	if (ns->flags &= NSFL_SUSPEND) {
		privmsg(u->nick, s_NickServ, "Error, Your Nickname has been Suspended");
		return;
	}
	if (cmd == NULL) {
		goto nosetmsg;
	} else if (!strcasecmp(cmd, "HELP")) {
		privmsg(u->nick, s_NickServ, "TODO");
		return;
	} else if (!strcasecmp(cmd, "PASSWORD")) {
		tmp = strtok(NULL, " ");
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Error, You Must Supply a New Password");
			return;
		}
		strcpy(ns->pass, tmp);
		ns->lastchange = time(NULL);
		log("New Pass: %s", ns->pass);
		privmsg(u->nick, s_NickServ, "Password Updated to \002%s\002", ns->pass);
		return;
		
	} else if (!strcasecmp(cmd, "URL")) {
		tmp = strtok(NULL, " ");
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Incorrect Syntax, \002/%s help set url \002 for more information", s_NickServ);
			return;
		} else if (!strcasecmp(tmp, "NONE")) {
			strcpy(ns->url, "");
			privmsg(u->nick, s_NickServ, "URL Deleted");
			ns->lastchange = time(NULL);
			return;
		} else {
			strcpy(ns->url, tmp);
			privmsg(u->nick, s_NickServ, "URL Set to: \002%s\002", ns->url);
			ns->lastchange = time(NULL);
			return;
		}
		
	} else if (!strcasecmp(cmd, "ICQ")) {
		tmp = strtok(NULL, " ");
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Incorrect Syntax, \002/%s help set icq\002 for more information", s_NickServ);
			return;
		} else if (!strcasecmp(tmp, "NONE")) {
			ns->icq = 0;
			privmsg(u->nick, s_NickServ, "ICQ Number Deleted");
			ns->lastchange = time(NULL);
			return;
		} else {
			if (!strspn(tmp, "0123456789")) {
				privmsg(u->nick, s_NickServ, "Error, ICQ Field can only be a number");
				return;
			}

			ns->icq = atoi(tmp);
			privmsg(u->nick, s_NickServ, "ICQ Set to: \002%ul\002", ns->icq);
			ns->lastchange = time(NULL);
			return;
		}
	} else if (!strcasecmp(cmd, "INFO")) {
		tmp = strtok(NULL, "");
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Incorrect Syntax, \002/%s help set info\002 for more information", s_NickServ);
			return;
		}
		if (strlen(tmp) > NICK_INFO_LEN) {
			privmsg(u->nick, s_NickServ, "Error, info field can only be %d Characters long", NICK_INFO_LEN);
			return;
		} else {
			strcpy(ns->info, tmp);
			privmsg(u->nick, s_NickServ, "Info Field Set to: \002%s\002", tmp);
			ns->lastchange = time(NULL);
			return;
		}
	} else if (!strcasecmp(cmd, "AIM")) {
		tmp = strtok(NULL, " ");
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Incorrect Syntax, \002/%s help set AIM\002 for more information", s_NickServ);
			return;
		} else if (!strcasecmp(tmp, "NONE")) {
			strcpy(ns->aim, "");
			privmsg(u->nick, s_NickServ, "AIM Handle Deleted");
			ns->lastchange = time(NULL);
			return;
		} else {
			strcpy(ns->aim, tmp);
			privmsg(u->nick, s_NickServ, "AIM Handle Set to: \002%s\002", ns->aim);
			ns->lastchange = time(NULL);
			return;
		}
	} else if (!strcasecmp(cmd, "GREET")) {
		tmp = strtok(NULL, " ");
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Incorrect Syntax, \002/%s help set greet\002 for more information", s_NickServ);
			return;
		} else if (!strcasecmp(tmp, "NONE")) {
			strcpy(ns->greet, "");
			privmsg(u->nick, s_NickServ, "Channel Greeting Disabled");
			ns->lastchange = time(NULL);
			return;
		} else if (strlen(tmp) > 100) {
			privmsg(u->nick, s_NickServ, "Error, your Channel Greeting must be smaller than 100 Characters");
			return;
		} else {
			strcpy(ns->greet, tmp);
			privmsg(u->nick, s_NickServ, "Channel Greeting Set to: \002%s\002", ns->greet);
			ns->lastchange = time(NULL);
			return;
		}
	} else if (!strcasecmp(cmd, "EMAIL")) {
		tmp = strtok(NULL, " ");
/* TODO: check its a valid email address */
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Incorrect Syntax, \002/%s help set email\002 for more information", s_NickServ);
			return;
		} else {
			strcpy(ns->email, tmp);
			privmsg(u->nick, s_NickServ, "Email Set to: \002%s\002", ns->email);
			ns->lastchange = time(NULL);
			return;
		}
	} else if (!strcasecmp(cmd, "KILL")) {
		if (ns->kill >= 1) { 
			ns->kill = 0; 
		} else {
			ns->kill = 1; 
		}
		ns->lastchange = time(NULL);
		privmsg(u->nick, s_NickServ, "Kill Option is now \002%s\002", ns->kill ? "ON" : "OFF");
		return;
	} else if (!strcasecmp(cmd, "SECURE")) {
		if (ns->secure >= 1) {
			ns->secure = 0;
		} else {
			ns->secure = 1;
		}
		ns->lastchange = time(NULL);
		privmsg(u->nick, s_NickServ, "Secure Option is now \002%s\002", ns->secure ? "ON" : "OFF");
		return;
	} else if (!strcasecmp(cmd, "PRIVATE")) {
		if (ns->private >= 1) {
			ns->private = 0;
		} else {
			ns->private = 1;
		}
		ns->lastchange = time(NULL);
		privmsg(u->nick, s_NickServ, "Private Option is now \002%s\002", ns->private ? "ON" : "OFF");
		return;

	} else if (!strcasecmp(cmd, "VHOST")) {
		if (!ENABLE_VHOST) 
			goto nosetmsg;
		if (ENABLE_VHOST == 2) {
			privmsg(u->nick, s_NickServ, "Access Denied");
			return;
		}	
		tmp = strtok(NULL, " ");
		if (!tmp) {
			privmsg(u->nick, s_NickServ, "Syntax Error, \002/%s help set vhost\002 for help", s_NickServ);
			return;
		} else if (!strcasecmp(tmp, "NONE")) {
			privmsg(u->nick, s_NickServ, "Vhost has been Disabled");		
			strcpy(ns->vhost, "");
			ns->lastchange = time(NULL);
			return;
		} else {
			strcpy(ns->vhost, tmp);
			ns->lastchange = time(NULL);
			privmsg(u->nick, s_NickServ, "VHOST has been set to %s. you will get this Vhost next time you identify", tmp);
			return;
		} 			
	} else {
		privmsg(u->nick, s_NickServ, "Unknown Command, Please type \002/%s help set\002", s_NickServ);
		return;
	}

	nosetmsg:
	privmsg(u->nick, s_NickServ, "Please type \002/%s help set\002 to see available options", s_NickServ);
	privmsg(u->nick, s_NickServ, "Your Current Options are:");
	ns_info(u, u->nick);
	return;

}

int ns_bye_user(User *u) {
	NS_User *tmp;
	char *buffer, *quitmsg;
	/* this should be easy... Just sync the database for that user, and then del them from regnick */

	tmp = findregnick(u->nick);
	if (tmp) {
		/* recbuf from the core contains the line that trigged this, in this 
		** case, we need to copy it and extract the quit message
		*/
		buffer = smalloc(255);
		strcpy(buffer, recbuf);
		quitmsg = strtok(buffer, ":");
		quitmsg = strtok(NULL, "");
		strcpy(tmp->last_quit, quitmsg);
		tmp->last_seen_at = time(NULL);
		free(buffer);
		sync_nick_to_db(tmp);
		del_regnick(u->nick);		

	}
	return 1;
}

int ns_nickchange_user(char *line) {
	char *orignick, *targetnick;
	
	orignick = strtok(line, " ");
	targetnick = strtok(NULL, " ");

	if (findregnick(orignick)) {
		sync_nick_to_db(findregnick(orignick));
		del_regnick(orignick);
	}
	
	/* this function is called after NeoStats has updated the Internal Tables, so finduser will work */

	ns_new_user(finduser(targetnick));
	sts(":%s SVSMODE %s :-r", me.name, targetnick);
	UserMode(targetnick, "-r");
	return 1;
}
int ns_new_user( User *u) {
	char *usrmask, *tmp;

	NS_User *temp_user;
	char *temp;
	Svs_Timers *svc_timers;
	temp_user = new_regnick(u->nick, 0);
	if (temp_user != NULL) {

	log("%d", temp_user->flags);
	if (temp_user->flags &= NSFL_SUSPEND) {
		privmsg(u->nick, s_NickServ, "Warning, Your Nickname has been Suspended");
		privmsg(u->nick, s_NickServ, "You will be unable to preform any Services Functions");
		return -1;
	}
	
	if (temp_user->kill == 0) return 1;
	
		/* The nick is a registered nick, Check kill, secure and access lists now */

		if (!temp_user->secure == 0) { 

			/* Check the Access list */

			usrmask = malloc(sizeof(u->username) + sizeof(u->hostname) +1);
			snprintf(usrmask, 200, "%s@%s", u->username, u->hostname);
			
			/* This will add the user to the "registered_users" list, but set their flag to be accesslist only */


			if (temp_user->acl != NULL) {
				tmp = strtok(temp_user->acl, " ");
				if (!match2(tmp, usrmask)) {
#ifdef DEBUG						
					log("access list match  for usrmask %s", usrmask);
#endif
					goto accessmatch;
				} else {
					while ((tmp = strtok(NULL, " ")) != NULL) {
						if (!match2(tmp, usrmask)) {
#ifdef DEBUG						
							log("access list match  for usrmask %s", usrmask);
#endif
						goto accessmatch;
						}
					}				
				}
				goto noaccessmatch;	
				accessmatch:
					privmsg(u->nick, s_NickServ, "This Nickname is Registered and Protected");
					privmsg(u->nick, s_NickServ, "You have been matched against your accesslist");
					privmsg(u->nick, s_NickServ, "but you may still have to identify to access certian functions");	
					if (strlen(temp_user->vhost) > 0) sts(":%s CHGHOST %s %s", s_NickServ, u->nick, temp_user->vhost);
					temp_user->onlineflags = NSFL_ACCESSMATCH;
#ifdef DEBUG
					log("flags: %d", temp_user->onlineflags);
#endif
                                	temp = malloc(sizeof(u->username) + sizeof(u->vhost) + 2);
                                        strcpy(temp, u->username);
                                        strcat(temp, "@");
                                        strcat(temp, u->vhost);
                                    	strcpy(temp_user->last_mask, temp);
					temp_user->last_seen_at = time(NULL);
 					free(temp);
					/* last change is for the database sync stuff */

					temp_user->lastchange = time(NULL);
					return 1;
			}
				
		}
		noaccessmatch:
		privmsg(u->nick, s_NickServ, "This Nickname is Registered and protected");
		privmsg(u->nick, s_NickServ, "Please Identify with your Password within 3 Minutes");

			/* kill works like this: if kill == 0, then kill is disabled 
			** but if kill >= 1 then kill is enabled, and the value of kill = no of bad passes -1
			** we must also reset kill now as well, make sure its 1, as this is the first time we have
			** seen this user - Fish
			*/

		if (temp_user->kill > 1) temp_user->kill = 1;
		privmsg(u->nick, s_NickServ, "Otherwise your nick will be changed");
		sts(":%s SVSMODE %s :-r", me.name, u->nick);
		UserMode(u->nick, "-r");

		/* create a new Svc's Timer to change the nick if they do not identify 
		** We Don't delete the timer if they do change, but when ns_ident_timeout gets called
		** it checks to see if the nick is still in use
		** and if it is, returns 1, which forces the garbage collection to clean it up
		*/
		
		tmp = smalloc(37);
		snprintf(tmp, 36, "IdentTO-%s", u->nick);
		svc_timers = new_svs_timers(tmp);
		svc_timers->interval = NICK_IDENT_TIME;
		svc_timers->lastrun = time(NULL);
		strcpy(svc_timers->varibles, u->nick);
		svc_timers->function = dlsym((int *)get_dl_handle("Services"), "ns_ident_timeout");
		if (!svc_timers->function) {
			log("Error: %s", dlerror());
		}
		free(tmp);
		return 1;
	}
	return -1;
}

int ns_ident_timeout(char *nick) {
	User *u;
	char *tmpnick;
	Svs_Timers *svc_timers, *svc_timers2;
	NS_User *tmp_user;
	
	u = finduser(nick);
	/* see if the nick is still there, otherwise return straight away */
	if (!u) return 1;

	/* check to see if they identified, if so, then return straight away */
	tmp_user = findregnick(nick);
	
	if (tmp_user->onlineflags == NSFL_IDENTIFED) return 1;
	
	/* create a guest user nick, and make sure its not used */
	tmpnick = smalloc(32);
	snprintf(tmpnick, 32, "Guest%i", guest_num++);
	while (finduser(tmpnick))
		snprintf(tmpnick, 32, "Guest%i", guest_num++);

	privmsg(u->nick, s_NickServ, "Your Nickname has been changed to \002%s\002 as you did not identify", tmpnick);
	sts(":%s SVSNICK %s %s :Identify Timeout for Protected Nick", s_NickServ, u->nick, tmpnick);
	free(tmpnick);	

/* now we need to signon a new user to take this nick, so that if the clients script 
** tries to regain the nick, it wont work.
** one problem with NeoStats at the moment, is that if someone messages's this 
** enforced nick, NeoStats is going to Generate a Error message
** so thats a TODO for NeoStats, find some way to fix that
** currently Modules can only have 1 Bot... will fix that when I start writting CommServ (as it will be two Bots in the same 
** Module, and its needed to fix)
*/
	
/* TODO: this is hardcoded stuff, change it when the config interface is available */

	/* add a timer to delete this nick again, hard coded*/

	tmpnick = smalloc(37);
	snprintf(tmpnick, 36, "QuitEnforcer-%s", u->nick);
	svc_timers = new_svs_timers(tmpnick);
	svc_timers->interval = ENFORCER_HOLD_TIME;
	svc_timers->lastrun = time(NULL);
	strcpy(svc_timers->varibles, u->nick);
	svc_timers->function = dlsym((int *)get_dl_handle("Services"), "ns_quit_enforced_nick");
	if (!svc_timers->function) {
		log("Error: %s", dlerror());
	}
	free(tmpnick);
	tmpnick = smalloc(254);
	snprintf(tmpnick, 254, "SignonEnforcer-%s", u->nick);
	svc_timers2 = new_svs_timers(tmpnick);
	
	/* run it straight away */
	/* Straight away is NOT 0, as this would run it straight after this function, which deletes the purpose of a delay */
	
	svc_timers2->interval = 2;
	svc_timers2->lastrun = time(NULL);
	snprintf(svc_timers2->varibles, 254, "NICK %s 1 %d enforcer %s %s 0 :Nickname Enforcer", u->nick, (int)time(NULL), me.name, me.name);
	svc_timers2->function = dlsym((int *)get_dl_handle("Services"), "ns_signon_enforced_nick");
	if (!svc_timers2->function) {
		log("Error: %s", dlerror());
	}
	free(tmpnick);
	return 1;	
}

int ns_signon_enforced_nick(char *vars) {
	NS_User *temp_user;

	char *nick, *user;

	sts("%s", vars);
	nick = strtok(vars, " ");
	nick = strtok(NULL, " ");
	user = strtok(NULL, " ");
	user = strtok(NULL, " ");
	user = strtok(NULL, " ");
	
	/* this is bad... NeoStats wont know how to handle this user... See the comments in ns_ident_timeout */
	
	AddUser(nick, user, me.name, me.name);
	
	/* have to add this user to the regnicks list, so recover can work */
	
	temp_user = new_regnick(nick, 1);
	temp_user->onlineflags = NSFL_ENFORCED;
	return 1;
}
int ns_quit_enforced_nick(char *nick) {
	NS_User *temp_user;
	
	/* check if the enforced nick has been recovered */
	temp_user = findregnick(nick);
	if (temp_user->onlineflags != NSFL_ENFORCED) {
		return 1;
	}
	
	sts(":%s QUIT :Enforcer Timeout", nick);

	/* TODO: its this user is added to any list, then remove it */

	DelUser(nick);

	/* remove it from the Registered Nicks List 
	** we don't have to check if its in the list, as its a Enforcer Nick
	** and only registered nicks can become enforced
	** and if it is enforced, then it was added in ns_signon_enforced_nick, wasn't it?
	*/
	
	del_regnick(nick);	
	return 1;
}
void lslog(char *fmt, ...)
{
        va_list ap;
        FILE *nickfile = fopen("nickserv.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!nickfile) {
		log("Unable to open nickserv.log for writing.");
		return;
	}

	fprintf(nickfile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(nickfile);

}

static void ns_recover(User *u, char *nick, char *pass) {
	NS_User *temp_user;
	User *target;
	char *tmp;

	if (!strcasecmp(u->nick, nick)) {
		privmsg(u->nick, s_NickServ, "Error, you can't recover yourself");
		return;
	}
	temp_user = findregnick(nick);
	target = finduser(nick);
	if (!temp_user) {
		privmsg(u->nick, s_NickServ, "Error, \002%s\002 is not online at the moment", nick);
		return;
	}
	if (temp_user->flags &= NSFL_SUSPEND) {
		privmsg(u->nick, s_NickServ, "ERROR, Your Nickname has been Suspended");
		return;
	}

	log("ns_recover password %s -> %s", temp_user->pass, pass);
	if (strcasecmp(temp_user->pass, pass)) {
		privmsg(u->nick, s_NickServ, "Error, Password incorrect");
		return;
	}
	if (temp_user->onlineflags == NSFL_ENFORCED) {
		/* this is a enforced Nick, just quit the Enforced User */
		ns_quit_enforced_nick(nick);
		privmsg(u->nick, s_NickServ, "Your Nickname has been Recovered");
		return;
	}
	
	tmp = smalloc(10);
	snprintf(tmp, 10, "Guest%i", guest_num++);
	
	/* make sure that the nickname isn't used, and if it is, keep trying till we find a nick not being used! */
	
	while (finduser(tmp)) {
		snprintf(tmp, 10, "Guest%i", guest_num++);
	}
	privmsg(nick, s_NickServ, "The Nick \002%s\002 was recovered by %s, your Nickname has been changed to %s", nick, u->nick, tmp);
	sts(":%s SVSMODE %s :-r", me.name, nick);
	UserMode(nick, "-r");
	sts(":%s SVSNICK %s %s :Recovery", s_NickServ, nick, tmp);
	Usr_Nick(nick, tmp);
	privmsg(u->nick, s_NickServ, "The Nick \002%s\002 was Recovered", nick);
	notice(s_NickServ, "The Nick \002%s(%s@%s)\002 was recovered by %s(%s@%s)", nick, target->username, target->hostname, u->nick, u->username, u->hostname);
	free(tmp);	

}

static void ns_ghost(User *u, char *nick, char *pass) {
	NS_User *temp_user;
	User *target;
	char *tmp;
	
	target = finduser(nick);
	if (!target) {	
		privmsg(u->nick, s_NickServ, "Error, That nick is not currently online");
		return;
	}
	temp_user = findregnick(nick);
	if (!temp_user) {
		privmsg(u->nick, s_NickServ, "Error, The nick \002%s\002 is not registered", nick);
		return;
	}
	if (temp_user->flags &= NSFL_SUSPEND) {
		privmsg(u->nick, s_NickServ, "ERROR, Your Nickname has been Suspended");
		return;
	}
	if (strcasecmp(temp_user->pass, pass)) {
		privmsg(u->nick, s_NickServ, "Error, Incorrect Password");
		notice(s_NickServ, "%s Specified a Invalid Password for Ghost for %s", u->nick, nick);
		return;
	}
	privmsg(u->nick, s_NickServ, "Success. the Nickname %s has been killed", target->nick);
	sts(":%s SVSKILL %s :Ghost Command Used by %s(%s@%s)", s_NickServ, target->nick, u->nick, u->username, u->vhost); 
	notice(s_NickServ, "%s(%s@%s) Used Ghost Comand on %s(%s@%s)", u->nick, u->username, u->hostname, target->nick, target->username, target->hostname); 
	tmp = smalloc(255);
	snprintf(tmp, 255, "%s", target->nick);
	Usr_Kill(s_NickServ, tmp);
	free(tmp);
	free(temp_user);
	return;
}


static void ns_info(User *u, char *nick) {
	NS_User *temp_user;
	int myinfo =0, freemem = 0;
	

	temp_user = findregnick(nick);
	if (!temp_user) {
		temp_user = lookup_regnick(nick);
		freemem = 1;
		if (!temp_user) {
			privmsg(u->nick, s_NickServ, "Error: The Nickname \002%s\002 has not been registered", nick);
/*			free(temp_user); */
			return;
		}
	} 
	/* see if they are online */

	/* if they want info on themselves, then set myinfo 1 */
	if (!strcasecmp(temp_user->nick, u->nick)) {
		if ((temp_user->secure == 1) && (temp_user->onlineflags &= NSFL_IDENTIFED)) {
			myinfo = 1;
		} else if (temp_user->secure == 0) {
			myinfo = 1;
		}
	}
	log("%d", temp_user->onlineflags);
	privmsg(u->nick, s_NickServ, "Info on: \002%s\002", temp_user->nick);
	if (temp_user->flags &= NSFL_SUSPEND) privmsg(u->nick, s_NickServ, "\002%s has been suspended for use by a Operator\002", temp_user->nick);

/* TODO: This doesn't work too well.. Figure it out */		
		
		
	if (strlen(temp_user->info) > 0) privmsg(u->nick, s_NickServ, "%s is \002%s\002", u->nick, temp_user->info);
	if ((temp_user->onlineflags &= NSFL_IDENTIFED) || (temp_user->onlineflags &= NSFL_ACCESSMATCH)) {
		privmsg(u->nick, s_NickServ, "%s is \002Online now. Do /whois %s\002", temp_user->nick, temp_user->nick);
	}
/*	privmsg(u->nick, s_NickServ, "\002%s\002 Registered on \002%ul\002", temp_user->registered_at); */
	privmsg(u->nick, s_NickServ, "Last seen on: \002%d\002", temp_user->last_seen_at);

	if ((temp_user->private == 0) || myinfo == 1) {
		privmsg(u->nick, s_NickServ, "Last Connected From: \002%s\002", temp_user->last_mask);
		privmsg(u->nick, s_NickServ, "Last Quit Message: \002%s\002", temp_user->last_quit);
		if (strlen(temp_user->url) > 1) privmsg(u->nick, s_NickServ, "URL: \002%s\002", temp_user->url);
		privmsg(u->nick, s_NickServ, "Email: \002%s\002", temp_user->email);
		if (strlen(temp_user->aim) > 1) privmsg(u->nick, s_NickServ, "AIM Handle: \002%s\002", temp_user->aim);
		if (temp_user->icq != 0) privmsg(u->nick, s_NickServ, "ICQ Number: \002%ld\002", temp_user->icq);
	}  	
	if (myinfo == 1) {
		privmsg(u->nick, s_NickServ, "Vhost: %s", strlen(temp_user->vhost) ? temp_user->vhost : "NOT SET");
		privmsg(u->nick, s_NickServ, "Channel Greeting: %s", strlen(temp_user->greet) ? temp_user->greet : "NOT SET");
		privmsg(u->nick, s_NickServ, "Kill Protection:      \002%s\002", temp_user->kill ? "ON" : "OFF");
		privmsg(u->nick, s_NickServ, "Private Flag:         \002%s\002", temp_user->private ? "ON" : "OFF");
		privmsg(u->nick, s_NickServ, "Secure Flag:          \002%s\002", temp_user->secure ? "ON" : "OFF");
	}
	if (UserLevel(u)  >= OPERCOMMENT_SEE_ACCESS_LEVEL) {
		if (strlen(temp_user->opercomment) > 0) privmsg(u->nick, s_NickServ, "OperComment: %s", temp_user->opercomment);
	}
/*	if (freemem == 1) free(temp_user); */
}


static void ns_identify(User *u, char *pass) {
	NS_User *temp_user;
	int i=0;
	DBT key, data;
	char *temp;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));


	/* first, see if they are already in the registered_nicks list */
	temp_user = findregnick(u->nick);


	if (!temp_user) {
		privmsg(u->nick, s_NickServ, "Error, Your Nickname is not Registered");
		return;
	}
	if (temp_user->flags &= NSFL_SUSPEND) {
		privmsg(u->nick, s_NickServ, "ERROR, Your Nickname has been Suspended");
		return;
	}

	if (i == 0) {
		if (!strcasecmp(temp_user->nick, u->nick)) {
			log("%s %s", temp_user->pass, pass);
			if (!strcasecmp(temp_user->pass, pass)) {
				privmsg(u->nick, s_NickServ, "You are Now Identified");
				sts(":%s SVSMODE %s :+r", me.name, u->nick);
				UserMode(u->nick, "+r");
				temp = malloc(sizeof(u->username) + sizeof(u->vhost) + 2);
				strcpy(temp, u->username);
				strcat(temp, "@");
				strcat(temp, u->vhost);
				strcpy(temp_user->last_mask, temp);
				free(temp);
				temp_user->lastchange = time(NULL);
				temp_user->last_seen_at = time(NULL);
				temp_user->onlineflags |= NSFL_IDENTIFED;
				if (strlen(temp_user->vhost) > 0) sts(":%s CHGHOST %s %s", s_NickServ, u->nick, temp_user->vhost);
				return;			
			} else {
				privmsg(u->nick, s_NickServ, "Invalid Password");
				sts(":%s SVSMODE %s :-r", me.name, u->nick);
				UserMode(u->nick, "-r");
				temp_user->kill++;
				if ((temp_user->kill -1) >= NO_BAD_PASS) {
					sts(":%s SVSKILL %s :%d Bad Passwords for nick %s", s_NickServ, u->nick, NO_BAD_PASS, u->nick); 
					Usr_Kill(s_NickServ, u->nick);
				} 
				return;
			}
		}
	}
	return;

}


static void ns_register(User *u, char *email, char *pass) {

	NS_User *new_user;
	char userid[15], hostmask[100]; int i, j=0; 
	DBT key, data;

	new_user = new_regnick(u->nick,1);
	if (strlen(new_user->pass) > 0) {
		privmsg(u->nick, s_NickServ, "Error: The Nickname \002%s\002 has already been registered", u->nick);
		return;
	}
	if (is_forbidden(u->nick)) {
		privmsg(u->nick, s_NickServ, "Error: That nickname is Forbidden from being registered", u->nick);
		return;
	}


	bzero(hostmask, 100);
	strncpy(new_user->nick, u->nick, strlen(u->nick));
	strncpy(new_user->pass, pass, strlen(pass));
	new_user->registered_at = time(NULL);
/* Check their email address, make sure it *looks* valid */
	strncpy(new_user->email, email, strlen(email));
/*	strncpy(new_user->last_realname, u->realname, 100); */
	strncpy(userid, u->username, strlen(u->username));
	new_user->kill = DEF_KILL;
	new_user->secure = DEF_SECURE;
	new_user->private = DEF_PRIVATE;	
	/* if their ident contains ~, make the accesslist be *ident@*hostname */
	
	if ((strncmp(userid, "~", 1) ==0)) {
		userid[0] = '*';
	}
	
/* TODO: handle IP address and hostnames */	
/* this only does hostnames... */	
	
	for (i=0; i<strlen(u->hostname); i++) {
		if (!j && u->hostname[i] == '.') {
			j = i+1;
		} else if (j) {
			hostmask[i-j+1] = u->hostname[i];
		}
	}
	hostmask[0] = '*';
	snprintf(new_user->acl, 1024, "%s %s@%s", new_user->acl, userid, hostmask);
	

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	key.data = (void *)u->nick;
	key.size = strlen(u->nick);
	data.data = (void *)new_user;
	data.size = sizeof(*new_user);
	i = dbp->put(dbp, NULL, &key, &data, 0);
	if (i != 0) {
		privmsg(u->nick, s_NickServ, "A Error has occured Please Try Later");
		notice(s_NickServ, "Register: DB Error: %s", db_strerror(i));
		log("NS Register: DB Error %s", db_strerror(i));
		return;
	}
	dbp->sync(dbp, 0);
	privmsg(u->nick, s_NickServ, "Nickname \002%s\002 Registered under your account %s", u->nick, new_user->acl);
	privmsg(u->nick, s_NickServ, "Your Password is \002%s\002 - Remember this for Later use.", new_user->pass);
	UserMode(u->nick, "+r");
	sts(":%s SVSMODE %s :+r", me.name, u->nick);
	return;

}

