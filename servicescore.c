/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module:  LoveServ
** Version: 1.0
*/

/* TODO STUFF
** Delete or add as u do stuff
*/




#include <stdio.h>
#include <db.h>
#include "dl.h"
#include "stats.h"
#include "services.h"

const char servicesversion_date[] = __DATE__;
const char servicesversion_time[] = __TIME__;


Module_Info my_info[] = { {
	"Services",
	"Services for Neostats",
	"0.1.4A"
} };


int new_m_version(char *origin, char **av, char *tmp) {
	snumeric_cmd(351, origin, "Module NickServ Loaded, Version %s %s %s",my_info[0].module_version,servicesversion_date,servicesversion_time);
	return 0;
}

Functions my_fn_list[] = {
	{ "MSG_VERSION",	new_m_version,	1 },
	{ "TOK_VERSION", 	new_m_version,  1 },
	{ NULL,		NULL,		0 }
};


int Online(Server *data) {

	if (init_bot(s_NickServ,"nick",me.name,"Network Nick Service", "+xd", my_info[0].module_name) == -1 ) {
		/* Nick was in use!!!! */
		s_NickServ = strcat(s_NickServ, "_");
		init_bot(s_NickServ,"nick",me.name,"Network Nick Service", "+xd", my_info[0].module_name);
	}
/* TODO: don't use constants here */
	add_mod_timer("runsvstimers", "Services", "Services", 1);
	add_mod_timer("sync_changed_nicks_to_db", "DB_Sync", "Services", DB_SYNC_TIME);
	return 1;
};

EventFnList my_event_list[] = {
	{ "ONLINE", 		(void *)Online},
	{ "SIGNON", 		(void *)ns_new_user},
	{ "SIGNOFF", 		(void *)ns_bye_user},
	{ "KILL",		(void *)ns_bye_user},
	{ "NICK_CHANGE",	(void *)ns_nickchange_user},
	{ NULL, 		NULL}
};



Module_Info *__module_get_info() {
	return my_info;
};

Functions *__module_get_functions() {
	return my_fn_list;
};

EventFnList *__module_get_events() {
	return my_event_list;
};

void _init() {
	int ret;

	s_NickServ = "NS";


	ret = db_create(&dbp, NULL,0);
	if (ret != 0) {
		log("nickserv dbcreate error");
		return;
	}
	ret = dbp->open(dbp, NSDBASE, NULL, DB_HASH, 0, 0644);
	if (ret == ENOENT) { 
		/* db doesn't exist */
		log("nickserv Database Doesn't Exist, Creating it!");
		ret = dbp->open(dbp, NSDBASE, NULL, DB_HASH, DB_CREATE, 0644);
	}
	if (ret != 0) {
		log("nickserv dbopen error %s", db_strerror(ret));
		return;
	}
	log("NickServ Loaded");
	init_regnick_hash();
	/* load the nickserv forbidden list */
	init_nick_forbid_list();
	lang_init();
}


void _fini() {
	int ret;
	if (dbp != NULL) {
		if ((ret = dbp->close(dbp, 0) != 0)) {
			globops("NickServ DB Close Error: %s", db_strerror(ret));
		};
	}
	log("NickServ Unloaded");
};





char *makemask(User *u, int vhost) {
	char userid[15], hostmask[255], *temp;
	int i,j=0;

	bzero(userid, 15);
	bzero(hostmask, 255);

	strncpy(userid, u->username, strlen(u->username)); 
	if ((strncmp(userid, "~", 1) ==0)) {
		userid[0] = '*';
	}
	
/* TODO: handle IP address and hostnames */	
/* this only does hostnames... */	

	if (vhost == 1) {
		for (i=0; i<strlen(u->vhost); i++) {
			if (!j && u->vhost[i] == '.') {
				j = i+1;
			} else if (j) {
				hostmask[i-j+1] = u->vhost[i];
			}
		}
	
	} else {	
		for (i=0; i<strlen(u->hostname); i++) {
			if (!j && u->hostname[i] == '.') {
				j = i+1;
			} else if (j) {
				hostmask[i-j+1] = u->hostname[i];
			}
		}
	}
	hostmask[0] = '*';
	temp = malloc(sizeof(userid) + sizeof(hostmask) +1);
	snprintf(temp, strlen(userid) + strlen(hostmask) +2 , "%s@%s", userid, hostmask);
	return (char *)temp;
}
/*************************************************************************/

/* strnrepl:  Replace occurrences of `old' with `new' in string `s'.  Stop
 *            replacing if a replacement would cause the string to exceed
 *            `size' bytes (including the null terminator).  Return the
 *            string.
 */

char *strnrepl(char *s, int size, const char *old, const char *new)
{
    char *ptr = s;
    int left = strlen(s);
    int avail = size - (left+1);
    int oldlen = strlen(old);
    int newlen = strlen(new);
    int diff = newlen - oldlen;

    while (left >= oldlen) {
	if (strncmp(ptr, old, oldlen) != 0) {
	    left--;
	    ptr++;
	    continue;
	}
	if (diff > avail)
	    break;
	if (diff != 0)
	    memmove(ptr+oldlen+diff, ptr+oldlen, left+1);
	strncpy(ptr, new, newlen);
	ptr += newlen;
	left -= oldlen;
    }
    return s;
}
