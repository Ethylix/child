/*
Child, Internet Relay Chat Services
Copyright (C) 2005-2020  David Lebrun (target0@geeknode.org)

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

#include "server.h"
#include "user.h"

#define HASHMAX 65000
#define CONF_LOAD   0
#define CONF_REHASH 1

#define DEFAULT_CONFFILE "child.conf"

#define operlog(x, ...) mylog(me.logfile, x, ##__VA_ARGS__)

struct {
    char nick[32];
    char name[32];
    char ident[32];
    char host[32]; 
    char server[40];
    int port;
    char sid[SIDLEN + 1];
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
    char guest_prefix[32];
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
    char usercloak[HOSTLEN+1];
    int emailreg;
    int retry_attempts, connected, nextretry;

    char remote_server[SERVERNAMELEN+1];
    char remote_sid[SIDLEN+1];
    char uid[UIDLEN+1];
} me;

extern char *strcasestr (const char *, const char *);

void child_die (int);
void child_restart (int);
void child_clean (void) __attribute__((noreturn));
void init_srandom (void);
void mylog (char *, char *, ...);

/* loadconf.c */
void loadconf (int);

/* md5.c */
char *md5_hash (char *);

/* parseline.c */
int ParseLine (void);

#endif
