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


#include "db.h"

#include "botserv.h"
#include "channel.h"
#include "child.h"
#include "core.h"
#include "hashmap.h"
#include "modules.h"
#include "string_utils.h"
#include "trust.h"
#include "user.h"

#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

extern cflaglist cflag_list;
extern chanbotlist chanbot_list;
extern chanlist chan_list;
extern linklist link_list;
extern trustlist trust_list;
extern userlist user_list;

extern MYSQL mysql;
extern int verbose, vv;

void loaduserdb()
{
    char accname[50];
    int acclevel;
    int seen,options,timeout;
    char md5_pass[35];
    User *uptr;
    MYSQL_RES *result;
    MYSQL_ROW row;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"SELECT * FROM child_users");
    if ((result = mysql_use_result(&mysql)) == NULL) {
        printf("CRITICAL: Cannot load user table: mysql_use_result returned NULL\n");
        return;
    }
    while ((row = mysql_fetch_row(result))) {
        strncpy(accname,row[0],50);
        acclevel = strtol(row[1],NULL,10);
        seen = strtol(row[2],NULL,10);
        strncpy(md5_pass,row[4],35);
        options = strtol(row[5],NULL,10);
        timeout = strtol(row[6],NULL,10);

        if (!timeout) timeout = TIMEOUT_DFLT;

        if (vv) printf("Adding user %s with level %d and pass %s and vhost %s\n",accname,acclevel,md5_pass,row[3]);

        if (find_user(accname))
            continue;

        uptr = AddUser(accname,acclevel);

        if (seen) uptr->lastseen = seen; else uptr->lastseen = time(NULL);
        uptr->options = options;
        uptr->timeout = timeout;

        if (vv) printf("User %s added (%p)\n",accname,uptr);

        strncpy(uptr->vhost,row[3],HOSTLEN);
        strncpy(uptr->email,row[7],EMAILLEN);
        uptr->regtime = strtol(row[8], NULL, 10);

        memset(uptr->md5_pass,'\0',34);
        strncpy(uptr->md5_pass,md5_pass,32);
    }

    mysql_close(&mysql);
}

void loadchandb()
{
    char channelname[CHANLEN+1];
    char owner[NICKLEN+1];
    User *owner_ptr;
    User *user_ptr;
    Chan *chan_ptr;
    char accname[NICKLEN+MASKLEN+1];
    int acclvl, automode, uflags;
    int options;
    char entrymsg[250];
    
    MYSQL_RES *result;
    MYSQL_ROW row;
    Cflag *cflag;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"SELECT * FROM child_chans");
    if ((result = mysql_use_result(&mysql)) == NULL) {
        printf("CRITICAL: Cannot load chan table: mysql_use_result returned NULL\n");
        return;
    }
    while ((row = mysql_fetch_row(result))) {
        strncpy(channelname,row[0],CHANLEN);
        strncpy(owner,row[1],NICKLEN);
        strncpy(entrymsg,row[2],200);
        options = strtol(row[3],NULL,10);
        owner_ptr = find_user(owner);

        if (!owner_ptr) {
            fprintf(stderr,"ERROR: Cannot load channel %s with owner %s : find_user returned NULL !!\n",channelname,owner);
            continue;
        }

        if (vv) printf("Adding channel %s with owner %s (%p) and options %d\n",channelname,owner,owner_ptr,options);

        if (find_channel(channelname))
            continue;
        chan_ptr = CreateChannel(channelname,owner,atoi(row[6]));
        chan_ptr->regtime = strtol(row[7], NULL, 10);

        if (vv) printf("Channel %s added (%p)\n",channelname,chan_ptr);

        chan_ptr->options = options;
        chan_ptr->autolimit = atoi(row[5]);
        strncpy(chan_ptr->entrymsg,entrymsg,sizeof(chan_ptr->entrymsg)-1);
        chan_ptr->entrymsg[250] = '\0';
        strncpy(chan_ptr->mlock,row[4],50);
        chan_ptr->mlock[50] = '\0';
        strncpy(chan_ptr->topic, row[8], TOPICLEN);
    }

    mysql_query(&mysql,"SELECT * from child_chan_access");
    if ((result = mysql_use_result(&mysql)) == NULL) {
        printf("CRITICAL: Cannot load chan access table: mysql_use_result returned NULL!\n");
        return;
    }
    while ((row = mysql_fetch_row(result))) {
        strncpy(channelname,row[0],CHANLEN);
        strncpy(accname,row[1],NICKLEN+MASKLEN);
        acclvl = strtol(row[2],NULL,10);
        automode = strtol(row[3],NULL,10);
        chan_ptr = find_channel(channelname);
        if (!chan_ptr) {
            fprintf(stderr,"ERROR: Cannot load access for user %s for channel %s : find_channel returned NULL !!\n",accname,channelname);
            continue;
        }

        user_ptr = find_user(accname);
        if (!user_ptr && !IsMask(accname)) {
            fprintf(stderr,"ERROR: Cannot load access for user %s for channel %s : find_user returned NULL !!\n",accname,channelname);
            continue;
        }

        if (find_cflag(accname,channelname))
            continue;
        if (vv) printf("Adding user %s (%p) for channel %s with level %d\n",accname,user_ptr,channelname,acclvl);
        if (acclvl < CHLEV_OWNER && !IsMask(accname)) {
            uflags = strtol(row[5], NULL, 10);
            cflag = AddUserToChannel(user_ptr,chan_ptr,acclvl,uflags);
            cflag->automode = automode;
            cflag->suspended = strtol(row[4], NULL, 10);
        }
        if (IsMask(accname))
            AddMaskToChannel(accname,chan_ptr,acclvl);
        if (vv) printf("User %s added for channel %s\n",accname,channelname);
    }

    mysql_close(&mysql);
}

