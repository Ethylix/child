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


#include "db.h"

#include "botserv.h"
#include "channel.h"
#include "child.h"
#include "core.h"
#include "core_api.h"
#include "hashmap.h"
#include "logging.h"
#include "modules.h"
#include "string_utils.h"
#include "trust.h"
#include "user.h"

#include <mysql/mysql.h>
#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int reconnect_to_db(void)
{
    return mysql_ping(&get_core()->mysql_handle) == 0;
}

static int load_one_user(const char *name,
                         const int level,
                         const int lastseen,
                         const char *vhost,
                         const char *md5_pass,
                         const char *pwhash,
                         const int options,
                         const int timeout,
                         const char *email,
                         const int regtime)
{
        User *uptr;

        if (find_user(name)) {
            fprintf(stderr, "Cannot load user %s from db: user already exists.\n", name);
            return -1;
        }

        if (get_core()->vv) printf("Loading user %s\n", name);

        uptr = AddUser(name, level);
        if (!uptr) {
            fprintf(stderr, "Cannot load user %s from db: AddUser() returned NULL.\n", name);
            return -1;
        }

        uptr->lastseen = lastseen;
        strncpy(uptr->vhost, vhost, HOSTLEN);
        strncpy(uptr->md5_pass, md5_pass, MD5_LEN);
        strncpy(uptr->pwhash, pwhash, crypto_pwhash_STRBYTES);
        uptr->options = options;
        uptr->timeout = timeout ?: TIMEOUT_DFLT;
        strncpy(uptr->email, email, EMAILLEN);
        uptr->regtime = regtime;

        if (get_core()->vv) printf("Loaded user %s at %p\n", name, uptr);

        return 0;
}

static int load_user_db(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int err = 0;

    mysql_query(&get_core()->mysql_handle,"SELECT username, authlevel, seen, vhost, md5_pass, pwhash, options, timeout, email, regtime FROM child_users");

    if ((result = mysql_use_result(&get_core()->mysql_handle)) == NULL) {
        fprintf(stderr, "CRITICAL: Cannot load user table: mysql_use_result returned NULL\n");
        return -1;
    }

    while ((row = mysql_fetch_row(result))) {
        err = load_one_user(/*name=*/row[0],
                            /*level=*/strtol(row[1], NULL, 10),
                            /*lastseem=*/strtol(row[2], NULL, 10),
                            /*vhost=*/row[3],
                            /*md5_pass=*/row[4],
                            /*pwhash=*/row[5],
                            /*options=*/strtol(row[6], NULL, 10),
                            /*timeout=*/strtol(row[7], NULL, 10),
                            /*email=*/row[8],
                            /*regtime=*/strtol(row[9], NULL, 10));
        if (err) {
            fprintf(stderr, "Failed to load user %s from db, aborting load.\n", row[0]);
            break;
        }
    }

    mysql_free_result(result);
    return err;
}

static int load_one_chan(const char *name,
                         const char *owner,
                         const char *entrymsg,
                         const int options,
                         const char *mlock,
                         const int autolimit,
                         const int lastseen,
                         const int regtime,
                         const char *topic)
{
    Chan *chptr;

    if (find_channel(name)) {
        fprintf(stderr, "Cannot load channel %s from db: channel already exists.\n", name);
        return -1;
    }

    if (!find_user(owner)) {
        fprintf(stderr, "Cannot load channel %s from db: owner %s does not exist.\n", name, owner);
        return -1;
    }

    if (get_core()->vv)
        printf("Loading channel %s\n", name);

    chptr = CreateChannel(name, owner, lastseen);
    if (!chptr) {
        fprintf(stderr, "Cannot load channel %s from db: CreateChannel() returned NULL.\n", name);
        return -1;
    }

    strncpy(chptr->entrymsg, entrymsg, 250);
    chptr->options = options;
    strncpy(chptr->mlock, mlock, 50);
    chptr->autolimit = autolimit;
    chptr->regtime = regtime;
    strncpy(chptr->topic, topic, TOPICLEN);

    if (get_core()->vv)
        printf("Loaded channel %s at %p\n", name, chptr);

    return 0;
}

