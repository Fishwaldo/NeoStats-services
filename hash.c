/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: hash.c,v 1.3 2002/03/31 08:37:14 fishwaldo Exp $
*/



#include <fnmatch.h>
#include "services.h"


void del_svs_dead_timers();



void sync_changed_nicks_to_db() {
	NS_User *tmp;
	hnode_t *nsn;
	hscan_t nss;
	int j = 0;
	int starttime;
	
	starttime = time(NULL);
	hash_scan_begin(&nss, nsuserlist);
	while ((nsn = hash_scan_next(&nss))) {
		tmp = hnode_get(nsn);
		if (tmp->lastchange > last_db_sync) {
			tmp->lastchange = time(NULL);
#ifdef DEBUG
			log("Syncing Nick %s", tmp->nick);
#endif		
			sync_nick_to_db(tmp);
			j++;
		}
	}
	last_db_sync = time(NULL);
	/* we always sync anyway, in case other gets/puts have happened outside this timer */
	dbp->sync(dbp, 0);

	/* this checks how long it took to sync the databaes 
	** if it took too long, then it warns the opers
	** and asks them to decrease the database sync time
	** to stop lag of NeoStats
	*/
	if ((time(NULL) - starttime) > 5) {
		notice(s_NickServ, "\002Warning, Database Sync is exceeding threasholds, NeoStats is Lagging out\002");
		notice(s_NickServ, "\002Suggest you Change the Database Sync interval to something smaller\002");
		notice(s_NickServ, "\002We Synced %d Records in %d Secs...\002", j, time(NULL) - starttime);
	}
	
}	

void sync_nick_to_db(NS_User *tmp) {
	DBT key, data;
	int i;
	
        memset(&key, 0, sizeof(key)); 
        memset(&data, 0, sizeof(data));	
	key.data = (void *)tmp->nick;
	key.size = strlen(tmp->nick);
	data.data = (void *)tmp;
	data.size = sizeof(*tmp);
	i = dbp->put(dbp, NULL, &key, &data, 0);
	if (i != 0) {
		log("Database Sync Error for %s: %s", tmp->nick, db_strerror(i));
		notice(s_NickServ, "\002Database Sync Error\002 for %s: %s", tmp->nick, db_strerror(i));
		return;
	}
}

void del_nick_from_db(char *nick) {
	DBT key;
	int i;
	
	memset(&key, 0, sizeof(key));
	key.data = (void *)nick;
	key.size = strlen(nick);
	i = dbp->del(dbp, NULL, &key, 0);
	if (i != 0) {
		log("Database Sync Error for dropping nick %s: %s", nick, db_strerror(i));
		notice(s_NickServ, "\002Database Sync error for dropping nick %s: %s", nick, db_strerror(i));
		return;
	}	

}
void init_nick_forbid_list() 
{
	DBT key, data;
	int i;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	/* we store the forbidden lists in a key called ns_forbid_list - Hopefully no user tries to use this nick */
	key.data = "ns_forbid_list";
	key.size = strlen(key.data);
	i = dbp->get(dbp, NULL, &key, &data, 0);
	if (i == DB_NOTFOUND) {
		log("Warning: No Forbidden Nick list found in Database!");
		return;
	} else if (i != 0) {
		log("Error: Forbidden Nick list retrival from database %s", db_strerror(i));
		return;
	} else {
		strcpy(ns_forbid_list, data.data);
	}
}

void save_nick_forbid_list()
{
	DBT key, data;
	int i;
	
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	key.data = "ns_forbid_list";
	key.size = strlen(key.data);
	data.data = ns_forbid_list;
	data.size = strlen(ns_forbid_list);
	i = dbp->put(dbp, NULL, &key, &data, 0);
	if (i != 0) {
		log("Error: Forbidden Nick list save to database: %s", db_strerror(i));
		return;
	} else {
		notice(s_NickServ, "Saved Forbidden Nick List to database");
	}
}
int is_forbidden(char *nick) {
	char *tmp, *tmp2;
	
	tmp = malloc(strlen(ns_forbid_list));
	strcpy(tmp, ns_forbid_list);
	tmp2 = strtok(tmp, " ");
	while (tmp2) {
		if (fnmatch(tmp2, nick, 0)) {
			/* its a match */
			free(tmp);
			return 1;
		}
		tmp2 = strtok(NULL, " ");
	}
	free(tmp);
	return 0;
}	
	
