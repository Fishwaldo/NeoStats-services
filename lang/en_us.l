# United States English language file.
#
# Epona (c) 2000-2001 PegSoft
# Contact us at epona@pegsoft.net
#
# This program is free but copyrighted software; see the file COPYING for
# details.
#
# Based on the original code of Services by Andy Church.

# When translating this file to another language, keep in mind that the
# order of parameters for sprintf() is fixed in the source code, so your
# messages need to take the same parameters in the same order as the
# English messages do.  (Obviously, this doesn't hold for the strftime()
# format lines immediately below.)  If you can't get a natural translation
# of a message without changing the order of the parameters, let me know
# (achurch@dragonfire.net) which message is causing a problem and I'll see
# what I can do.
#
# In help messages, "%S" (capital S, not lowercase) refers to the name of
# the service sending the message; for example, in NickServ help messages,
# "%S" is replaced by "NickServ" (or whatever it is renamed to in
# config.h).  The %S's do not count as sprintf() parameters, so they can be
# rearranged, removed, or added as necessary.
#
# Also in help messages, please try to limit line lengths to 60 characters
# of text (not including the leading tab).  This length was chosen because
# it does not cause line wrap under default settings on most current IRC
# clients.  Remember that format characters (control-B, control-_) are not
# included in that 60-character limit (since they don't show on the user's
# screen).  Also remember that format specifiers (%S, etc.) will expand
# when displayed, so remember to take this into account; you can assume
# that the length of a pseudoclient name (%S replacement) will be eight
# characters, so reduce the maximum line length by 6 for every %S on a
# line.
#
# Finally, remember to put a tab at the beginning of every line of text
# (including empty lines).  This has to be a tab, not spaces.

#


###########################################################################
#
# Name of this language
#
###########################################################################

# For languages other than English, this string should have the following
# format:
#     language-name-in-language (language-name-in-English)
# For example, "Español (Spanish)" or "Français (French)".

LANG_NAME
	English

###########################################################################
#
# General messages
#
###########################################################################

# strftime() format strings.  man 3 strftime for information on the
# meanings of the format specifiers.  Short version:
#	%a = weekday name (short)	%H = hour
#	%b = month name (short)		%M = minute
#	%d = day of month		%S = second
#	%Y = year			%Z = time zone

# This is used as the format string for strftime() for a date and time
# together.
STRFTIME_DATE_TIME_FORMAT
	%b %d %H:%M:%S %Y %Z
# This is used as the format string for strftime() for a date alone in long
# format (English: including weekday).
STRFTIME_LONG_DATE_FORMAT
	%a %b %d %Y
# This is used as the format string for strftime() for a date alone in
# short format (English: without weekday).
STRFTIME_SHORT_DATE_FORMAT
	%b %d %Y

# These tell our strftime() what the names of months and days are.  If you
# don't use %a, %A, %b, or %B in your strftime() strings above, you can
# leave these empty.  However, if you enter names, they MUST stay in order,
# one per line, and the list MUST be complete!

# %a
STRFTIME_DAYS_SHORT
	Sun
	Mon
	Tue
	Wed
	Thu
	Fri
	Sat
# %A
STRFTIME_DAYS_LONG
	Sunday
	Monday
	Tuesday
	Wednesday
	Thursday
	Friday
	Saturday
# %b
STRFTIME_MONTHS_SHORT
	Jan
	Feb
	Mar
	Apr
	May
	Jun
	Jul
	Aug
	Sep
	Oct
	Nov
	Dec
# %B
STRFTIME_MONTHS_LONG
	January
	February
	March
	April
	May
	June
	July
	August
	September
	October
	November
	December


# This is used in ChanServ/NickServ INFO displays.
COMMA_SPACE
	, 