static int load_chan_db(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int err = 0;

    mysql_query(&get_core()->mysql_handle,"SELECT chname, founder, entrymsg, options, mlock, autolimit, lastseen, regtime, topic FROM child_chans");

    if ((result = mysql_use_result(&get_core()->mysql_handle)) == NULL) {
        fprintf(stderr, "CRITICAL: Cannot load chan table: mysql_use_result returned NULL\n");
        return -1;
    }

    while ((row = mysql_fetch_row(result))) {
        err = load_one_chan(/*name=*/row[0],
                              /*owner=*/row[1],
                              /*entrymsg=*/row[2],
                              /*options=*/strtol(row[3], NULL, 10),
                              /*mlock=*/row[4],
                              /*autolimit=*/strtol(row[5], NULL, 10),
                              /*lastseen=*/strtol(row[6], NULL, 10),
                              /*regtime=*/strtol(row[7], NULL, 10),
                              /*topic=*/row[8]);

        if (err) {
            fprintf(stderr, "Failed to load channel %s from db, aborting load.\n", row[0]);
            break;
        }
    }

    mysql_free_result(result);
    return err;
}

static int load_one_cflag(const char *chname,
                          const char *username,
                          const int level,
                          const int automode,
                          const int suspended,
                          const int uflags)
{
    Chan *chptr;
    User *uptr;
    Cflag *cflag;

    /* Owner is added through CreateChannel() */
    if (level == CHLEV_OWNER)
        return 0;

    if (level > CHLEV_OWNER) {
        fprintf(stderr, "Cannot load cflag for channel %s and user %s from db: invalid level %d.\n", chname, username, level);
        return -1;
    }

    chptr = find_channel(chname);
    if (!chptr) {
        fprintf(stderr, "Cannot load cflag for channel %s from db: channel does not exist.\n", chname);
        return -1;
    }

    uptr = find_user(username);
    if (!uptr) {
        fprintf(stderr, "cannot load cflag for channel %s from db: user %s does not exist.\n", chname, username);
        return -1;
    }

    if (find_cflag(chptr, uptr)) {
        fprintf(stderr, "Cannot load cflag for channel %s and user %s from db: cflag already exists.\n", chname, username);
        return -1;
    }

    if (get_core()->vv)
        printf("Loading cflag (%s, %s)\n", chname, username);

    cflag = AddUserToChannel(uptr, chptr, level, uflags);
    if (!cflag) {
        fprintf(stderr, "Cannot load cflag (%s, %s) from db: AddUserToChannel() returned NULL.\n", chname, username);
        return -1;
    }

    cflag->automode = automode;
    cflag->suspended = suspended;

    if (get_core()->vv)
        printf("Loaded cflag (%s, %s) at %p\n", chname, username, cflag);

    return 0;
}

static int load_cflag_db(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int err = 0;

    mysql_query(&get_core()->mysql_handle,"SELECT chname, username, level, automode, suspended, uflags from child_chan_access");

    if ((result = mysql_use_result(&get_core()->mysql_handle)) == NULL) {
        fprintf(stderr, "CRITICAL: Cannot load chan access table: mysql_use_result returned NULL!\n");
        return -1;
    }

    while ((row = mysql_fetch_row(result))) {
        err = load_one_cflag(/*chname=*/row[0],
                             /*username=*/row[1],
                             /*level=*/strtol(row[2], NULL, 10),
                             /*automode=*/strtol(row[3], NULL, 10),
                             /*suspended=*/strtol(row[4], NULL, 10),
                             /*uflags=*/strtol(row[5], NULL, 10));

        if (err) {
            fprintf(stderr, "Failed to load cflag (%s, %s) from db, aborting load.\n", row[0], row[1]);
            break;
        }
    }

    mysql_free_result(result);
    return err;
}

static int load_one_trust(const char *hostname,
                          const int clones)
{
    Trust *trust;

    if (find_trust_strict(hostname)) {
        fprintf(stderr, "Cannot load trust for %s from db: trust already exists.\n", hostname);
        return -1;
    }

    if (get_core()->vv)
        printf("Loading trust %s\n", hostname);

    trust = AddTrust(hostname, clones);
    if (!trust) {
        fprintf(stderr, "Cannot load trust for %s from db: AddTrust() returned NULL.\n", hostname);
        return -1;
    }

    if (get_core()->vv)
        printf("Loaded trust %s at %p\n", hostname, trust);

    return 0;
}