void init_regnick_hash()
{
	nsuserlist = hash_create(NS_USER_LIST, 0, 0);
}

NS_User *lookup_regnick(char *name) 
{
	NS_User *ns2;
	DBT key, data;
	int i;

	ns2 = smalloc(sizeof(NS_User));
	bzero(ns2, sizeof(NS_User));
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));
	key.data = (char *)name;
	key.size = strlen(key.data);
	i = dbp->get(dbp, NULL, &key, &data, 0);
	if (i == DB_NOTFOUND) {
		free(ns2);
		return NULL;
	}
	if (i == 0) {
#ifdef DEBUG
		log("Getting RegNick From Database");
#endif
		ns2 = data.data;
		return ns2;
	} else {
		notice(s_NickServ, "DataBase Error %s", db_strerror(i));
		log("Database Error: %s", db_strerror(i));
	}		
	return NULL;
}

NS_User *new_regnick(char *name, int create)
{
	NS_User *ns, *ns2;
	hnode_t *nsn;
	DBT key, data;
	int i;

	ns = smalloc(sizeof(NS_User));
	ns2 = smalloc(sizeof(NS_User));
	bzero(ns, sizeof(NS_User));
	bzero(ns2, sizeof(NS_User));
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));
	key.data = name;
	key.size = strlen(key.data);
	i = dbp->get(dbp, NULL, &key, &data, 0);
	if ((i == DB_NOTFOUND) && (create == 0)) {
		free(ns);
		free(ns2);
		return NULL;
	}
	if ((i == 0) && (create ==0)) {
#ifdef DEBUG
		log("Getting RegNick From Database");
#endif
		ns2 = data.data;
		strcpy(ns->pass, ns2->pass);
		strcpy(ns->url, ns2->url);
		strcpy(ns->email, ns2->email);
		strcpy(ns->acl, ns2->acl);
		strcpy(ns->last_mask, ns2->last_mask);
		strcpy(ns->last_quit, ns2->last_quit);
		strcpy(ns->aim, ns2->aim);
		strcpy(ns->info, ns2->info);
		strcpy(ns->greet, ns2->greet);
		strcpy(ns->vhost, ns2->vhost);
		ns->icq = ns2->icq;
		ns->language = ns2->language;
		ns->kill = ns2->kill;
		ns->secure = ns2->secure;
		ns->private = ns2->private;
		ns->onlineflags = 0;
		ns->lastchange = 0;
		ns->registered_at = ns2->registered_at;
		ns->flags = ns2->flags;
		ns->last_seen_at = ns2->last_seen_at;
/*		free(ns2); */
	}	

	memcpy(ns->nick, name, strlen(name)+1);
	nsn = hnode_create(ns);
	if (!hash_isfull(nsuserlist)) {
		hash_insert(nsuserlist, nsn, name);
	} else {
		log("eeek, NickServ User list hash is full");
	}

	return ns;
}
NS_User *findregnick(char *name)
{
	NS_User *ns = NULL;
	hnode_t *nsn;
	
	nsn = hash_lookup(nsuserlist, name);
	if (nsn) {
		ns = hnode_get(nsn);
	}
#ifdef DEBUG
	log("findregnick(%s) -> %s", name, (ns) ? ns->nick : "NOTFOUND");
#endif

	return ns;
}
void del_regnick(char *name)
{
	NS_User *ns;
	hnode_t *nsn = hash_lookup(nsuserlist, name);

#ifdef DEBUG
	log("DelRegnick(%s)", name);
#endif

	if (!nsn) {
		log("Delregnick(%s) failed!", name);
		return;
	}
	ns = hnode_get(nsn);
	hnode_destroy(nsn);
	free(ns);
}



/* this is the Timer Hash setting... Things can be added to the services timer, which get called again (Consider them a timer within a timer!) */


