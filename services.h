/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: services.h,v 1.3 2002/03/31 08:37:14 fishwaldo Exp $
*/

#ifndef M_SERVICES
#define M_SERVICES

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <db.h>

#include "stats.h"
#include <hash.h>
#include "options.h"


DB *dbp;
char *s_NickServ;

/* used to determine which nicks have changed, and need sync'ing */
int last_db_sync;


char ns_forbid_list[4096];
typedef struct ns_user NS_User;
hash_t *nsuserlist;


struct ns_user {
	NS_User *prev;
	NS_User *next;
	long hash;
	char nick[15];
	char pass[15];
	int language;
	char url[100];
	char email[100];
	int icq;
	char aim[100];
	int kill;
	int secure;
	int private;
	int onlineflags;
	int lastchange;
	char acl[1024];
	struct {
		char usermask:1;
		char email:1;
		char quit:1;
	} hide;
	int registered_at;
	int last_seen_at;
	int flags;
	char last_mask[100];
	char vhost[100];
	char last_quit[200];
	char info[100];
	char greet[100];
	char opercomment[255];
};

typedef struct svs_timers Svs_Timers;
Svs_Timers *serv_timers[MAX_SVS_TIMERS];

struct svs_timers {
	Svs_Timers *prev;
	Svs_Timers *next;
	long hash;
	char varibles[255];
	char timername[255];
	int interval;
	time_t lastrun;
	int (*function)();
};

struct ns_access {
	struct ns_access *prev;
	struct ns_access *next;
	char hostmask[100];
};

typedef struct ns_access NS_AccessList;

struct ns_links {
	struct ns_links *prev;
	struct ns_links *next;
	NS_User *user;
	char nick[15];
};

typedef struct ns_links NS_LinkList;

int NSDefLanguage;

int match2(char *, char *);
void init_regnick_hash();
NS_User *new_regnick(char *, int create);
NS_User *findregnick(char *);
void del_regnick(char *);
NS_User *lookup_regnick(char *);
int ns_new_user(User *);
int ns_bye_user(User *);
int ns_nickchange_user(char *);
Svs_Timers *new_svs_timers(char *);
void sync_nick_to_db(NS_User *);
void del_nick_from_db(char *);
char *makemask(User *u, int vhost);
void init_nick_forbid_list();
void save_nick_forbid_list();
int is_forbidden(char *);
char *strnrepl(char *s, int size, const char *old, const char *new);
void lang_init();
extern void send_help(User *u, char *line);
void notice_help(const char *source, char *dest, int message, ...);


/* these functions are in the core neostats, and are needed for internal functions */
void Usr_Kill(char *, char *);
void Usr_Nick(char *, char *);
int get_dl_handle(char *);



/* this is just stuff we need */
#define getstring(na,index) \
	(langtexts[((na)&&((NS_User*)na)->language?((NS_User*)na)->language:NSDefLanguage)][(index)])





/* Flags for NS_User->onlineflags */

#define NSFL_IDENTIFED		0x0001 /* user has Identified */
#define NSFL_BADPASS		0x0002 /* User specified a badpass */
#define NSFL_ACCESSMATCH	0x0004 /* user has been matched against the accesslist */
#define NSFL_ENFORCED		0x0008 /* this is a Enforcer.. ie, not a real user */
#define NSFL_SUSPEND		0x0010 /* nickname is suspended */



#define NUM_LANGS		10
#define LANG_EN_US		0       /* United States English */
#define USED_LANGS		6   /* Number of languages provided */
#define DEF_LANGUAGE		LANG_EN_US

#endif