static int load_trust_db(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int err = 0;

    mysql_query(&get_core()->mysql_handle,"SELECT hostname, clones FROM child_trusts");
    if ((result = mysql_use_result(&get_core()->mysql_handle)) == NULL) {
        fprintf(stderr, "CRITICAL: Cannot load trust table: mysql_use_result returned NULL\n");
        return -1;
    }

    while ((row = mysql_fetch_row(result))) {
        err = load_one_trust(/*hostname=*/row[0],
                             /*clones=*/strtol(row[1], NULL, 10));

        if (err) {
            fprintf(stderr, "Failed to load trust for %s from db, aborting load.\n", row[0]);
            break;
        }
    }

    mysql_free_result(result);
    return err;
}

static int load_one_link(const char *master,
                         const char *slave)
{
    Link *link;

    if (!find_user(master)) {
        fprintf(stderr, "Cannot load link (%s, %s) from db: master does not exist.\n", master, slave);
        return -1;
    }

    if (!find_user(slave)) {
        fprintf(stderr, "Cannot load link (%s, %s) from db: slave does not exist.\n", master, slave);
        return -1;
    }

    if (find_link(slave)) {
        fprintf(stderr, "Cannot load link (%s, %s) from db: slave is already linked.\n", master, slave);
        return -1;
    }

    if (get_core()->vv)
        printf("Loading link (%s, %s)\n", master, slave);

    link = AddLink(master, slave);
    if (!link) {
        fprintf(stderr, "Cannot load link (%s, %s) from db: AddLink() returned NULL.\n", master, slave);
        return -1;
    }

    if (get_core()->vv)
        printf("Loaded link (%s, %s) at %p\n", master, slave, link);

    return 0;
}

static int load_link_db(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int err = 0;

    mysql_query(&get_core()->mysql_handle,"SELECT master, slave FROM child_links");

    if ((result = mysql_use_result(&get_core()->mysql_handle)) == NULL) {
        fprintf(stderr, "CRITICAL: Cannot load link table: mysql_use_result returned NULL\n");
        return -1;
    }

    while ((row = mysql_fetch_row(result))) {
        err = load_one_link(/*master=*/row[0],
                            /*slave=*/row[1]);

        if (err) {
            fprintf(stderr, "Failed to load link (%s, %s) from db, aborting load.\n", row[0], row[1]);
            break;
        }
    }

    mysql_free_result(result);
    return err;
}

static int load_one_botserv_bot(const char *name,
                                const char *ident,
                                const char *host)
{
    Bot *bot;

    if (find_bot(name)) {
        fprintf(stderr, "Cannot load bot %s from db: bot already exists.\n", name);
        return -1;
    }

    if (get_core()->vv)
        printf("Loading bot %s\n", name);

    bot = add_bot(name, ident, host);
    if (!bot) {
        fprintf(stderr, "Cannot load bot %s from db: add_bot() returned NULL.\n", name);
        return -1;
    }

    if (get_core()->vv)
        printf("Loaded bot %s at %p\n", name, bot);

    return 0;
}

static int load_botserv_bots_db(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int err = 0;

    mysql_query(&get_core()->mysql_handle,"SELECT name, ident, host FROM child_botserv_bots");

    if ((result = mysql_use_result(&get_core()->mysql_handle)) == NULL) {
        fprintf(stderr, "CRITICAL: Cannot load botserv bots table: mysql_use_result returned NULL\n");
        return -1;
    }

    while ((row = mysql_fetch_row(result))) {
        err = load_one_botserv_bot(/*name=*/row[0],
                                   /*ident=*/row[1],
                                   /*host=*/row[2]);

        if (err) {
            fprintf(stderr, "Failed to load bot %s from db, aborting load.\n", row[0]);
            break;
        }
    }

    mysql_free_result(result);
    return err;
}

static int load_one_botserv_chan(const char *chan,
                                 const char *botname)
{
    Chan *chptr;
    Bot *bot;

    bot = find_bot(botname);
    if (!bot) {
        fprintf(stderr, "Cannot load chanbot (%s, %s) from db: bot does not exist.\n", chan, botname);
        return -1;
    }

    chptr = find_channel(chan);
    if (!chptr) {
        fprintf(stderr, "Cannot load chanbot (%s, %s) from db: channel does not exist.\n", chan, botname);
        return -1;
    }

    if (chptr->chanbot) {
        fprintf(stderr, "Cannot load chanbot (%s, %s) from db: channel already has a bot assigned (%s).\n", chan, botname, chptr->chanbot->nick);
        return -1;
    }

    chptr->chanbot = bot;

    if (get_core()->vv)
        printf("Loaded chanbot (%s, %s)\n", chan, botname);

    return 0;
}