void loadtrustdb()
{
    char host[50];
    int limit;
    MYSQL_RES *result;
    MYSQL_ROW row;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"SELECT * FROM child_trusts");
    if ((result = mysql_use_result(&mysql)) == NULL) {
        printf("CRITICAL: Cannot load trust table: mysql_use_result returned NULL\n");
        return;
    }
    while ((row = mysql_fetch_row(result))) {
        strncpy(host,row[0],50);
        limit = strtol(row[1],NULL,10);
        if (find_trust_strict(host)) continue;
        AddTrust(host,limit);
        if (vv) printf("Trust on %s added (limit: %d)\n",host,limit);
    }

    mysql_close(&mysql);
}

void loadlinkdb()
{
    char master[NICKLEN+1];
    char slave[NICKLEN+1];
    MYSQL_RES *result;
    MYSQL_ROW row;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"SELECT * FROM child_links");
    if ((result = mysql_use_result(&mysql)) == NULL) {
        printf("CRITICAL: Cannot load link table: mysql_use_result returned NULL\n");
        return;
    }
    while ((row = mysql_fetch_row(result))) {
        strncpy(master,row[0],NICKLEN);
        strncpy(slave,row[1],NICKLEN);
        if (find_link(slave) || find_link2(master,slave)) continue;
        AddLink(master,slave);
        if (vv) printf("Link added with master %s and slave %s\n",master,slave);
    }

    mysql_close(&mysql);
}

/* loadbotservdb *MUST* be executed *AFTER* loadchandb. Otherwise all data regarding assigned bots will be lost */

void loadbotservdb()
{
    char nick[NICKLEN+1];
    char ident[NICKLEN+1];
    char host[HOSTLEN+1];
    char chan[CHANLEN+1];
    MYSQL_RES *result;
    MYSQL_ROW row;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"SELECT * FROM child_botserv_bots");
    if ((result = mysql_use_result(&mysql)) == NULL) {
        fprintf(stderr,"CRITICAL: Cannot load botserv bots table: mysql_use_result returned NULL\n");
        return;
    }
    while ((row = mysql_fetch_row(result))) {
        strncpy(nick,row[0],NICKLEN);
        strncpy(ident,row[1],NICKLEN);
        strncpy(host,row[2],HOSTLEN);
        if (find_bot(nick)) continue;
        add_bot(nick, ident, host);
        if (vv) printf("Bot %s added (%s@%s)\n",nick,ident,host);
    }

    mysql_query(&mysql,"SELECT * FROM child_botserv_chans");
    if ((result = mysql_use_result(&mysql)) == NULL) {
        fprintf(stderr,"CRITICAL: Cannot load botserv chans table: mysql_use_result returned NULL\n");
        return;
    }
    while ((row = mysql_fetch_row(result))) {
        strncpy(chan,row[0],CHANLEN);
        strncpy(nick,row[1],NICKLEN);
        if (find_chanbot(chan)) continue;
        if (!find_bot(nick)) continue;
        if (!find_channel(chan)) continue;
        addChanbot(chan,nick);
        if (vv) printf("Chanbot %s added with bot %s\n",chan,nick);
    }

    mysql_close(&mysql);
}