# Various error messages.
NICKSERV_HELP
	%S allows a IRC user to Register, or own a Nickname on the
	IRC Network. This can be usefull to prevent other uses from using
	your Nickname and impersinating you. You can also use NickServ to
	store information about youself such as Email address, or webpage URL. 
	The Following list are the commands that you can use with %S.
	To get further help with these commands, or to find out what they 
	to, type /msg %S help <command>.
	
	Register  Allows you to Register a New NickName
	Identify  Allows you to identify for your nickname after registration
	Info  Retrive information about your nickname, or someone elses nickname
	Ghost  Kill a Ghost user that has your nickname
	Recover  Get your nickname released if a Enforcer has taken it
	Drop  Drop your nickname registration
	Logout  Logout from Services. (un-Identify)
	Set  Set various options for your nickname (/msg %S help set for more information)
	Access  Set your lists of hosts from which you want to be automatically identified 
	 
	NOTICE: This Service provides a interface for users to Register their Nickname,
	it is not indended to Steal nicknames on the network. Any abuse of
	this services will result in you loosing access to these services.
	
	Nicknames that are not used for a period of time can be "expired",
	meaning that the registration information is deleted, and the nickname is
	able to be registered by any other user. 
NICKSERV_OPERHELP
	
	Additional Commands available to Operators:
NICKSERV_HELPOPERCOMMENT
	OperComment 
NICKSERV_HELPSUSPEND
	Suspend 
NICKSERV_HELPFORBID
	Forbid 
NICKSERV_HELPGETPASS
	GetPass 
NICKSERV_HELPSETPASS
	SetPass 
NICKSERV_HELPVHOST
	Vhost 
NICKSERV_HELP_REGISTER
	Syntax: /msg %S register <email> <password>
	Register allows you to Register a nickname with services.
	This allows you to "own" this nickname on the irc network, and to
	use the advanced features of these Services.
	Set and Access allows you to configure this nickname
	registration to your prefered settings.
	Your email address is considered private, and will not be share with
	any 3rd parties, but may be viewed via the info command. (This can be
	disabled via a set command)
	You should use a password that is hard to guess, and contains both
	numbers and letters. This can stop people from easily guessing your password
	and taking over your nickname.
NICKSERV_HELP_IDENTIFY
	Syntax: /msg %S identify <password>
	Identify allows you to tell services that you really own
	your nickname. Identify is like a logon command for Services. A lot
	of Commands that services have require that you are "identified"
	(sent this command and recieved a positive acknowledgement)
NICKSERV_HELP_INFO
	Syntax: /msg %S info <nickname>
	Info allows you to view any public information on a
	particular registered nickname. Information such as the last time a user was
	seen on the network, Email addresses, or ICQ numbers maybe visible depending
	on what a user has set. 
	Additionally, if you specify your own nickname after identifing to
	Services, you can see your settings.
NICKSERV_HELP_INFO_OPERS
	Additionally, as a Operator, allows you to view all the users
	settings, including OperComments, or Vhost Settings.
NICKSERV_HELP_GHOST
	Syntax: /msg %S ghost <nickname> <password>
	Ghost Allows a User to regain their registered nickname, if has
	been taken by another user, or if it is a result of a Ghosted Connection.
	This can be a result of when you get disconnected from the irc
	server, but the server hasn't notice it yet.
NICKSERV_HELP_RECOVER
	Syntax: /msg %S recover <nickname> <password>
	Recover allows you to regain a nickname when someone else has
	taken your Nickname, but it works a bit differently. Recover will change the
	Person who currently has your Nick, to a Random Nickname, and will Sign on a
	"Enforcer" or fake user with your nickname. This can stop other
	people from automatically re-taking your nickname. This "Enforcer" user will
	disconnect after a short period, and allow you to take your Nickname. After
	which, you maybe required to Identify again
NICKSERV_HELP_OPERDROP
	Syntax: /msg %S drop <nickname>
	Drop allows you to drop a Registered Nickname if you have the
	correct Privledges with Services.
NICKSERV_HELP_DROP
	Syntax: /msg %S drop
	Drop will drop your nickname registration from the Services
	Database, and delete any records associated with any of the services to do
	with this nickname.
	if you drop your Nickname, it cannot be un-done, so use with care
