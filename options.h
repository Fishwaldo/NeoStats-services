/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: options.h,v 1.2 2002/03/28 08:58:16 fishwaldo Exp $
*/


/* this is the config file for services. 
** anything that is configurable should be set via this file for now
** until we have the config interface coded and into the core neostats software
**
** I'll try to document each setting and what it does
*/


#ifndef M_OPTIONS
#define M_OPTIONS



/* this setting is the maxium no of registered online nicks at any one tiem
** if more users than this setting identify 
** god knows what will happen to Neostats (Most likely it will core 
**
** just setting this to a high value is not a good idea if you dont need it
** its going to increase memory usage, and also probably slow down NeoStats
** when it does its Database Sync's, as it will have to go through each record, even 
** if its not being used 
*/

#define NS_USER_LIST 	1024


/* This is the Maxium number of timers that services can activate at any one time
** things that use timers at the moment are
** - when a registered nick sign's on, and its kill option is active, but its not in a access list
** - When nicks are changed (upto 2 timers as well!)
**
** again, setting this high, might lag out NeoStats, so be carefull with it.
*/

#define MAX_SVS_TIMERS  255

/* this defines the databaes Location.
** you can put it anywhere you want, if Services can't find a database, it will
** automatically create the database for you
** if the database gets corrupted you can use the Berkeley database recovery tools
** on it
*/

#define NSDBASE 	"data/nsdata.db"


/* how long do users have to ident to nickserv before their nick is changed.
** this is set if kill is active for that nick (kill isn't a good word, read the online help about set kill.
** This relates to timers above, so setting this to a long time, and having lots of users pending "identification"
** can mean you run out of timer slots.. be carefull
*/

#define NICK_IDENT_TIME		30

/* how long does a enforcer stay online 
** if a Enforcer Nick joins the network to stop people regaining the nickname
** how long does it stay online?
** again, relates to timers above, so set this carefully
*/

#define ENFORCER_HOLD_TIME	30

/* how often to check registered nick records that need to be synced to the database
** each online nick has a "lastchanged" field.
** every period you set here (Seconds) Services goes through the list of nicks
** looking for nicks that need to have the database synced (for instance, if they update the 
** set interface or so on.
**
** Setting this too low will lag out NeoStats, Setting it too high might mean if you have a lot of nicks
** that need to be synced, NeoStats gets lagged out for a while, as the database is synced
** I suggest around 5-10 minutes for most networks
** if Services thinks its taking too long to sync the database
** it will broadcast a message into the services channel, you should change it then
*/

#define DB_SYNC_TIME		30

/* how long can user have their info line (viewable via /nickserv info <nick>)
** some networks prefer to limit how long it can be
** but you can't go over 100, 
** if you do, it will still be limited to 100, as thats the database field size
*/ 

#define NICK_INFO_LEN		100 /* is the max */

/* No of bad passwords before kill'ing the user 
** if the number of bad passwords is this, then the user is killed from the network
*/

#define NO_BAD_PASS		3

/* Default Kill setting for New nicks 
** what default setting should new registered nicks have?
** setting this to 1 means that kill is active
** setting to 0 means that kill is not active
** setting to anything higher than 1 will work as well, but wont change the function of it
*/ 

#define DEF_KILL		1

/* Default secure settings for new nicks 
** what default setting shoudl new registered nicks have?
** setting this to 1 means secure is active
** setting to 0 means its not active
*/

#define DEF_SECURE		0

/* default private setting for new nicks 
** what default setting should new registered nicks have?
** setting this to 1 means private is active
** setting to 0 means its not active
*/

#define DEF_PRIVATE		0

/* What level does a user have to be to set the opercomment
** You should look at the NeoStats core file users.c to figure out levels
** Default, is 50 (Global Oper)
*/

#define OPERCOMMENT_SET_ACCESS_LEVEL	50

/* what level does a user have to be to see the opercomment
** you should look at the Neostats core file users.c to figure out levels
** default, is 40 (LocalOp)
*/

#define OPERCOMMENT_SEE_ACCESS_LEVEL	40

/* what level is required to drop a nickname
** you should look at the Neostats core file users.c to figure out levels
** default, is 100 (Service Admins)
*/

#define CAN_DROP_NICK_LEVEL	100

/* what level is reqiured to suspend a nickname
** you should look at the neostats core file users.c to figure out levels
** default is 100 (service admins)
*/

#define OPERSUSPEND_SET_ACCESS_LEVEL	100

/* what level is required to forbid a nickname
** default is 100 (Service Admins)
*/

#define OPERFORBID_SET_ACCESS_LEVEL 	100


/* should we enable getpass for nickserv, chanset et al?
** value of 1 says yes
** value of 0 says no
*/

#define ENABLE_GETPASS			1

/* if ENABLE_GETPASS is 1 above, then what level to we require for 
** operators to use this command?
** default is 100 (service admins)
*/

#define GETPASS_ACCESS_LEVEL		100


/* should we enable setpass for nickserv, chanserv et al?
** value of 1 says yes
** value of 0 says no
*/

#define ENABLE_SETPASS			1

/* if ENABLE_SETPASS is 1 above, then what level to we require for 
** operators to use this command?
** default is 100 (service admins)
*/

#define SETPASS_ACCESS_LEVEL		100


/* Vhost Support. 
** 3 settings here for networks, this is how it works
** if set to 0, then vhost is disabled
** if set to 1, then users are allowed to set their own vhost
** if set to 2, then users get vhost on identify/accesslist match, but can not change, only opers can change
*/

#define ENABLE_VHOST			2

/* if ENABLE_VHOST is set to 2 then what access
** level is required to set a users Vhost?
** default is 100 (Service Admins)
** if ENABLE_VHOST is 1 then this is what level a admin can change the vhost
** if ENABLE_VHOST is 0, this has no effect
*/ 

#define SETVHOST_ACCESS_LEVEL		100




#endif