static int load_botserv_chans_db(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int err = 0;

    mysql_query(&get_core()->mysql_handle,"SELECT chan, bot FROM child_botserv_chans");

    if ((result = mysql_use_result(&get_core()->mysql_handle)) == NULL) {
        fprintf(stderr, "CRITICAL: Cannot load botserv chans table: mysql_use_result returned NULL\n");
        return -1;
    }

    while ((row = mysql_fetch_row(result))) {
        err = load_one_botserv_chan(/*chan=*/row[0],
                                    /*bot=*/row[1]);

        if (err) {
            fprintf(stderr, "Failed to load chanbot (%s, %s) from db: aborting load.\n", row[0], row[1]);
            break;
        }
    }

    mysql_free_result(result);
    return err;
}

void saveuserdb()
{
    char tmp[1024];
    char buf[512];
    User *uptr;
    struct hashmap_entry *entry;

    bzero(buf,512);

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db: %s\n", mysql_error(&get_core()->mysql_handle));
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&get_core()->mysql_handle,"DELETE FROM child_users");

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_users(), entry, uptr) {
        snprintf(tmp,1024,"INSERT INTO child_users ("
                            "nick,"
                            "level,"
                            "lastseen,"
                            "vhost,"
                            "md5_pass,"
                            "pwhash,"
                            "options,"
                            "timeout,"
                            "email,"
                            "regtime"
                            ")"
                            "VALUES ('%s',%d,%d,'%s','%s','%s',%ld,%d,'%s',%d)",
                            strtosql(buf,uptr->nick,512),
                            uptr->level,
                            uptr->lastseen,
                            uptr->vhost,
                            uptr->md5_pass,
                            uptr->pwhash,
                            uptr->options,
                            uptr->timeout,
                            uptr->email,
                            uptr->regtime);
        mysql_query(&get_core()->mysql_handle,tmp);
    }
}

void savechandb()
{
    char tmp[1024];
    Chan *chptr;
    Cflag *cflag;
    char chname[CHANLEN+1];
    char nick[NICKLEN+1];
    char owner[NICKLEN+1];
    char entrymsg[250+1];
    char topic[TOPICLEN+1];
    struct hashmap_entry *entry;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db: %s\n", mysql_error(&get_core()->mysql_handle));
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&get_core()->mysql_handle,"DELETE FROM child_chans");
    mysql_query(&get_core()->mysql_handle,"DELETE FROM child_chan_access");

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_chans(), entry, chptr) {
        bzero(chname,CHANLEN);
        bzero(nick,NICKLEN);
        bzero(owner,NICKLEN);
        bzero(entrymsg,250);
        bzero(topic, TOPICLEN);
        strtosql(chname,chptr->channelname, CHANLEN);
        strtosql(topic, chptr->topic, TOPICLEN);

        snprintf(tmp,1024,"INSERT INTO `child_chans` VALUES ('%s','%s','%s',%ld,'%s',%d,%d,%d,'%s')",chname,strtosql(owner,chptr->owner,NICKLEN),strtosql(entrymsg,chptr->entrymsg,250),chptr->options,chptr->mlock,chptr->autolimit,chptr->lastseen, chptr->regtime, topic);
        mysql_query(&get_core()->mysql_handle,tmp);

        LLIST_FOREACH_ENTRY(&chptr->cflags, cflag, chan_head) {
            if (cflag->flags >= CHLEV_OWNER)
                continue;

            bzero(nick, NICKLEN);
            strtosql(nick, cflag->user->nick, NICKLEN);

            snprintf(tmp,1024,"INSERT INTO `child_chan_access` VALUES ('%s','%s',%d,%d,%d,%d)",chname,nick,cflag->flags,cflag->automode,cflag->suspended,cflag->uflags);
            mysql_query(&get_core()->mysql_handle,tmp);
        }
    }
}

void savetrustdb()
{
    char tmp[1024];
    Trust *trust;
    struct hashmap_entry *entry;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db: %s\n", mysql_error(&get_core()->mysql_handle));
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&get_core()->mysql_handle,"DELETE FROM child_trusts");

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_trusts(), entry, trust) {
        snprintf(tmp,1024,"INSERT INTO child_trusts VALUES ('%s',%d)",trust->host,trust->limit);
        mysql_query(&get_core()->mysql_handle,tmp);
    }
}