static void add_svstimer_to_hash_table(char *name, Svs_Timers *svstmr)
{
	svstmr->hash = HASH(name, MAX_SVS_TIMERS);
	svstmr->next = serv_timers[svstmr->hash];
	serv_timers[svstmr->hash] = (void *)svstmr;
}

static void del_svstimer_from_hash_table(char *name, Svs_Timers *svstmr)
{
	Svs_Timers *tmp, *prev = NULL;

	for (tmp = serv_timers[svstmr->hash]; tmp; tmp = tmp->next) {
		if (tmp == svstmr) {
			if (prev)
				prev->next = tmp->next;
			else
				serv_timers[svstmr->hash] = tmp->next;
			tmp->next = NULL;
			return;
		}
		prev = tmp;
	}
}

void init_svstimer_hash()
{
	int i;
	Svs_Timers *svstmr, *prev;

	for (i = 0; i < MAX_SVS_TIMERS; i++) {
		svstmr = serv_timers[i];
		while (svstmr) {
			prev = svstmr->next;
			free(svstmr);

			svstmr = prev;
		}
		serv_timers[i] = NULL;
	}
	bzero((char *)serv_timers, sizeof(serv_timers));
}

int runsvstimers()
{
	Svs_Timers *svstmr;
	time_t current = time(NULL);
	int i, ret = 0;
	int startime, count = 0;

	startime = time(NULL);
	for (i = 0; i < MAX_SVS_TIMERS; i++) {
		svstmr = serv_timers[i];
		while (svstmr) {
			if (current - svstmr->lastrun >= svstmr->interval) {
#ifdef DEBUG
				log("runsvstimers(%s)", svstmr->timername);
#endif
				svstmr->lastrun = time(NULL);
				ret = svstmr->function(svstmr->varibles);
				/* if ret = 1 then delete the timer */
				if (ret == 1) {
					svstmr->interval = 0;
				}
				count++;
			}
			svstmr = svstmr->next;
		}
	}
	/* only run garbage cleanup if there is something to clean up */
	if (ret == 1) del_svs_dead_timers();

	if ((time(NULL) - startime) > 5) {
		notice(s_NickServ, "\002Warning, Timers are taking a long time to run\002");
		notice(s_NickServ, "\002This is Lagging out NeoStats\002");
		notice(s_NickServ, "\002Suggest you Change your Timer Settings (Such a EnforcerTime, Identtime) to something smaller\002");
		notice(s_NickServ, "\002If this message keeps re-occuring. (it might be normal when a split server rejoins)\002");
		notice(s_NickServ, "\002We Ran %d timers and it took %d Sec...\002", count, time(NULL) - startime);
	}
		

	return 1;
}

void del_svs_dead_timers()
{
	Svs_Timers *svstmr;
	int i;
#ifdef DEBUG
	log("Services Timer Garbage Cleanup");
#endif
	for (i = 0; i < MAX_SVS_TIMERS; i++) {
		svstmr = serv_timers[i];
		while (svstmr) {
			if (svstmr->interval == 0) {
			del_svstimer_from_hash_table(svstmr->timername, svstmr);
			}
		svstmr = svstmr->next;
		}
	}
}
void del_svs_timers(char *name)
{
	Svs_Timers *svstmr;

#ifdef DEBUG
	log("Delsvstimer(%s)", name);
#endif

	svstmr = serv_timers[HASH(name, MAX_SVS_TIMERS)];
	while (svstmr && strcasecmp(svstmr->timername, name) != 0)
		svstmr = svstmr->next;

	if (!svstmr) {
		log("Delsvstimer(%s) failed!", name);
		return;
	}

	del_svstimer_from_hash_table(name, svstmr);
}



Svs_Timers *new_svs_timers(char *name)
{
	Svs_Timers *svstmr;

	svstmr = malloc(sizeof(Svs_Timers));
	bzero(svstmr, sizeof(Svs_Timers));
#ifdef DEBUG
	log("New Services Timer: %s", name);
#endif	
	memcpy(svstmr->timername, name, strlen(name));  
	add_svstimer_to_hash_table(name, svstmr);

	return svstmr;
}
