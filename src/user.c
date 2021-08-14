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
#include "core_api.h"
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

User *find_user(const char *name)
{
    struct hashmap_entry *entry;

    if (!name)
        return NULL;

    if (!HASHMAP_FIND(core_get_users(), name, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(core_get_users(), entry);
}

Guest *find_guest(const char *name)
{
    struct hashmap_entry *entry;

    if (!name)
        return NULL;

    if (!HASHMAP_FIND(core_get_guests(), name, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(core_get_guests(), entry);
}

/* A nickname has only one master but can have several slaves */

Link *find_link(const char *slave)
{
    struct hashmap_entry *entry;

    if (!slave)
        return NULL;

    if (!HASHMAP_FIND(core_get_links(), slave, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(core_get_links(), entry);
}

Link *find_link2 (char *master, char *slave)
{
    Link *tmp;
    struct hashmap_entry *entry;

    // TODO(target0): do a proper O(1) lookup.
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_links(), entry, tmp) {
        if (!Strcmp(tmp->master,master) && !Strcmp(tmp->slave,slave))
            return tmp;
    }

    return NULL;
}

Fake *find_fake(const char *nick_or_uid)
{
    Fake *tmp;
    struct hashmap_entry *entry;

    // TODO(target0): O(1) lookup.
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_fakeusers(), entry, tmp) {
        if (!Strcmp(tmp->nick, nick_or_uid) || !Strcmp(tmp->uid, nick_or_uid))
            return tmp;
    }

    return NULL;
}

User *AddUser (const char *nick, int level)
{
    User *new_user;
    new_user = (User *)malloc(sizeof(User));
    memset(new_user, 0, sizeof(*new_user));

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

    LLIST_INIT(&new_user->cflags);

    if (!HASHMAP_INSERT(core_get_users(), new_user->nick, new_user, NULL)) {
        fprintf(stderr, "Failed to insert new user \"%s\" into hashmap (duplicate entry?)\n", new_user->nick);
        free(new_user);
        return NULL;
    }

    return new_user;
}

Guest *AddGuest (char *nick, int timeout, int nickconn)
{
    Guest *new_guest;
    new_guest = (Guest *)malloc(sizeof(Guest));
    memset(new_guest, 0, sizeof(*new_guest));

    strncpy(new_guest->nick,nick,NICKLEN);
    new_guest->timeout = timeout;
    new_guest->nickconn = nickconn;

    if (!HASHMAP_INSERT(core_get_guests(), new_guest->nick, new_guest, NULL)) {
        fprintf(stderr, "Failed to insert new guest \"%s\" into hashmap (duplicate entry?)\n", new_guest->nick);
        free(new_guest);
        return NULL;
    }

    return new_guest;
}

Link *AddLink(const char *master, const char *slave)
{
    Link *new_link;
    new_link = (Link *)malloc(sizeof(Link));
    memset(new_link, 0, sizeof(*new_link));

    strncpy(new_link->master,master,NICKLEN);
    strncpy(new_link->slave,slave,NICKLEN);

    if (!HASHMAP_INSERT(core_get_links(), new_link->slave, new_link, NULL)) {
        fprintf(stderr, "Failed to insert new link \"%s\" into hashmap (duplicate entry?)\n", new_link->slave);
        free(new_link);
        return NULL;
    }

    return new_link;
}

Fake *AddFake(const char *nick, const char *ident, const char *host, const char *uid)
{
    Fake *new_fake;

    if ((find_fake(nick)) || find_fake(uid))
        return NULL;

    new_fake = malloc(sizeof(*new_fake));
    if (!new_fake)
        return NULL;
    memset(new_fake, 0, sizeof(*new_fake));

    strncpy(new_fake->nick, nick, NICKLEN + 1);
    strncpy(new_fake->ident, ident, NICKLEN + 1);
    strncpy(new_fake->host, host, HOSTLEN + 1);

    if (uid)
        strncpy(new_fake->uid, uid, UIDLEN + 1);
    else
        generate_uid(new_fake->uid);

    if (!HASHMAP_INSERT(core_get_fakeusers(), new_fake->nick, new_fake, NULL)) {
        fprintf(stderr, "Failed to insert new fakeuser \"%s\" into hashmap (duplicate entry?)\n", new_fake->nick);
        free(new_fake);
        return NULL;
    }

    return new_fake;
}

void DeleteAccount (User *user)
{
    if (!HASHMAP_ERASE(core_get_users(), user->nick))
        return;

    if (get_core_api()->find_nick(user->nick) && get_core()->eos)
        get_core_api()->send_raw("SVSMODE %s -r",user->nick);
    if (HasOption(user, UOPT_PROTECT)) DeleteGuest(user->nick);
    free(user);
}

void DeleteWildNick (Nick *nptr)
{
    Clone *clone;

    if (!HASHMAP_ERASE(core_get_nicks(), nptr->nick))
        return;

    if ((clone = find_clone(nptr->reshost)) != NULL) {
        clone->count--;
        if (clone->count == 0) {
            HASHMAP_ERASE(core_get_clones(), clone->host);
            free(clone);
        }
    }
    LLIST_REMOVE(&nptr->server_head);
    free(nptr);
}

void clear_nicks(void)
{
    struct hashmap_entry *entry, *tmp_entry;
    Nick *nptr;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_nicks(), entry, tmp_entry, nptr) {
        DeleteWildNick(nptr);
    }
}

void DeleteGuest (char *nick)
{
    Guest *guest = find_guest(nick);
    if (!guest) return;
    HASHMAP_ERASE(core_get_guests(), nick);
    free(guest);
}

void clear_guests(void)
{
    struct hashmap_entry *entry, *tmp_entry;
    Guest *guest;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_guests(), entry, tmp_entry, guest) {
        DeleteGuest(guest->nick);
    }
}

void DeleteLink (char *slave)
{
    Link *link = find_link(slave);
    if (!link) return;
    HASHMAP_ERASE(core_get_links(), slave);
    free(link);
}

void DeleteLinks (char *nick)
{
    Link *tmp;
    struct hashmap_entry *entry, *tmp_entry;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_links(), entry, tmp_entry, tmp) {
        if (!Strcmp(tmp->master,nick) || !Strcmp(tmp->slave,nick))
            DeleteLink(tmp->slave);
    }
}

void DeleteFake (Fake *fake)
{
    HASHMAP_ERASE(core_get_fakeusers(), fake->nick);
    free(fake);
}

void clear_fakes(void)
{
    struct hashmap_entry *entry, *tmp_entry;
    Fake *fake;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_fakeusers(), entry, tmp_entry, fake) {
        DeleteFake(fake);
    }
}