NICKSERV_HELP_LOGOUT
	Syntax: /msg %S logout
	Logout allows you to "un-identify" to services. This means that
	you will not be reconised with serivices at all, regardless of your settings
	(eg, even if you match your access list, you will still not be reconised).
	This is usefull if you wish to stay on IRC, but are using a shared
	computer, and do not want other people to mess with your Registered
	Nickname.
NICKSERV_HELP_SET
	Syntax: /msg %S set <option> <value>
	Set allows you to customize certian settings with regards to your
	Nickname Registration. Some of these Settings include email or passwords,
	and Channel Greeting Messages, among others.
	The Available Options are:
	
	PASSWORD   Allows you to set a New Password for your account
	URL        Allows you to set a URL that will be displayed with the info command
	EMAIL      Allows you to change your registered email account
	ICQ        Allows you to set a ICQ number that is displayed in the info command
	INFO       Set a General Description of youself. Maybe Something Funny or witty
	AIM        Allows you to set a AIM handle that is displayed in the info command
	GREET      Sets your Channel Entry message
	KILL       Toggles your Kill setting.
	SECURE     Toggles your Secure Setting.
	PRIVATE    Toggles your Private Setting.
	VHOST      Change your Vhost (May not be enabled on all networks)

	/msg %S set will display your current Settings
	more information can be found on each option, by typing /msg $S help set <option> 
NICKSERV_HELP_SET_PASSWORD
	Syntax: /msg %S set password <newpass>
	Set Password allows you to change the password with your nickname registration.
	You should pick a password that is difficult to guess, to stop
	people from taking over your account by guessing your password.
NICKSERV_HELP_SET_URL
	Syntax: /msg %S set url <newurl>/NONE
	Set URL allows you to set or delete a URL that is displayed when people lookup your 
	nickname using the info command. 
	This can be used as a way for people to find out more information about yourself.
	if you type NONE instead of a URL, then your current URL will be deleted.
NICKSERV_HELP_SET_EMAIL
	Syntax: /msg %S set email <newemail>
	Set Email allows you to change the email account that is assocated
	with your services registration. The email must be a valid email account, so
	that if you loose or forget your password, a Service Operator can send you a
	your password via Email. Please keep this upto date.
NICKSERV_HELP_SET_ICQ
	Syntax: /msg %S set icq <account>/NONE
	Set ICQ allows you to set your ICQ number that will be displayed
	when people lookup your information via the info command. 
	this can be used as a way for people to find out more information
	about yourself.
	if you type NONE instead of a ICQ number, then your ICQ number will
	be deleted.
NICKSERV_HELP_SET_INFO
	Syntax: /msg %S set info <text>
	Set Info allows you to set a brief set of text that is displayed
	when someone looks up your nickname with the info command.
	Optionally, some networks also add this info line to your WHOIS
	information, but this may be disabled on some networks.
NICKSERV_HELP_SET_AIM
	Syntax: /msg %S set AIM <handle>/NONE
	Set AIM allows you to set your AOL Instant Messaganger handle that
	is displayed when someone looks up your nickname with the info command
	this can be usefull to display additional information about
	yourself.
	you can delete this information with the NONE command.
NICKSERV_HELP_SET_GREET
	Syntax: /msg %S set greet <text>/NONE
	Set Greet allows you to set a message that is displayed when you
	enter a channel that has this feature turned on and is registered.
	This text could be a description of yourself, or a witty comment.
	setting this to NONE will delete your Channel Greeting message.
NICKSERV_HELP_SET_KILL
	Syntax: /msg %S set kill
	Set Kill allows you to enforce some identification features of %S
	kill when turned on, means that if you do not match a host in your
	access list, or secure is turned off, you will have to identify to %S to
	gain access to your nickname, otherwise your nickname will be changed.
	this can be used to stop people using your nickname while you are
	not online, and it does NOT kill a user for using your nickname, it only
	changes their nickname if they do not supply the correct password within a
	time limit.
	
	see also: Set Secure
