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


#include "user.h"

#include "botserv.h"
#include "channel.h"
#include "child.h"
#include "core.h"
#include "hashmap.h"
#include "mem.h"
#include "net.h"
#include "string_utils.h"

#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

extern cflaglist cflag_list;
extern fakelist fake_list;

extern int eos;

User *find_user(char *name)
{
    struct hashmap_entry *entry;

    if (!HASHMAP_FIND(get_core()->users, name, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(get_core()->users, entry);
}

Nick *find_nick(char *name)
{
    Nick *tmp;
    struct hashmap_entry *entry;

    // TODO(target0): do a proper O(1) lookup.
    HASHMAP_FOREACH_ENTRY_VALUE(get_core()->nicks, entry, tmp) {
        if (!Strcmp(tmp->nick,name) || !Strcmp(tmp->uid,name))
            return tmp;
    }

    return NULL;
}

Guest *find_guest(char *name)
{
    struct hashmap_entry *entry;

    if (!HASHMAP_FIND(get_core()->guests, name, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(get_core()->guests, entry);
}

/* A nickname has only one master but can have several slaves */

Link *find_link (char *slave)
{
    struct hashmap_entry *entry;

    if (!HASHMAP_FIND(get_core()->links, slave, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(get_core()->links, entry);
}

Link *find_link2 (char *master, char *slave)
{
    Link *tmp;
    struct hashmap_entry *entry;

    // TODO(target0): do a proper O(1) lookup.
    HASHMAP_FOREACH_ENTRY_VALUE(get_core()->links, entry, tmp) {
        if (!Strcmp(tmp->master,master) && !Strcmp(tmp->slave,slave))
            return tmp;
    }

    return NULL;
}

Fake *find_fake (char *nick)
{
    Fake *tmp;
    LIST_FOREACH(fake_list, tmp, HASH(nick)) {
        if (!Strcmp(tmp->nick, nick))
            return tmp;
    }

    return NULL;
}

User *AddUser (char *nick, int level)
{
    User *new_user;
    new_user = (User *)malloc(sizeof(User));

    strncpy(new_user->nick,nick,NICKLEN);
    new_user->level = level;
    new_user->authed = 0;
    new_user->options = 0;
    SetOption(new_user, UOPT_HIDEMAIL);
    new_user->timeout = TIMEOUT_DFLT;
    bzero(new_user->vhost,HOSTLEN);
    bzero(new_user->email,EMAILLEN);
    new_user->regtime = 0;
    new_user->lastseen = 0;

    if (!HASHMAP_INSERT(get_core()->users, new_user->nick, new_user, NULL)) {
        fprintf(stderr, "Failed to insert new user \"%s\" into hashmap (duplicate entry?)\n", new_user->nick);
        free(new_user);
        return NULL;
    }

    return new_user;
}

Nick *AddNick(char *nick, char *ident, char *host, char *uid, char *hiddenhost, long int umodes, char *reshost)
{
    Nick *new_nick;

    new_nick = (Nick *)malloc(sizeof(Nick));

    strncpy(new_nick->nick,nick,NICKLEN);
    strncpy(new_nick->ident,ident,NICKLEN);
    strncpy(new_nick->host,host,HOSTLEN);
    strncpy(new_nick->uid,uid,HOSTLEN);
    strncpy(new_nick->hiddenhost,hiddenhost,HOSTLEN);
    strncpy(new_nick->reshost,reshost,HOSTLEN);
    new_nick->umodes = umodes;
    new_nick->msgnb = 0;
    new_nick->msgtime = 0;
    new_nick->ignored = 0;
    new_nick->ignoretime = 0;
    new_nick->loginattempts = 0;
    new_nick->lasttry = 0;

    if (!HASHMAP_INSERT(get_core()->nicks, new_nick->nick, new_nick, NULL)) {
        fprintf(stderr, "Failed to insert new nick \"%s\" into hashmap (duplicate entry?)\n", new_nick->nick);
        free(new_nick);
        return NULL;
    }

    Clone *clone;
    if ((clone = find_clone(reshost)) != NULL)
        clone->count++;
    else {
        clone = (Clone *)malloc(sizeof(Clone));
        strncpy(clone->host, reshost, HOSTLEN);
        clone->count = 1;
        if (!HASHMAP_INSERT(get_core()->clones, clone->host, clone, NULL)) {
            fprintf(stderr, "Failed to insert new clone \"%s\" into hashmap (duplicate entry?)\n", clone->host);
            free(clone);
        }
    }

    return new_nick;
}

Guest *AddGuest (char *nick, int timeout, int nickconn)
{
    Guest *new_guest;
    new_guest = (Guest *)malloc(sizeof(Guest));

    strncpy(new_guest->nick,nick,NICKLEN);
    new_guest->timeout = timeout;
    new_guest->nickconn = nickconn;

    if (!HASHMAP_INSERT(get_core()->guests, new_guest->nick, new_guest, NULL)) {
        fprintf(stderr, "Failed to insert new guest \"%s\" into hashmap (duplicate entry?)\n", new_guest->nick);
        free(new_guest);
        return NULL;
    }

    return new_guest;
}

Link *AddLink(char *master, char *slave)
{
    Link *new_link;
    new_link = (Link *)malloc(sizeof(Link));

    strncpy(new_link->master,master,NICKLEN);
    strncpy(new_link->slave,slave,NICKLEN);

    if (!HASHMAP_INSERT(get_core()->links, new_link->slave, new_link, NULL)) {
        fprintf(stderr, "Failed to insert new link \"%s\" into hashmap (duplicate entry?)\n", new_link->slave);
        free(new_link);
        return NULL;
    }

    return new_link;
}

Fake *AddFake(char *nick, char *ident, char *host)
{
    Fake *new_fake;
    if ((find_fake(nick)) != NULL)
        return NULL;
    new_fake = (Fake *)malloc(sizeof(Fake));

    strncpy(new_fake->nick, nick, NICKLEN);
    strncpy(new_fake->ident, ident, NICKLEN);
    strncpy(new_fake->host, host, HOSTLEN);

    LIST_INSERT_HEAD(fake_list, new_fake, HASH(nick));
    return new_fake;
}

void DeleteAccount (User *user)
{
    if (!HASHMAP_ERASE(get_core()->users, user->nick))
        return;

    if (find_nick(user->nick) && eos)
        SendRaw("SVSMODE %s -r",user->nick);
    if (HasOption(user, UOPT_PROTECT)) DeleteGuest(user->nick);
    free(user);
}

void DeleteWildNick (Nick *nptr)
{
    Clone *clone;

    if (!HASHMAP_ERASE(get_core()->nicks, nptr->nick))
        return;

    if ((clone = find_clone(nptr->reshost)) != NULL) {
        clone->count--;
        if (clone->count == 0) {
            HASHMAP_ERASE(get_core()->clones, clone->host);
            free(clone);
        }
    }
    free(nptr);
}

void clear_nicks(void)
{
    struct hashmap_entry *entry, *tmp_entry;
    Nick *nptr;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(get_core()->nicks, entry, tmp_entry, nptr) {
        DeleteWildNick(nptr);
    }
}

void DeleteGuest (char *nick)
{
    Guest *guest = find_guest(nick);
    if (!guest) return;
    HASHMAP_ERASE(get_core()->guests, nick);
    free(guest);
}

void clear_guests(void)
{
    struct hashmap_entry *entry, *tmp_entry;
    Guest *guest;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(get_core()->guests, entry, tmp_entry, guest) {
        DeleteGuest(guest->nick);
    }
}

void DeleteLink (char *slave)
{
    Link *link = find_link(slave);
    if (!link) return;
    HASHMAP_ERASE(get_core()->links, slave);
    free(link);
}

void DeleteLinks (char *nick)
{
    Link *tmp;
    struct hashmap_entry *entry, *tmp_entry;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(get_core()->links, entry, tmp_entry, tmp) {
        if (!Strcmp(tmp->master,nick) || !Strcmp(tmp->slave,nick))
            DeleteLink(tmp->slave);
    }
}

void DeleteFake (Fake *fake)
{
    LIST_REMOVE(fake_list, fake, HASH(fake->nick));
    free(fake);
}

void FakeMsg (char *who, char *chan, char *msg, ...)
{
    char buf[512];
    va_list val;
    ircsprintf(buf,511,msg,val);

    SendRaw(":%s PRIVMSG %s :%s",who,chan,buf);
}

void FakeNotice (char *who, Nick *nptr, char *msg, ...)
{
    char buf[512];
    va_list val;
    ircsprintf(buf,511,msg,val);
    
    SendRaw(":%s NOTICE %s :%s",who,nptr->nick,buf);
}

void globops(char *msg, ...)
{
    char buf[512];
    va_list val;

    ircsprintf(buf,511,msg,val);

    SendRaw("GLOBOPS :%s",buf);
}

void send_global (char *target, char *msg, ...)
{
    char buf[512];
    va_list val;

    ircsprintf(buf,511,msg,val);

    SendRaw(":%s NOTICE $%s :%s", me.nick, target, buf);
}

Clone *find_clone (char *host)
{
    struct hashmap_entry *entry;

    if (!HASHMAP_FIND(get_core()->clones, host, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(get_core()->clones, entry);
}

int howmanyclones(char *host)
{
    Clone *clone;
    if ((clone = find_clone(host)) != NULL)
        return clone->count;
    return 0;
}

void CheckGuests()
{
    struct hashmap_entry *entry, *tmp_entry;
    Guest *guest;
    int gv;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(get_core()->guests, entry, tmp_entry, guest) {
        // TODO(target0): improve this.
        if ((time(NULL) - guest->nickconn) >= guest->timeout) {
            init_srandom();
            gv = random()%999999;
            gv += hash(guest->nick);
            gv = gv%999999;
            SendRaw("SVSNICK %s %s%d %ld",guest->nick,me.guest_prefix,gv,time(NULL));
            DeleteGuest(guest->nick);
        }
    }
}

void userquit (char *nick)
{
    User *uptr;
    Nick *nptr;

    nptr = find_nick(nick);
    if (!nptr) return;

    uptr = find_user(nick);
    if (uptr) {
        if (uptr->authed == 1) {
            uptr->authed = 0;
            uptr->lastseen = time(NULL);
        } else {
            if (HasOption(uptr, UOPT_PROTECT))
                DeleteGuest(nptr->nick);
        }
    }
    
    DeleteUserFromWchans(nptr);
    DeleteWildNick(nptr);
}

int match_mask (char *reg, char *tomatch)
{
    char mask[128];
    bzero(mask,128);

    unsigned int i,j;
    for (i=0,j=0;i<strlen(reg);i++,j++) {
        if (reg[i] == '*') mask[j++] = '.';
        mask[j] = reg[i];
    }

    if (__match_regex(mask, tomatch, REG_EXTENDED|REG_NOSUB|REG_ICASE)) return 1;
    return 0;
}

int IsMask(char *mask)
{
    char tmp[128];
    char *nick,*ident,*host;
    strncpy(tmp,mask,128);

    nick = tmp;
    ident = strstr(nick,"!");
    if (!ident) return 0;
    *ident = '\0';
    ident++;
    host = strstr(ident,"@");
    if (!host) return 0;
    *host = '\0';
    host++;

    if (*nick == '\0' || *ident == '\0' || *host == '\0')
        return 0;

    return 1;
}

int sendmail(char *to, char *mail)
{
    int pid = fork();
    if (pid == 0) { /* child pid */
        FILE *fp;
        char cmd[256];
        snprintf(cmd,256,"%s %s",me.sendmail,to);
        if ((fp = popen(cmd,"w")) == NULL)
            return 0;

        fprintf(fp,"%s",mail);
        pclose(fp);
        exit(0);
    } else if (pid < 0)
        return 0;

    return 1;
}

void killallfakes()
{
    struct hashmap_entry *entry;
    Bot *bot;

    HASHMAP_FOREACH_ENTRY_VALUE(get_core()->bots, entry, bot) {
        fakekill(bot->nick,"Exiting");
    }
}

void loadallfakes()
{
    struct hashmap_entry *entry;
    Bot *bot;

    HASHMAP_FOREACH_ENTRY_VALUE(get_core()->bots, entry, bot) {
        fakeuser(bot->nick,bot->ident,bot->host,BOTSERV_UMODES);
        SendRaw("SQLINE %s :Reserved for services",bot->nick);
    }
}

void userdrop (User *uptr)
{
    struct hashmap_entry *entry, *tmp_entry;
    Chan *chptr;
    User *uptr2;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(get_core()->chans, entry, tmp_entry, chptr) {
        if (!Strcmp(chptr->owner, uptr->nick)) {
            if ((uptr2 = get_coowner(chptr)) != NULL) {
                DeleteUserFromChannel(uptr2, chptr);
                AddUserToChannel(uptr2, chptr, CHLEV_OWNER, UFLAG_CHANOWNER);
                strncpy(chptr->owner, uptr2->nick, NICKLEN);
            } else
                chandrop(chptr);
        }
    }

    DeleteLinks(uptr->nick);
    DeleteUserFromChannels(uptr);
    DeleteAccount(uptr);
}

void sync_user (Nick *nptr)
{
    Cflag *cflag;

    LIST_FOREACH_ALL(cflag_list, cflag) {
        if (!Strcmp(cflag->nick, nptr->nick))
            sync_cflag(cflag, nptr);
    }
}

void sync_cflag (Cflag *cflag, Nick *nptr)
{
    Chan *chptr;
    Wchan *wchan;
    User *uptr;
    Member *member;
    char *bot;

    if ((uptr = find_user(nptr->nick)) == NULL)
        return;

    if ((chptr = find_channel(cflag->channel)) == NULL)
        return;

    if ((member = find_member(cflag->channel, uptr->nick)) == NULL)
        return;

    if (HasOption(chptr, COPT_NOAUTO))
        return;

    if ((wchan = find_wchan(cflag->channel)) == NULL)
        return;

    if ((!HasOption(chptr, COPT_AXXFLAGS) && cflag->automode == CFLAG_AUTO_OFF) || cflag->suspended == 1)
        return;

    bot = whatbot(wchan->chname);
    int hasaccess = 0;

    if (HasOption(chptr, COPT_AXXFLAGS)) {
        if (cflag->uflags & UFLAG_AUTOOWNER) SetStatus(nptr, cflag->channel, CHFL_OWNER, 1, bot);
        if (cflag->uflags & UFLAG_AUTOPROTECT) SetStatus(nptr, cflag->channel, CHFL_PROTECT, 1, bot);
        if (cflag->uflags & UFLAG_AUTOOP) SetStatus(nptr, cflag->channel, CHFL_OP, 1, bot);
        if (cflag->uflags & UFLAG_AUTOHALFOP) SetStatus(nptr, cflag->channel, CHFL_HALFOP, 1, bot);
        if (cflag->uflags & UFLAG_AUTOVOICE) SetStatus(nptr, cflag->channel, CHFL_VOICE, 1, bot);
        if (cflag->uflags & UFLAG_NOOP) SetStatus(nptr, cflag->channel, member->flags, 0, bot);
        if (cflag->uflags & UFLAG_AUTOKICK) KickUser(bot, nptr->nick, cflag->channel, "Get out of this chan !");
        if (cflag->uflags & UFLAG_AUTOKICKBAN) {
            SendRaw(":%s MODE %s +b *!*@%s", bot, nptr->nick, nptr->hiddenhost);
            KickUser(bot, nptr->nick, cflag->channel, "Get out of this chan !");
        }
    } else {
    if ((cflag->flags == CHLEV_OWNER || cflag->flags == CHLEV_COOWNER)) {
        if ((cflag->automode == CFLAG_AUTO_OP || cflag->automode == CFLAG_AUTO_ON) && (!IsOwner(nptr->nick, wchan) || !IsOp(nptr->nick, wchan)))
            SetStatus(nptr, cflag->channel, CHFL_OWNER|CHFL_OP, 1, bot);
        else if (cflag->automode == CFLAG_AUTO_VOICE && !IsVoice(nptr->nick, wchan))
            SetStatus(nptr, cflag->channel, CHFL_VOICE, 1, bot);

        hasaccess = 1;
    } else if (!hasaccess && !(cflag->flags == CHLEV_OWNER) && cflag->flags >= me.chlev_admin) {
        if ((cflag->automode == CFLAG_AUTO_OP || cflag->automode == CFLAG_AUTO_ON) && (!IsProtect(nptr->nick, wchan) || !IsOp(nptr->nick, wchan)))
            SetStatus(nptr, cflag->channel, CHFL_PROTECT|CHFL_OP, 1, bot);
        else if (cflag->automode == CFLAG_AUTO_VOICE && !IsVoice(nptr->nick, wchan))
            SetStatus(nptr, cflag->channel, CHFL_VOICE, 1, bot);

        hasaccess = 1;
    } else if (!hasaccess && cflag->flags >= me.chlev_op && cflag->flags < me.chlev_admin) {
        if ((cflag->automode == CFLAG_AUTO_OP || cflag->automode == CFLAG_AUTO_ON) && (!IsOp(nptr->nick, wchan)))
            SetStatus(nptr, cflag->channel, CHFL_OP, 1, bot);
        else if (cflag->automode == CFLAG_AUTO_VOICE && !IsVoice(nptr->nick, wchan))
            SetStatus(nptr, cflag->channel, CHFL_VOICE, 1, bot);

        hasaccess = 1;
    } else if (!hasaccess && cflag->flags >= me.chlev_halfop && cflag->flags < me.chlev_op) {
        if ((cflag->automode == CFLAG_AUTO_OP || cflag->automode == CFLAG_AUTO_ON) && (!IsHalfop(nptr->nick, wchan)))
            SetStatus(nptr, cflag->channel, CHFL_HALFOP, 1, bot);
        else if (cflag->automode == CFLAG_AUTO_VOICE && !IsVoice(nptr->nick, wchan))
            SetStatus(nptr, cflag->channel, CHFL_VOICE, 1, bot);

        hasaccess = 1;
    } else if (!hasaccess && cflag->flags >= me.chlev_voice && cflag->flags < me.chlev_halfop && !IsVoice(nptr->nick, wchan)) {
        SetStatus(nptr, cflag->channel, CHFL_VOICE, 1, bot);
        hasaccess = 1;
    } else if (cflag->flags == me.chlev_akick) {
        KickUser(bot, nptr->nick, cflag->channel, "Get out of this chan !");
    } else if (cflag->flags == me.chlev_akb) {
        SendRaw(":%s MODE %s +b *!*@%s", bot, nptr->nick, nptr->hiddenhost);
        KickUser(bot, nptr->nick, cflag->channel, "Get out of this chan !");
    } else if (cflag->flags == me.chlev_nostatus)
        SetStatus(nptr, cflag->channel, member->flags, 0, bot);
    }
}

User *get_link_master (User *uptr)
{
    Link *l;
    if ((l = find_link(uptr->nick)) == NULL)
        return NULL;
    return find_user(l->master);
}

int IsSuperAdmin (User *uptr)
{
    Nick *nptr;

    if ((nptr = find_nick(uptr->nick)) == NULL)
        return 0;

    if (!IsAuthed(uptr))
        return 0;

    if (HasUmode(nptr, UMODE_SUPERADMIN))
        return 1;

    return 0;
}
