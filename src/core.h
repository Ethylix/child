#ifndef _CORE_H
#define _CORE_H

#include "hashmap.h"
#include "partyline.h"

#include <mysql/mysql.h>
#include <poll.h>
#include <stdbool.h>

struct bot;
struct user_;
struct clone;
struct module_;

struct config {
    char *nick;
    char *name;
    char *ident;
    char *host;

    char *server;
    int port;
    char *bindip;

    char *sid;
    char *linkpass;
    int maxclones;

    int nick_expire;
    int chan_expire;

    int chanperuser;

    int level_oper;
    int level_admin;
    int level_root;
    int level_owner;

    char *mysql_host;
    char *mysql_db;
    char *mysql_login;
    char *mysql_passwd;

    char *logfile;

    int limittime;

    int savedb_interval;

#ifdef USE_GNUTLS
    int ssl;
#endif  // USE_GNUTLS

    char *guest_prefix;

    int listen_port;
    char *pl_logfile;

    int enable_exec;
    int anonymous_global;

    char *sendmail;
    char *sendfrom;

    int maxmsgtime;
    int maxmsgnb;

    int ignoretime;

    int maxloginatt;

    int chlev_sadmin;
    int chlev_admin;
    int chlev_op;
    int chlev_halfop;
    int chlev_voice;
    int chlev_nostatus;
    int chlev_akick;
    int chlev_akb;
    int chlev_invite;

#ifdef USE_FILTER
    int filter;
#endif  // USE_FILTER

    char *usercloak;

    int emailreg;

    int retry_attempts;
    int connected;
    int nextretry;
};

struct core {
    DECLARE_HASHMAP(users, const char *, struct user_ *);
    DECLARE_HASHMAP(nicks, const char *, struct nick *);
    DECLARE_HASHMAP(clones, const char *, struct clone *);
    DECLARE_HASHMAP(modules, const char *, struct module_ *);
    struct hashmap *hooks;
    struct hashmap *trusts;
    struct hashmap *links;
    struct hashmap *eclients;
    struct hashmap *guests;
    struct hashmap *chans;
    struct hashmap *wchans;
    struct hashmap *cflags;
    struct hashmap *members;
    struct hashmap *limits;
    DECLARE_HASHMAP(bots, const char *, struct bot *);
    struct hashmap *chanbots;
    struct hashmap *commands;
#ifdef USE_FILTER
    struct hashmap *rules;
#endif  // USE_FILTER
    struct hashmap *timebans;
    struct hashmap *fakes;

    struct config config;

    int sock;
    int esock;
    int startuptime;
    bool verbose;
    bool vv;
    int raws;
    int eos;
    int emerg;
    int emerg_req;
    MYSQL mysql_handle;
    struct pollfd ufds[ECL_MAXSOCK];
};

struct core *get_core(void);
void init_core(void);
void free_core(void);

#endif  // _CORE_H