NICKSERV_HELP_SET_SECURE
	Syntax: /msg %S set secure
	Set Secure allows you to enable or disable the use of your access
	lists for identification. if you have secure enabled, and kill is enabled as
	well, then you must supply your password, otherwise your nickname will be
	changed. if you have secure disabled, and kill enabled, then you will gain
	limited access to your nickname, but some features or channels will still
	require you to identify to gain access to them.
	
	see also: Set Kill
NICKSERV_HELP_SET_PRIVATE
	Syntax: /msg %S set private
	Set Private allows you to disable the display of your email
	address, the last time you were online, and your last hostname you connected
	from in the info command.
	if you have set things like ICQ, INFO, AIM etc, they will still be
	visible.
NICKSERV_HELP_SET_VHOST
	Syntax: /msg %S set vhost <hostname>/NONE
	Set Vhost allows you to change the hostname you will appear to
	connect from when you identify to %S.
	Setting it to NONE will mean that your hostname will not change when
	you identify to %S
	Some Networks may have this feature disabled, or they may require a
	Operator to set it for you. Check with your Networks Help Channel.
NICKSERV_HELP_OPERCOMMENT
	Syntax: /msg %S opercomment <nick> <comment>
	OperComment allows Operators to set comments associated with
	registered nicknames, that are only visible by other Operators in the info
	command.
	This can be used as a way to comunicate information about a user
	between operators. When you set a Comment, your nickname is also saved with
	that comment.
	e.g.: /msg %S opercomment this user always asks for ops in the help channels, don't give it to him.
	would display that comment when another operator looks up his
	registration information with %S
NICKSERV_HELP_SUSPEND
	Syntax: /msg %S suspend <nick> <comment>/OFF
	Suspend allows a Operator to suspend the use of a registered nickname
	with services. This could be used by a operator to syspend a registration
	for a troublesome user while investigations are made. While a user is
	suspended, they can not access any services functions unless they register a
	new nickname.
NICKSERV_HELP_FORBID
	Syntax: /msg %S forbid list/add/del <option>
	Forbid controls what type of nicknames can be registered with
	services. It does not forbid the use of a nickname on a network (use SQLINE
	for that). 
	/msg %S forbid list
	this will list the current forbidden list.
	/msg %S forbid add <nickmask>
	this will add a nickname mask to the forbidden list. You can use Wildcards here (Such as * and ?).
	/msg %S forbid del <num>
	this will delete the corrosponding nickname from the forbidden list. 
	you can get the num, via the /msg %S forbid list command.
NICKSERV_HELP_GETPASS
	Syntax: /msg %S getpass <nick>
	GetPass allows a operator to retrive a registered nicknames
	password from the database.
	when this command is used, a Global message is sent to all online
	operators, and the action is logged, to help stop abuse of this command by
	operators.
NICKSERV_HELP_SETPASS
	Syntax: /msg %S setpass <nick> <newpass>
	SetPass allows a operator to reset a users password to a new
	password.
	When this command is used, a Global message is sent to all online
	operators and the action is logged, to help stop abuse of this command by
	operators.
NICKSERV_HELP_VHOST
	Syntax: /msg %S vhost <nick> <vhost>/NONE
	Vhost allows a operator to set or remove a users Virtual host that
	they will recieve when they identify to services.
	typing NONE will remove the users vhost.
NICKSERV_HELP_ACCESS
	Syntax: /msg %S access list/add/current/del <option>
	Access allows you to control your access lists with Services. 
	if you have kill turned on, and secure turned off, then if you
	connect and match a access list, you will not be required to identify to
	gain access to certian functions. 
	/msg %S access list
	will list your current access lists associated with your nickname.
	/msg %S access add <user@host>
	will add the user@host to your access lists (You can use the
	wildcards ? and * here as well>
	/msg %S access current
	will add your current hostmask to the access list if it does not
	already exist
	/msg %S access del <num>
	will delete the corrosponding entry out of your access list. You can
	figure out what num is via /msg %S access list