void savelinkdb()
{
    char tmp[1024];
    Link *link;
    char master[NICKLEN];
    char slave[NICKLEN];
    struct hashmap_entry *entry;

    bzero(master,NICKLEN);
    bzero(slave,NICKLEN);

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db: %s\n", mysql_error(&get_core()->mysql_handle));
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&get_core()->mysql_handle,"DELETE FROM child_links");

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_links(), entry, link) {
        snprintf(tmp,1024,"INSERT INTO child_links VALUES ('%s','%s')",strtosql(master,link->master,NICKLEN),strtosql(slave,link->slave,NICKLEN));
        mysql_query(&get_core()->mysql_handle,tmp);
    }
}

void savebotservdb()
{
    struct hashmap_entry *entry;
    char tmp[1024];
    Chan *chptr;
    Bot *bot;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db: %s\n", mysql_error(&get_core()->mysql_handle));
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&get_core()->mysql_handle,"DELETE FROM child_botserv_bots");

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_bots(), entry, bot) {
        snprintf(tmp,1024,"INSERT INTO child_botserv_bots VALUES ('%s','%s','%s')",bot->nick,bot->ident,bot->host);
        mysql_query(&get_core()->mysql_handle,tmp);
    }

    mysql_query(&get_core()->mysql_handle,"DELETE FROM child_botserv_chans");

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_chans(), entry, chptr) {
        if (chptr->chanbot == NULL)
            continue;
        snprintf(tmp,1024,"INSERT INTO child_botserv_chans VALUES ('%s','%s')",chptr->channelname, chptr->chanbot->nick);
        mysql_query(&get_core()->mysql_handle,tmp);
    }
}

void savealldb()
{
    if (get_core()->verbose) printf("Saving DBs\n");

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db: %s\n", mysql_error(&get_core()->mysql_handle));
        operlog("Cannot connect to db");
        return;
    }


    saveuserdb();
    savechandb();
    savetrustdb();
    savelinkdb();
    savebotservdb();
    RunHooks(HOOK_SAVEDB,NULL,NULL,NULL,NULL); /* MOD_STOP is not checked here for some obvious reasons */
    if (get_core()->verbose) printf("DBs saved\n");
}

static int load_all_db(void)
{
    int err = 0;

    if (get_core()->verbose) printf("Loading DBs\n");

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db: %s\n", mysql_error(&get_core()->mysql_handle));
        operlog("Cannot connect to db");
        return -1;
    }

    err = load_user_db();
    if (err) {
        fprintf(stderr, "Failed to load user db.\n");
        return err;
    }

    err = load_link_db();
    if (err) {
        fprintf(stderr, "Failed to load link db.\n");
        return err;
    }

    err = load_trust_db();
    if (err) {
        fprintf(stderr, "Failed to load trust db.\n");
        return err;
    }

    err = load_chan_db();
    if (err) {
        fprintf(stderr, "Failed to load chan db.\n");
        return err;
    }

    err = load_cflag_db();
    if (err) {
        fprintf(stderr, "Failed to load cflag db.\n");
        return err;
    }

    err = load_botserv_bots_db();
    if (err) {
        fprintf(stderr, "Failed to load botserv bots db.\n");
        return err;
    }

    err = load_botserv_chans_db();
    if (err) {
        fprintf(stderr, "Failed to load botserv chan db.\n");
        return err;
    }

    RunHooks(HOOK_LOADDB,NULL,NULL,NULL,NULL);

    if (get_core()->verbose) printf("DBs loaded\n");

    return 0;
}

int connect_to_db()
{
    bool reconnect = true;

    mysql_init(&get_core()->mysql_handle);
    mysql_options(&get_core()->mysql_handle, MYSQL_OPT_RECONNECT, &reconnect);
    if (!mysql_real_connect(&get_core()->mysql_handle,core_get_config()->mysql_host,core_get_config()->mysql_login,core_get_config()->mysql_passwd,core_get_config()->mysql_db,0,NULL,0)) return 0;
    return 1;
}

static struct core_api db_api = {
    .load_all_db = load_all_db,
};

REGISTER_CORE_API(&db_api);
