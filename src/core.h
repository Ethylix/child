#ifndef _CORE_H
#define _CORE_H

#include "hashmap.h"
#include "server.h"
#include "user.h"

#include <mysql/mysql.h>
#include <poll.h>
#include <stdbool.h>

struct bot;
struct user;
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
    char nick[32];
    char name[32];
    char ident[32];
    char host[32];

    char server[40];
    int port;
    char bindip[40];

    char sid[SIDLEN+1];
    char linkpass[50];
    int maxclones;

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

    char usercloak[HOSTLEN+1];

    int emailreg;
};

struct core {
    DECLARE_HASHMAP(users, const char *, struct user *);
    DECLARE_HASHMAP(nicks, const char *, struct nick *);
    DECLARE_HASHMAP(clones, const char *, struct clone *);
    DECLARE_HASHMAP(modules, const char *, struct module_ *);
    DECLARE_HASHMAP(trusts, const char *, struct trust *);
    DECLARE_HASHMAP(links, const char *, struct link_ *);
    DECLARE_HASHMAP(guests, const char *, struct guest *);
    DECLARE_HASHMAP(chans, const char *, struct chan *);
    DECLARE_HASHMAP(wchans, const char *, struct wchan *);
    DECLARE_HASHMAP(bots, const char *, struct bot *);
    DECLARE_HASHMAP(fakeusers, const char *, struct fakeuser *);

    struct llist_head limits;
    struct llist_head timebans;

    struct llist_head servers;

    // TODO(target0): replace with a better data structure
    struct llist_head commands;

    // Startup config
    struct config config;

    // Runtime parameters
    int startuptime;
    bool verbose;
    bool vv;
    bool raws;
    bool eos;
    MYSQL mysql_handle;
    int connected;

    struct timeval next_connect_time;
    struct timeval next_savedb_time;
    struct timeval next_expired_check_time;
    int connect_retries;

    char remote_server[SERVERNAMELEN+1];
    char remote_sid[SIDLEN+1];
    char uid[UIDLEN+1];

    struct net net;

    bool stop;
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
#define core_get_commands() (&get_core()->commands)

#define core_get_config() (&get_core()->config)

#define core_get_net() (&get_core()->net)

struct core *get_core(void);
void init_core(void);
void free_core(void);

#endif  // _CORE_H