void saveuserdb()
{
    char tmp[1024];
    char buf[512];
    User *uptr;
    bzero(buf,512);

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"DELETE FROM child_users");

    LIST_FOREACH_ALL(user_list, uptr) {
        snprintf(tmp,1024,"INSERT INTO child_users VALUES ('%s',%d,%d,'%s','%s',%ld,%d,'%s',%d)",strtosql(buf,uptr->nick,512),uptr->level,uptr->lastseen,uptr->vhost,uptr->md5_pass,uptr->options,uptr->timeout,uptr->email,uptr->regtime);
        mysql_query(&mysql,tmp);
    }

    mysql_close(&mysql);
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

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"DELETE FROM child_chans");
    mysql_query(&mysql,"DELETE FROM child_chan_access");

    LIST_FOREACH_ALL(chan_list, chptr) {
        bzero(chname,CHANLEN);
        bzero(nick,NICKLEN);
        bzero(owner,NICKLEN);
        bzero(entrymsg,250);
        bzero(topic, TOPICLEN);
        strtosql(chname,chptr->channelname, CHANLEN);
        strtosql(topic, chptr->topic, TOPICLEN);

        snprintf(tmp,1024,"INSERT INTO `child_chans` VALUES ('%s','%s','%s',%ld,'%s',%d,%d,%d,'%s')",chname,strtosql(owner,chptr->owner,NICKLEN),strtosql(entrymsg,chptr->entrymsg,250),chptr->options,chptr->mlock,chptr->autolimit,chptr->lastseen, chptr->regtime, topic);
        mysql_query(&mysql,tmp);
    }

    LIST_FOREACH_ALL(cflag_list, cflag) {
        if (cflag->flags >= CHLEV_OWNER)
            continue;
        bzero(chname,CHANLEN);
        strtosql(chname,cflag->channel,50);

        snprintf(tmp,1024,"INSERT INTO `child_chan_access` VALUES ('%s','%s',%d,%d,%d,%d)",chname,strtosql(nick,cflag->nick,NICKLEN),cflag->flags,cflag->automode,cflag->suspended,cflag->uflags);
        mysql_query(&mysql,tmp);
    }

    mysql_close(&mysql);
}

void savetrustdb()
{
    char tmp[1024];
    Trust *trust;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"DELETE FROM child_trusts");

    LIST_FOREACH_ALL(trust_list, trust) {
        snprintf(tmp,1024,"INSERT INTO child_trusts VALUES ('%s',%d)",trust->host,trust->limit);
        mysql_query(&mysql,tmp);
    }

    mysql_close(&mysql);
}

void savelinkdb()
{
    char tmp[1024];
    Link *link;
    char master[NICKLEN];
    char slave[NICKLEN];
    bzero(master,NICKLEN);
    bzero(slave,NICKLEN);

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"DELETE FROM child_links");

    LIST_FOREACH_ALL(link_list, link) {
        snprintf(tmp,1024,"INSERT INTO child_links VALUES ('%s','%s')",strtosql(master,link->master,NICKLEN),strtosql(slave,link->slave,NICKLEN));
        mysql_query(&mysql,tmp);
    }

    mysql_close(&mysql);
}

void savebotservdb()
{
    char tmp[1024];
    Bot *bot;
    Chanbot *chanbot;
    struct hashmap_entry *entry;

    if (!reconnect_to_db()) {
        fprintf(stderr,"Cannot connect to db\n");
        operlog("Cannot connect to db");
        return;
    }

    mysql_query(&mysql,"DELETE FROM child_botserv_bots");

    HASHMAP_FOREACH_ENTRY(get_core()->bots, entry) {
        bot = entry->value;
        snprintf(tmp,1024,"INSERT INTO child_botserv_bots VALUES ('%s','%s','%s')",bot->nick,bot->ident,bot->host);
        mysql_query(&mysql,tmp);
    }

    mysql_query(&mysql,"DELETE FROM child_botserv_chans");

    LIST_FOREACH_ALL(chanbot_list, chanbot) {
        snprintf(tmp,1024,"INSERT INTO child_botserv_chans VALUES ('%s','%s')",chanbot->name,chanbot->bot);
        mysql_query(&mysql,tmp);
    }

    mysql_close(&mysql);
}

void savealldb()
{
    if (verbose) printf("Saving DBs\n");
    saveuserdb();
    savechandb();
    savetrustdb();
    savelinkdb();
    savebotservdb();
    RunHooks(HOOK_SAVEDB,NULL,NULL,NULL,NULL); /* MOD_STOP is not checked here for some obvious reasons */
    if (verbose) printf("DBs saved\n");
}

void loadalldb()
{
    if (verbose) printf("Loading DBs\n");
    loaduserdb();
    loadlinkdb();
    loadtrustdb();
    loadchandb();
    loadbotservdb();
    RunHooks(HOOK_LOADDB,NULL,NULL,NULL,NULL);
    if (verbose) printf("DBs loaded\n");
}


int connect_to_db()
{
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql,me.mysql_host,me.mysql_login,me.mysql_passwd,me.mysql_db,0,NULL,0)) return 0;
    return 1;
}

int reconnect_to_db()
{
    mysql_close(&mysql);
    return connect_to_db();
}
