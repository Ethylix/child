/*
Child, Internet Relay Chat Services
Copyright (C) 2005-2009  David Lebrun (target0@geeknode.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _CHILD_H
#define _CHILD_H

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <mysql/mysql.h>
#include <openssl/evp.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <err.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <regex.h>
#include <sys/wait.h>
#include <setjmp.h>
#include <sys/resource.h>
#ifdef USE_GNUTLS
#include <gnutls/gnutls.h>
#endif

#ifdef __FreeBSD__
#include <kvm.h>
#include <paths.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#endif

#include <botserv.h>
#include <channel.h>
#include <commands.h>
#ifdef USE_FILTER
#include <filter.h>
#endif
#include <mem.h>
#include <modules.h>
#include <net.h>
#include <partyline.h>
#include <trust.h>
#include <user.h>

#ifndef __unused
#define __unused __attribute__((unused))
#endif

#define Strstr(x,y) strcasestr(x,y)
#define ircsprintf(a,b,c,d) va_start(d,c); \
                            vsnprintf(a,b,c,d); \
                            va_end(d)

#define SetOption(x, y) ((x)->options |= (y))
#define ClearOption(x, y) ((x)->options &= ~(y))
#define HasOption(x, y) ((x)->options & (y))

#define HASHMAX 65000
#define CONF_LOAD   0
#define CONF_REHASH 1

#define DEFAULT_CONFFILE "child.conf"

#define operlog(x, ...) mylog(me.logfile, x, ##__VA_ARGS__)
#define pllog(x, ...) mylog(me.pl_logfile, x, ##__VA_ARGS__)

#define match_regex(x, y) __match_regex(x, y, REG_EXTENDED|REG_NOSUB)

struct {
    char nick[32];
    char name[32];
    char ident[32];
    char host[32]; 
    char server[40];
    int port;
    char sid[8];
    char linkpass[50];
    int maxclones;
    char bindip[40];
    int nick_expire;
    int chan_expire;
    int chanperuser;
    int level_oper;
    int level_admin;
    int level_root;
    int level_owner;
    char mysql_host[40];
    char mysql_db[32];
    char mysql_login[32];
    char mysql_passwd[32];
    char logfile[32];
    int limittime;
    int savedb_interval;
#ifdef USE_GNUTLS
    int ssl;
#endif
    char guest_prefix[32];
    int listen_port;
    char pl_logfile[32];
    int enable_exec;
    int anonymous_global;
    char sendmail[128];
    char sendfrom[128];
    int maxmsgtime,maxmsgnb;
    int ignoretime;
    int maxloginatt;
    int chlev_sadmin,chlev_admin,chlev_op;
    int chlev_halfop,chlev_voice;
    int chlev_nostatus,chlev_akick,chlev_akb;
    int chlev_invite;
#ifdef USE_FILTER
    int filter;
#endif
    char usercloak[HOSTLEN+1];
    int emailreg;
    int retry_attempts, connected, nextretry;
} me;

extern char *strcasestr (const char *, const char *);

void child_die (int);
void child_restart (int);
void child_clean (void) __attribute__((noreturn));
void init_srandom (void);
void mylog (char *, char *, ...);

/* db.c */
void loaduserdb (void);
void loadchandb (void);
void loadtrustdb (void);
void loadlinkdb (void);
void loadbotservdb (void);
void saveuserdb (void);
void savechandb (void);
void savetrustdb (void);
void savelinkdb (void);
void savebotservdb (void);
void savealldb (void);
void loadalldb (void);

/* hash.c */
int hash (char *);

/* loadconf.c */
void loadconf (int);

/* md5.c */
char *md5_hash (char *);

/* mysql.c */
int connect_to_db (void);
int reconnect_to_db (void);

/* parseline.c */
int ParseLine (void);

/* str.c */
int IsCharInString (char, char *);
char *SeperateWord (char *);
char *StripBlanks (char *);
int Strcmp (const char *, const char *);
char *strtosql(char *, char *, int);
int __match_regex (char *, char *, int);
char *parse_range (char *, char *);
char *gen_rand_string (char *, char *, int);
void ToLower(char *, char *, unsigned int);

#endif
