#ifndef _CORE_H
#define _CORE_H

#include "hashmap.h"

#include <mysql/mysql.h>
#include <poll.h>
#include <stdbool.h>

struct bot;
struct user_;
struct clone;
struct module_;
struct trust;
struct link_;
struct guest;
struct chan;
struct wchan;
struct fakeuser;
struct server;

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
    DECLARE_HASHMAP(trusts, const char *, struct trust *);
    DECLARE_HASHMAP(links, const char *, struct link_ *);
    DECLARE_HASHMAP(guests, const char *, struct guest *);
    DECLARE_HASHMAP(chans, const char *, struct chan *);
    DECLARE_HASHMAP(wchans, const char *, struct wchan *);
    DECLARE_HASHMAP(bots, const char *, struct bot *);
    struct hashmap *commands;
#ifdef USE_FILTER
    struct hashmap *rules;
#endif  // USE_FILTER
    DECLARE_HASHMAP(fakeusers, const char *, struct fakeuser *);

    struct llist_head limits;
    struct llist_head timebans;

    struct llist_head servers;

    struct config config;

    int sock;
    int startuptime;
    bool verbose;
    bool vv;
    int raws;
    int eos;
    MYSQL mysql_handle;
};

#define core_get_users() (get_core()->users)
#define core_get_nicks() (get_core()->nicks)
#define core_get_clones() (get_core()->clones)
#define core_get_modules() (get_core()->modules)
#define core_get_trusts() (get_core()->trusts)
#define core_get_links() (get_core()->links)
#define core_get_guests() (get_core()->guests)
#define core_get_chans() (get_core()->chans)
#define core_get_wchans() (get_core()->wchans)
#define core_get_bots() (get_core()->bots)
#define core_get_fakeusers() (get_core()->fakeusers)

#define core_get_limits() (&get_core()->limits)
#define core_get_timebans() (&get_core()->timebans)
#define core_get_servers() (&get_core()->servers)

struct core *get_core(void);
void init_core(void);
void free_core(void);

#endif  // _CORE_H
