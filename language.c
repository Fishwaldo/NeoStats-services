/* Multi-language support.
 *
 * Epona (c) 2000-2001 PegSoft
 * Contact us at epona@pegsoft.net
 *
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 *
 * Based on the original code of Services by Andy Church.
 *
 */

#include <stdio.h>
#include "services.h"
#include "language.h"
#include "stats.h"

/*************************************************************************/

/* The list of lists of messages. */
char **langtexts[NUM_LANGS];

/* The list of names of languages. */
char *langnames[NUM_LANGS];

/* Indexes of available languages: */
int langlist[NUM_LANGS];

/* Order in which languages should be displayed: (alphabetical) */
static int langorder[NUM_LANGS] = {
    LANG_EN_US,		/* English (US) */
};

/*************************************************************************/

/* Load a language file. */

static int read_int32(int *ptr, FILE *f)
{
    int a = fgetc(f);
    int b = fgetc(f);
    int c = fgetc(f);
    int d = fgetc(f);
    if (a == EOF || b == EOF || c == EOF || d == EOF)
	return -1;
    *ptr = a<<24 | b<<16 | c<<8 | d;
    return 0;
}

static void load_lang(int index, const char *filename)
{
    char buf[256];
    FILE *f;
    int num, i;

#ifdef DEBUG
	log("debug: Loading language %d from file `dl/services/lang/%s'",
		index, filename);
#endif
    snprintf(buf, sizeof(buf), "dl/services/lang/%s", filename);
    if (!(f = fopen(buf, "r"))) {
	log("Failed to load language %d (%s)", index, filename);
	return;
    } else if (read_int32(&num, f) < 0) {
	log("Failed to read number of strings for language %d (%s)",
		index, filename);
	return;
    } else if (num != NUM_STRINGS) {
	log("Warning: Bad number of strings (%d, wanted %d) "
	    "for language %d (%s)", num, NUM_STRINGS, index, filename);
    }
    langtexts[index] = smalloc(sizeof(char *) * NUM_STRINGS);
    if (num > NUM_STRINGS)
	num = NUM_STRINGS;
    for (i = 0; i < num; i++) {
	int pos, len;
	fseek(f, i*8+4, SEEK_SET);
	if (read_int32(&pos, f) < 0 || read_int32(&len, f) < 0) {
	    log("Failed to read entry %d in language %d (%s) TOC",
			i, index, filename);
	    while (--i >= 0) {
		if (langtexts[index][i])
		    free(langtexts[index][i]);
	    }
	    free(langtexts[index]);
	    langtexts[index] = NULL;
	    return;
	}
	if (len == 0) {
	    langtexts[index][i] = NULL;
	} else if (len >= 65536) {
	    log("Entry %d in language %d (%s) is too long (over 64k)--"
		"corrupt TOC?", i, index, filename);
	    while (--i >= 0) {
		if (langtexts[index][i])
		    free(langtexts[index][i]);
	    }
	    free(langtexts[index]);
	    langtexts[index] = NULL;
	    return;
	} else if (len < 0) {
	    log("Entry %d in language %d (%s) has negative length--"
		"corrupt TOC?", i, index, filename);
	    while (--i >= 0) {
		if (langtexts[index][i])
		    free(langtexts[index][i]);
	    }
	    free(langtexts[index]);
	    langtexts[index] = NULL;
	    return;
	} else {
	    langtexts[index][i] = smalloc(len+1);
	    fseek(f, pos, SEEK_SET);
	    if (fread(langtexts[index][i], 1, len, f) != len) {
		log("Failed to read string %d in language %d (%s)",
			i, index, filename);
		while (--i >= 0) {
		    if (langtexts[index][i])
			free(langtexts[index][i]);
		}
		free(langtexts[index]);
		langtexts[index] = NULL;
		return;
	    }
	    langtexts[index][i][len] = 0;
	}
    }
    fclose(f);
}

/*************************************************************************/

/* Initialize list of lists. */

void lang_init()
{
    int i, j, n = 0;

    load_lang(LANG_EN_US, 	"en_us");

    for (i = 0; i < NUM_LANGS; i++) {
	if (langtexts[langorder[i]] != NULL) {
	    langnames[langorder[i]] = langtexts[langorder[i]][LANG_NAME];
	    langlist[n++] = langorder[i];
	    for (j = 0; j < NUM_STRINGS; j++) {
		if (!langtexts[langorder[i]][j]) {
		    langtexts[langorder[i]][j] =
				langtexts[DEF_LANGUAGE][j];
		}
		if (!langtexts[langorder[i]][j]) {
		    langtexts[langorder[i]][j] =
				langtexts[LANG_EN_US][j];
		}
	    }
	}
    }
    while (n < NUM_LANGS)
		langlist[n++] = -1;
		
	/* Not what I intended to do, but these services are so archaïc
	 * that it's difficult to do more. */
	if ((NSDefLanguage = langlist[NSDefLanguage]) < 0)
		NSDefLanguage = DEF_LANGUAGE;

    if (!langtexts[DEF_LANGUAGE]) {
		log("Unable to load default language");
		exit(-1);
    }
    for (i = 0; i < NUM_LANGS; i++) {
		if (!langtexts[i])
	    	langtexts[i] = langtexts[DEF_LANGUAGE];
    }
}

