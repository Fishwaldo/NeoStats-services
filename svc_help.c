/* NeoStats - IRC Statistical Services Copryight (c) 1999-2001 NeoStats Group.
*
** Module: Services
*/

#include "language.h"
#include "services.h"
#include "stats.h"

struct int_help {
        char *name;
        int function;
        int level; 
};
typedef struct int_help SVC_help_ns;


const SVC_help_ns svc_ns_help_index[] = {
	/* help, 	help function, 			level */	
	{"HELP", 	NICKSERV_HELP,  		0},
	{"HELP", 	NICKSERV_OPERHELP, 		40},
	{"HELP", 	NICKSERV_HELPOPERCOMMENT, 	OPERCOMMENT_SET_ACCESS_LEVEL},
	{"HELP", 	NICKSERV_HELPSUSPEND,		OPERSUSPEND_SET_ACCESS_LEVEL},
	{"HELP", 	NICKSERV_HELPFORBID, 		OPERFORBID_SET_ACCESS_LEVEL},
	{"HELP", 	NICKSERV_HELPGETPASS, 		GETPASS_ACCESS_LEVEL},
	{"HELP", 	NICKSERV_HELPSETPASS, 		SETPASS_ACCESS_LEVEL},
	{"HELP", 	NICKSERV_HELPVHOST,		SETVHOST_ACCESS_LEVEL},
	{"REGISTER", 	NICKSERV_HELP_REGISTER, 	0},
	{"IDENTIFY", 	NICKSERV_HELP_IDENTIFY,		0},
	{"INFO",	NICKSERV_HELP_INFO,		0},
	{"INFO",	NICKSERV_HELP_INFO_OPERS,	40},
	{"GHOST",	NICKSERV_HELP_GHOST,		0},
	{"RECOVER",	NICKSERV_HELP_RECOVER,		0},
	{"DROP", 	NICKSERV_HELP_OPERDROP,		CAN_DROP_NICK_LEVEL},
	{"DROP",	NICKSERV_HELP_DROP,		0},
	{"LOGOUT",	NICKSERV_HELP_LOGOUT, 		0},
	{"SET",		NICKSERV_HELP_SET,		0},
	{"SET PASSWORD",NICKSERV_HELP_SET_PASSWORD,	0},
	{"SET URL",	NICKSERV_HELP_SET_URL,		0},
	{"SET EMAIL",	NICKSERV_HELP_SET_EMAIL,	0},
	{"SET ICQ",	NICKSERV_HELP_SET_ICQ,		0},
	{"SET INFO",	NICKSERV_HELP_SET_INFO,		0},
	{"SET AIM",	NICKSERV_HELP_SET_AIM,		0},
	{"SET GREET",	NICKSERV_HELP_SET_GREET,	0},
	{"SET KILL",	NICKSERV_HELP_SET_KILL,	 	0},
	{"SET SECURE",	NICKSERV_HELP_SET_SECURE,	0},
	{"SET PRIVATE",	NICKSERV_HELP_SET_PRIVATE,	0},
	{"SET VHOST",	NICKSERV_HELP_SET_VHOST,	0},
	{"OPERCOMMENT",	NICKSERV_HELP_OPERCOMMENT,	OPERCOMMENT_SET_ACCESS_LEVEL},
	{"SUSPEND",	NICKSERV_HELP_SUSPEND,		OPERSUSPEND_SET_ACCESS_LEVEL},
	{"FORBID",	NICKSERV_HELP_FORBID,		OPERFORBID_SET_ACCESS_LEVEL},
	{"GETPASS",	NICKSERV_HELP_GETPASS,		GETPASS_ACCESS_LEVEL},
	{"SETPASS",	NICKSERV_HELP_SETPASS,		SETPASS_ACCESS_LEVEL},
	{"VHOST",	NICKSERV_HELP_VHOST,		SETVHOST_ACCESS_LEVEL},
	{"ACCESS",	NICKSERV_HELP_ACCESS,		0},
	{NULL, 		NULL, 		0}
};

extern void send_help(User *u, char *line) {
	int i;
	if (!line) {
		line = smalloc(5);
		strcpy(line, "HELP");
	}
	for (i = 0; i < ((sizeof(svc_ns_help_index) / sizeof(svc_ns_help_index[0])) -1); i++) {
	log("%s - %s %d", svc_ns_help_index[i].name, line, strcasecmp(svc_ns_help_index[i].name, line));
		if (!strcasecmp(svc_ns_help_index[i].name, line)) {
			if (UserLevel(u) >= svc_ns_help_index[i].level) {
				notice_help(s_NickServ, u->nick, svc_ns_help_index[i].function);
			}
		}
	}
}