void FakeMsg(const char *who, const char *chan, const char *msg, ...)
{
    char buf[512];
    va_list val;
    ircsprintf(buf,511,msg,val);

    get_core_api()->send_raw(":%s PRIVMSG %s :%s",who,chan,buf);
}

void FakeNotice(const char *who, const Nick *nptr, const char *msg, ...)
{
    char buf[512];
    va_list val;
    ircsprintf(buf,511,msg,val);
    
    get_core_api()->send_raw(":%s NOTICE %s :%s",who,nptr->nick,buf);
}

void globops(char *msg, ...)
{
    char buf[512];
    va_list val;

    ircsprintf(buf,511,msg,val);

    get_core_api()->send_raw("GLOBOPS :%s",buf);
}

void send_global (char *target, char *msg, ...)
{
    char buf[512];
    va_list val;

    ircsprintf(buf,511,msg,val);

    get_core_api()->send_raw(":%s NOTICE $%s :%s", core_get_config()->nick, target, buf);
}

Clone *find_clone (char *host)
{
    struct hashmap_entry *entry;

    if (!host)
        return NULL;

    if (!HASHMAP_FIND(core_get_clones(), host, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(core_get_clones(), entry);
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

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_guests(), entry, tmp_entry, guest) {
        // TODO(target0): improve this.
        if ((time(NULL) - guest->nickconn) >= guest->timeout) {
            gv = random()%999999;
            gv += hash(guest->nick);
            gv = gv%999999;
            get_core_api()->send_raw("SVSNICK %s %s%d %ld",guest->nick,core_get_config()->guest_prefix,gv,time(NULL));
            DeleteGuest(guest->nick);
        }
    }
}

void userquit (char *nick)
{
    User *uptr;
    Nick *nptr;

    nptr = get_core_api()->find_nick(nick);
    if (!nptr) return;

    uptr = find_account(nptr);
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
        snprintf(cmd,256,"%s %s",core_get_config()->sendmail,to);
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

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_bots(), entry, bot) {
        fakekill(bot->nick,"Exiting");
    }
}