/*************************************************************************/
/*************************************************************************/

/* Format a string in a strftime()-like way, but heed the user's language
 * setting for month and day names.  The string stored in the buffer will
 * always be null-terminated, even if the actual string was longer than the
 * buffer size.
 * Assumption: No month or day name has a length (including trailing null)
 * greater than BUFSIZE.
 */

int strftime_lang(char *buf, int size, NS_User *u, int format, struct tm *tm)
{
    int language = u && u->language ? u->language : NSDefLanguage;
    char tmpbuf[BUFSIZE], buf2[BUFSIZE];
    char *s;
    int i, ret;

    strncpy(tmpbuf, langtexts[language][format], sizeof(tmpbuf));
    if ((s = langtexts[language][STRFTIME_DAYS_SHORT]) != NULL) {
	for (i = 0; i < tm->tm_wday; i++)
	    s += strcspn(s, "\n")+1;
	i = strcspn(s, "\n");
	strncpy(buf2, s, i);
	buf2[i] = 0;
	strnrepl(tmpbuf, sizeof(tmpbuf), "%a", buf2);
    }
    if ((s = langtexts[language][STRFTIME_DAYS_LONG]) != NULL) {
	for (i = 0; i < tm->tm_wday; i++)
	    s += strcspn(s, "\n")+1;
	i = strcspn(s, "\n");
	strncpy(buf2, s, i);
	buf2[i] = 0;
	strnrepl(tmpbuf, sizeof(tmpbuf), "%A", buf2);
    }
    if ((s = langtexts[language][STRFTIME_MONTHS_SHORT]) != NULL) {
	for (i = 0; i < tm->tm_mon; i++)
	    s += strcspn(s, "\n")+1;
	i = strcspn(s, "\n");
	strncpy(buf2, s, i);
	buf2[i] = 0;
	strnrepl(tmpbuf, sizeof(tmpbuf), "%b", buf2);
    }
    if ((s = langtexts[language][STRFTIME_MONTHS_LONG]) != NULL) {
	for (i = 0; i < tm->tm_mon; i++)
	    s += strcspn(s, "\n")+1;
	i = strcspn(s, "\n");
	strncpy(buf2, s, i);
	buf2[i] = 0;
	strnrepl(tmpbuf, sizeof(tmpbuf), "%B", buf2);
    }
    ret = strftime(buf, size, tmpbuf, tm);
    if (ret == size)
	buf[size-1] = 0;
    return ret;
}

void notice_lang(const char *source, char *dest, int message, ...)
{
    va_list args;
    char buf[4096];	/* because messages can be really big */
    char *s, *t, nick[55];
    const char *fmt;
    int lang = 0;
    NS_User *tmp;
    User *tmp1;
    
    
    if (!dest)
	return;
    tmp = findregnick(dest);
    if (!tmp) {
    	tmp1 = finduser(dest);
	strcpy(nick, tmp1->nick);
    	lang = 0;
    } else {
    	lang = tmp->language;
	strcpy(nick, tmp->nick);
    }
    
    va_start(args, message);
    fmt = getstring(lang, message);
    if (!fmt)
	return;
    memset(buf, 0, 4096);
    vsnprintf(buf, sizeof(buf), fmt, args);
    s = buf;
    while (*s) {
	t = s;
	s += strcspn(s, "\n");
	if (*s)
	    *s++ = 0;
	sts(":%s %s %s :%s", source, "NOTICE", nick, *t ? t : " ");
    }
}


/* Like notice_lang(), but replace %S by the source.  This is an ugly hack
 * to simplify letting help messages display the name of the pseudoclient
 * that's sending them.
 */
void notice_help(const char *source, char *dest, int message, ...)
{
    va_list args;
    char buf[4096], buf2[4096], outbuf[BUFSIZE];
    char *s, *t, nick[55];
    const char *fmt;
    NS_User *tmp;
    User *tmp1;
    int lang = 0;

    if (!dest)
	return;
    log("get %s", dest);

    tmp = findregnick(dest);
    if (!tmp) { 
    	tmp1 = finduser(dest);
	strcpy(nick, tmp1->nick);
    } else {
    	lang = tmp->language;
	strcpy(nick, tmp->nick);
    }	
    log("get %s", nick);
    va_start(args, message);
    fmt = getstring(lang, message);
    if (!fmt) {
	return;
    }
    /* Some sprintf()'s eat %S or turn it into just S, so change all %S's
     * into \1\1... we assume this doesn't occur anywhere else in the
     * string. */
    strncpy(buf2, fmt, sizeof(buf2));
    strnrepl(buf2, sizeof(buf2), "%S", "\1\1");
    vsnprintf(buf, sizeof(buf), buf2, args);
    s = buf;
    while (*s) {
	t = s;
	s += strcspn(s, "\n");
	if (*s)
	    *s++ = 0;
	strncpy(outbuf, t, sizeof(outbuf));
	strnrepl(outbuf, sizeof(outbuf), "\1\1", source);
	sts(":%s %s %s :%s", source, "NOTICE", nick, *outbuf ? outbuf : " ");
    }
}