void loadallfakes()
{
    struct hashmap_entry *entry;
    Bot *bot;

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_bots(), entry, bot) {
        generate_uid(bot->uid);
        fakeuser(bot->nick, bot->ident, bot->host, bot->uid, BOTSERV_UMODES);
        get_core_api()->send_raw("SQLINE %s :Reserved for services", bot->nick);
    }
}

void userdrop (User *uptr)
{
    struct hashmap_entry *entry, *tmp_entry;
    Chan *chptr;
    User *uptr2;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_chans(), entry, tmp_entry, chptr) {
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

void sync_user(const User *uptr)
{
    Cflag *cflag;

    LLIST_FOREACH_ENTRY(&uptr->cflags, cflag, user_head) {
        sync_cflag(cflag);
    }
}

void sync_cflag(const Cflag *cflag)
{
    const char *chname = cflag->chan->channelname;
    Wchan *wchan;
    Member *member;
    Nick *nptr;
    const char *bot;

    if ((nptr = get_core_api()->find_nick(cflag->user->nick)) == NULL)
        return;

    if ((wchan = find_wchan(chname)) == NULL)
        return;

    if ((member = find_member(wchan, nptr)) == NULL)
        return;

    if (HasOption(cflag->chan, COPT_NOAUTO))
        return;

    if ((!HasOption(cflag->chan, COPT_AXXFLAGS) && cflag->automode == CFLAG_AUTO_OFF) || cflag->suspended == 1)
        return;

    bot = channel_botname(cflag->chan);
    int hasaccess = 0;

    if (HasOption(cflag->chan, COPT_AXXFLAGS)) {
        if (cflag->uflags & UFLAG_AUTOOWNER) SetStatus(nptr, chname, CHFL_OWNER, 1, bot);
        if (cflag->uflags & UFLAG_AUTOPROTECT) SetStatus(nptr, chname, CHFL_PROTECT, 1, bot);
        if (cflag->uflags & UFLAG_AUTOOP) SetStatus(nptr, chname, CHFL_OP, 1, bot);
        if (cflag->uflags & UFLAG_AUTOHALFOP) SetStatus(nptr, chname, CHFL_HALFOP, 1, bot);
        if (cflag->uflags & UFLAG_AUTOVOICE) SetStatus(nptr, chname, CHFL_VOICE, 1, bot);
        if (cflag->uflags & UFLAG_NOOP) SetStatus(nptr, chname, member->flags, 0, bot);
        if (cflag->uflags & UFLAG_AUTOKICK) KickUser(bot, nptr->nick, chname, "Get out of this chan !");
        if (cflag->uflags & UFLAG_AUTOKICKBAN) {
            get_core_api()->send_raw(":%s MODE %s +b *!*@%s", bot, nptr->nick, nptr->hiddenhost);
            KickUser(bot, nptr->nick, chname, "Get out of this chan !");
        }
    } else {
    if ((cflag->flags == CHLEV_OWNER || cflag->flags == CHLEV_COOWNER)) {
        if ((cflag->automode == CFLAG_AUTO_OP || cflag->automode == CFLAG_AUTO_ON) && (!HasOwner(member) || !HasOp(member)))
            SetStatus(nptr, chname, CHFL_OWNER|CHFL_OP, 1, bot);
        else if (cflag->automode == CFLAG_AUTO_VOICE && !HasVoice(member))
            SetStatus(nptr, chname, CHFL_VOICE, 1, bot);

        hasaccess = 1;
    } else if (!hasaccess && !(cflag->flags == CHLEV_OWNER) && cflag->flags >= core_get_config()->chlev_admin) {
        if ((cflag->automode == CFLAG_AUTO_OP || cflag->automode == CFLAG_AUTO_ON) && (!HasProtect(member) || !HasOp(member)))
            SetStatus(nptr, chname, CHFL_PROTECT|CHFL_OP, 1, bot);
        else if (cflag->automode == CFLAG_AUTO_VOICE && !HasVoice(member))
            SetStatus(nptr, chname, CHFL_VOICE, 1, bot);

        hasaccess = 1;
    } else if (!hasaccess && cflag->flags >= core_get_config()->chlev_op && cflag->flags < core_get_config()->chlev_admin) {
        if ((cflag->automode == CFLAG_AUTO_OP || cflag->automode == CFLAG_AUTO_ON) && (!HasOp(member)))
            SetStatus(nptr, chname, CHFL_OP, 1, bot);
        else if (cflag->automode == CFLAG_AUTO_VOICE && !HasVoice(member))
            SetStatus(nptr, chname, CHFL_VOICE, 1, bot);

        hasaccess = 1;
    } else if (!hasaccess && cflag->flags >= core_get_config()->chlev_halfop && cflag->flags < core_get_config()->chlev_op) {
        if ((cflag->automode == CFLAG_AUTO_OP || cflag->automode == CFLAG_AUTO_ON) && (!HasHalfop(member)))
            SetStatus(nptr, chname, CHFL_HALFOP, 1, bot);
        else if (cflag->automode == CFLAG_AUTO_VOICE && !HasVoice(member))
            SetStatus(nptr, chname, CHFL_VOICE, 1, bot);

        hasaccess = 1;
    } else if (!hasaccess && cflag->flags >= core_get_config()->chlev_voice && cflag->flags < core_get_config()->chlev_halfop && !HasVoice(member)) {
        SetStatus(nptr, chname, CHFL_VOICE, 1, bot);
        hasaccess = 1;
    } else if (cflag->flags == core_get_config()->chlev_akick) {
        KickUser(bot, nptr->nick, chname, "Get out of this chan !");
    } else if (cflag->flags == core_get_config()->chlev_akb) {
        get_core_api()->send_raw(":%s MODE %s +b *!*@%s", bot, nptr->nick, nptr->hiddenhost);
        KickUser(bot, nptr->nick, chname, "Get out of this chan !");
    } else if (cflag->flags == core_get_config()->chlev_nostatus)
        SetStatus(nptr, chname, member->flags, 0, bot);
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

    if ((nptr = get_core_api()->find_nick(uptr->nick)) == NULL)
        return 0;

    if (!IsAuthed(uptr))
        return 0;

    if (HasUmode(nptr, UMODE_SUPERADMIN))
        return 1;

    return 0;
}

/* From https://github.com/unrealircd/unrealircd/blob/b65584226c1d76aed8119549c586f441a9dbb8ee/src/user.c#L625 */
static char uid_int_to_char(int v)
{
    if (v < 10)
        return '0' + v;
    else
        return 'A' + v - 10;
}

void generate_uid(char *dst_uid)
{
    char uid[UIDLEN + 1];

    do {
        snprintf(uid, sizeof(uid), "%s%c%c%c%c%c%c",
                 core_get_config()->sid,
                 uid_int_to_char(random() % 36),
                 uid_int_to_char(random() % 36),
                 uid_int_to_char(random() % 36),
                 uid_int_to_char(random() % 36),
                 uid_int_to_char(random() % 36),
                 uid_int_to_char(random() % 36));
    } while (get_core_api()->find_nick(uid));

    strncpy(dst_uid, uid, UIDLEN + 1);
}
