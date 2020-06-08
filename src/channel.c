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


#include "channel.h"

#include "botserv.h"
#include "child.h"
#include "core.h"
#include "hashmap.h"
#include "mem.h"
#include "net.h"
#include "string_utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern int eos;

Chan *find_channel(const char *name)
{
    struct hashmap_entry *entry;

    if (!HASHMAP_FIND(core_get_chans(), name, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(core_get_chans(), entry);
}

Wchan *find_wchan(const char *name)
{
    struct hashmap_entry *entry;

    if (!HASHMAP_FIND(core_get_wchans(), name, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(core_get_wchans(), entry);
}

Cflag *find_cflag(const Chan *chptr, const User *uptr)
{
    Cflag *cflag;

    LLIST_FOREACH_ENTRY(&uptr->cflags, cflag, user_head) {
        if (cflag->chan == chptr)
            return cflag;
    }

    return NULL;
}

Cflag *find_cflag_recursive(const Chan *chptr, const User *uptr)
{
    Cflag *cflag;
    Link *l;
    User *uptr2;

    if ((cflag = find_cflag(chptr, uptr)) == NULL) {
        if ((l = find_link(uptr->nick)) == NULL)
            return NULL;
        if ((uptr2 = find_user(l->master)) == NULL)
            return NULL;
        return find_cflag_recursive(chptr, uptr2);
    }

    return cflag;
}

Timeban *find_timeban(const Chan *chptr, const char *mask)
{
    Timeban *tmp;

    LLIST_FOREACH_ENTRY(&chptr->timebans, tmp, chan_head) {
        if (!Strcmp(tmp->mask, mask))
            return tmp;
    }

    return NULL;
}

int GetFlag(User *uptr, Chan *chptr)
{
    Cflag *cflag;

    Nick *nptr;
    nptr = find_nick(uptr->nick);
    if (nptr) {
        if (HasUmode(nptr, UMODE_SUPERADMIN))
            return CHLEV_OWNER;
    }

    if (!uptr || !chptr) return 0;
    if (IsChanSuspended(chptr)) return 0;
    cflag = find_cflag(chptr, uptr);

/*
    if no result is found, let's have a look in the access list of the user's
    linked nick.
*/

    if (!cflag) {
        Link *l = find_link(uptr->nick);
        if (!l) return 0;
        User *user = find_user(l->master);
        if (!user) return 0;
        return GetFlag(user,chptr);
    }
    if (cflag->suspended == 1)
        return 0;
    return cflag->flags;
}

Chan *CreateChannel (char *name, char *owner, int lastseen)
{
    Chan *new_chan;
    new_chan = (Chan *)malloc(sizeof(Chan));

    strncpy(new_chan->channelname,name,CHANLEN);
    strncpy(new_chan->owner,owner,NICKLEN);
    new_chan->options = COPT_AXXFLAGS | COPT_SECURE;
    new_chan->regtime = 0;
    new_chan->autolimit = 0;
    bzero(new_chan->entrymsg,250);
    bzero(new_chan->mlock,50);
    bzero(new_chan->topic, TOPICLEN);
    if (!lastseen)
        new_chan->lastseen = time(NULL);
    else
        new_chan->lastseen = lastseen;

    LLIST_INIT(&new_chan->cflags);
    new_chan->active_autolimit = NULL;
    new_chan->chanbot = NULL;
    LLIST_INIT(&new_chan->timebans);

    if (!HASHMAP_INSERT(core_get_chans(), new_chan->channelname, new_chan, NULL)) {
        fprintf(stderr, "Failed to insert new channel \"%s\" into hashmap (duplicate entry?)\n", new_chan->channelname);
        free(new_chan);
        return NULL;
    }

    Chan *chan;
    User *uptr;

    chan = find_channel(name);
    uptr = find_user(owner);
    if (!chan || !uptr) return NULL;

/*
    There are two ways to recognize the owner of a channel. First, the field "owner" in
    the channel struct and then the user having a level CHLEV_OWNER in the channel access
    list.
*/

    AddUserToChannel(uptr, chan, CHLEV_OWNER, UFLAG_CHANOWNER);
    return new_chan;
}

Wchan *CreateWchan(char *name)
{
    Wchan *new_chan;
    new_chan = (Wchan *)malloc(sizeof(Wchan));

    strncpy(new_chan->chname,name,CHANLEN);
    bzero(new_chan->topic, TOPICLEN);
    LLIST_INIT(&new_chan->members);

    if (!HASHMAP_INSERT(core_get_wchans(), new_chan->chname, new_chan, NULL)) {
        fprintf(stderr, "Failed to insert new wchan \"%s\" into hashmap (duplicate entry?)\n", new_chan->chname);
        free(new_chan);
        return NULL;
    }

    return new_chan;
}

Limit *AddLimit(Chan *chptr)
{
    Limit *limit = chptr->active_autolimit;

    if (limit) {
        limit->time = time(NULL)+me.limittime;
        return limit;
    }

    limit = (Limit *)malloc(sizeof(Limit));

    limit->chan = chptr;
    limit->time = time(NULL)+me.limittime;
    chptr->active_autolimit = limit;

    LLIST_INSERT_TAIL(core_get_limits(), &limit->list_head);

    return limit;
}

Timeban *AddTimeban(Chan *chan, const char *mask, int duration, const char *reason)
{
    Timeban *new_tb;

    new_tb = (Timeban *)malloc(sizeof(Timeban));
    new_tb->chan = chan;
    strncpy(new_tb->mask, mask, MASKLEN);
    strncpy(new_tb->reason, reason, 256);
    new_tb->duration = duration;
    new_tb->setat = time(NULL);

    LLIST_INSERT_TAIL(&chan->timebans, &new_tb->chan_head);
    LLIST_INSERT_TAIL(core_get_timebans(), &new_tb->core_head);

    return new_tb;
}

void DeleteUsersFromChannel (Chan *chan)
{
    Cflag *cflag, *tmp_cflag;

    LLIST_FOREACH_ENTRY_SAFE(&chan->cflags, cflag, tmp_cflag, chan_head) {
        DeleteCflag(cflag);
    }
}

void DeleteChannel (Chan *chan)
{
    HASHMAP_ERASE(core_get_chans(), chan->channelname);

    if (eos)
        PartChannel(chan);

    free(chan);
}

void DeleteWchan (Wchan *wchan)
{
    HASHMAP_ERASE(core_get_wchans(), wchan->chname);
    free(wchan);
}

void clear_wchans(void)
{
    struct hashmap_entry *entry, *tmp_entry;
    Member *member, *tmp_member;
    Wchan *wchan;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_wchans(), entry, tmp_entry, wchan) {
        LLIST_FOREACH_ENTRY_SAFE(&wchan->members, member, tmp_member, wchan_head) {
            DeleteMember(member);
        }
        DeleteWchan(wchan);
    }
}

void DeleteLimit(Limit *limit)
{
    limit->chan->active_autolimit = NULL;
    LLIST_REMOVE(&limit->list_head);
    free(limit);
}

void clear_limits(void)
{
    Limit *limit, *tmp_limit;

    LLIST_FOREACH_ENTRY_SAFE(core_get_limits(), limit, tmp_limit, list_head) {
        DeleteLimit(limit);
    }
}

void DeleteTimeban(Timeban *tb)
{
    LLIST_REMOVE(&tb->chan_head);
    LLIST_REMOVE(&tb->core_head);
    free(tb);
}

Cflag *AddUserToChannel (User *user, Chan *chan, int level, int uflags)
{
    Cflag *new_cflag;
    new_cflag = (Cflag *)malloc(sizeof(Cflag));

    new_cflag->chan = chan;
    new_cflag->user = user;
    new_cflag->flags = level;
    new_cflag->automode = CFLAG_AUTO_ON;
    new_cflag->suspended = 0;

    if (uflags)
        new_cflag->uflags = uflags;
    else
        new_cflag->uflags = GetUFlagsFromLevel(level);

    LLIST_INSERT_TAIL(&user->cflags, &new_cflag->user_head);
    LLIST_INSERT_TAIL(&chan->cflags, &new_cflag->chan_head);

    return new_cflag;
}

void DeleteUserFromChannel (User *user, Chan *chan)
{
    Cflag *cflag = find_cflag(chan, user);
    if (!cflag) return;
    DeleteCflag(cflag);
}

void DeleteCflag (Cflag *cflag)
{
    LLIST_REMOVE(&cflag->chan_head);
    LLIST_REMOVE(&cflag->user_head);

    free(cflag);
}

Member *AddUserToWchan (Nick *nptr, Wchan *chan)
{
    Member *member;
    member = (Member *)malloc(sizeof(Member));

    member->wchan = chan;
    member->nick = nptr;
    member->flags = 0;

    LLIST_INSERT_TAIL(&nptr->wchans, &member->nick_head);
    LLIST_INSERT_TAIL(&chan->members, &member->wchan_head);

    Chan *chptr = find_channel(chan->chname);
    if (chptr && chptr->autolimit > 0)
        AddLimit(chptr);

    return member;
}

void DeleteUserFromWchan (Nick *nptr, Wchan *chan)
{
    Member *member = find_member(chan, nptr);
    if (!member) return;

    DeleteMember(member);

    if (!member_exists(chan)) DeleteWchan(chan);

    Chan *chptr = find_channel(chan->chname);
    if (chptr && chptr->autolimit > 0)
        AddLimit(chptr);
}

void DeleteMember (Member *member)
{
    LLIST_REMOVE(&member->nick_head);
    LLIST_REMOVE(&member->wchan_head);
    free(member);
}

void KickUser (const char *who, const char *nick, const char *chan, const char *reason, ...)
{
    Nick *nptr;
    char tmp[1024];
    va_list val;
    bzero(tmp, 1024);
    ircsprintf(tmp, 1023, reason, val);

    nptr = find_nick(nick);
    if (!nptr) return;

    if (IsNokick(nptr)) return;

    SendRaw(":%s KICK %s %s :%s",who,chan,nick,tmp);

    Wchan *wchan = find_wchan(chan);
    if (wchan)
        DeleteUserFromWchan(nptr,wchan);
}

void JoinChannel(const char *who, const char *name)
{
    Chan *chptr = find_channel(name);
    if (!chptr) return;
    if (chptr->mlock[0] != '\0')
        SendRaw(":%s MODE %s %s", who, name, chptr->mlock);
    if (!Strcmp(channel_botname(chptr),me.nick) && HasOption(chptr, COPT_NOJOIN))
        return;

    SendRaw(":%s JOIN %s",who,name);
    SendRaw(":%s MODE %s +ao %s %s",who,name,who,who);
}

Member *find_member(const Wchan *wchan, const Nick *nptr)
{
    Member *tmp;

    LLIST_FOREACH_ENTRY(&wchan->members, tmp, wchan_head) {
        if (tmp->nick == nptr)
            return tmp;
    }

    return NULL;
}

void SetStatus (Nick *nptr, const char *chan, long int flag, int what, const char *who)
{
    Member *member;
    Wchan *wchan;

    if (!nptr) return;
    if ((wchan = find_wchan(chan)) == NULL)
        return;

    char modes[50];
    bzero(modes,50);
    int args=0;
    char targ[1024];
    bzero(targ,1024);
    int i;

    if (flag & CHFL_OWNER) {
        strcat(modes,"q");
        args++;
    }

    if (flag & CHFL_PROTECT) {
        strcat(modes,"a");
        args++;
    }

    if (flag & CHFL_OP) {
        strcat(modes,"o");
        args++;
    }

    if (flag & CHFL_HALFOP) {
        strcat(modes,"h");
        args++;
    }

    if (flag & CHFL_VOICE) {
        strcat(modes,"v");
        args++;
    }

    for (i=0;i<args;i++) {
        strcat(targ,nptr->nick);
        strcat(targ," ");
    }

    member = find_member(wchan, nptr);
    if (!member) return;

    if (!what) {
        SendRaw(":%s MODE %s -%s %s",who,chan,modes,targ);
        ClearChanFlag(member, flag);
    } else {
        SendRaw(":%s MODE %s +%s %s",who,chan,modes,targ);
        SetChanFlag(member, flag);
    }
}

void DeleteUserFromChannels (User *user)
{
    Cflag *cflag, *tmp_cflag;

    LLIST_FOREACH_ENTRY_SAFE(&user->cflags, cflag, tmp_cflag, user_head) {
        DeleteCflag(cflag);
    }
}

void DeleteUserFromWchans (Nick *nptr)
{
    Member *member, *tmp_member;

    LLIST_FOREACH_ENTRY_SAFE(&nptr->wchans, member, tmp_member, nick_head) {
        DeleteMember(member);
        if (!member_exists(member->wchan))
            DeleteWchan(member->wchan);
    }
}

bool member_exists (Wchan *chan)
{
    return !LLIST_EMPTY(&chan->members);
}

int members_num (Wchan *wchan)
{
    int count = 0;
    Member *tmp;

    LLIST_FOREACH_ENTRY(&wchan->members, tmp, wchan_head) {
            count++;
    }

    return count;
}

int IsAclOnChan (Chan *chptr)
{
    Wchan *wchan;
    Member *member;
    User *uptr;
    Cflag *cflag;

    if ((wchan = find_wchan(chptr->channelname)) == NULL)
        return 0;

    LLIST_FOREACH_ENTRY(&wchan->members, member, wchan_head) {
        if ((uptr = find_user(member->nick->nick)) == NULL)
            continue;
        if (!IsAuthed(uptr))
            continue;
        if (HasOption(chptr, COPT_AXXFLAGS)) {
            if ((cflag = find_cflag(chptr, uptr)) == NULL)
                continue;
            if (cflag->uflags & (UFLAG_NOOP | UFLAG_AUTOKICK | UFLAG_AUTOKICKBAN))
                continue;
            return 1;
        } else {
            if (GetFlag(uptr, chptr) > 0)
                return 1;
        }
    }

    return 0;
}

void checkexpired()
{
    User *uptr;
    Chan *chptr;
    struct hashmap_entry *entry, *tmp_entry;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_users(), entry, tmp_entry, uptr) {
        if (((time(NULL) - uptr->lastseen) >= 60*60*24*me.nick_expire) && uptr->authed != 1
                && uptr->level < me.level_oper && !(IsUserNoexpire(uptr)))
            userdrop(uptr);
    }

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_chans(), entry, tmp_entry, chptr) {
        if (((time(NULL) - chptr->lastseen) >= 60*60*24*me.chan_expire) && !(IsChanNoexpire(chptr)) && !(IsAclOnChan(chptr)))
            chandrop(chptr);
    }
}

void CheckLimits()
{
    Limit *limit, *tmp_limit;
    Wchan *wchan;
    Chan *chptr;

    LLIST_FOREACH_ENTRY_SAFE(core_get_limits(), limit, tmp_limit, list_head) {
        if (time(NULL) >= limit->time) {
            wchan = find_wchan(limit->chan->channelname);
            if (!wchan) {
                DeleteLimit(limit);
                continue;
            }
            chptr = limit->chan;
            int ag = HasOption(chptr, COPT_NOJOIN) ? 0 : 1;
            SendRaw(":%s MODE %s +l %d",channel_botname(limit->chan),limit->chan->channelname,members_num(wchan)+chptr->autolimit+ag);
            DeleteLimit(limit);
        }
    }
}

int chansreg (char *nick)
{
    int count = 0;
    Chan *chan;
    struct hashmap_entry *entry;

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_chans(), entry, chan) {
        if (!Strcmp(chan->owner,nick))
            count++;
    }

    return count;
}

const char *channel_botname(const Chan *chan)
{
    if (chan->chanbot != NULL)
        return chan->chanbot->nick;

    return me.nick;
}

void joinallchans()
{
    struct hashmap_entry *entry;
    Chan *chptr;
    Wchan *wchan;

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_chans(), entry, chptr) {
        wchan = find_wchan(chptr->channelname);
        if (!HasOption(chptr, COPT_NOJOIN) || chptr->chanbot != NULL)
            JoinChannel(channel_botname(chptr), chptr->channelname);

        if (chptr->mlock[0] != '\0')
            SendRaw(":%s MODE %s %s",channel_botname(chptr),chptr->channelname,chptr->mlock);
        if (chptr->topic[0] != '\0' && (wchan && strcmp(chptr->topic, wchan->topic)))
            SendRaw(":%s TOPIC %s :%s", channel_botname(chptr), chptr->channelname, chptr->topic);
    }
}

void chandrop (Chan *chptr)
{
    if (eos)
        PartChannel(chptr);
    DeleteUsersFromChannel(chptr);
    DeleteChannel(chptr);
}

void CheckTimebans()
{
    Timeban *tb, *tmp_tb;

    LLIST_FOREACH_ENTRY_SAFE(core_get_timebans(), tb, tmp_tb, core_head) {
        if (tb->duration == -1 || time(NULL) < tb->setat + tb->duration)
            continue;

        SendRaw(":%s MODE %s -b %s", channel_botname(tb->chan), tb->chan->channelname, tb->mask);
        DeleteTimeban(tb);
    }
}

void acl_resync(Chan *chptr)
{
    Cflag *cflag;
    Member *member;
    Nick *nptr;
    User *uptr;
    Wchan *wchan;

    if (chptr->options & COPT_NOAUTO)
        return;

    if ((wchan = find_wchan(chptr->channelname)) == NULL)
        return;

    LLIST_FOREACH_ENTRY(&wchan->members, member, wchan_head) {
        nptr = member->nick;
        if ((uptr = find_user(nptr->nick)) == NULL)
            continue;

        cflag = find_cflag(chptr, uptr);
        if (!cflag || uptr->authed == 0 || (cflag && cflag->suspended == 1)) {
            SetStatus(nptr, chptr->channelname, member->flags, 0, channel_botname(chptr));
            continue;
        }

        sync_cflag(cflag);
    }
}

int parse_uflags (char *flags)
{
    unsigned int i, uflags = 0;
    for (i = 0; i < strlen(flags); i++) {
        switch(flags[i]) {
            case 'o':
                uflags |= UFLAG_OP;
                break;
            case 'v':
                uflags |= UFLAG_VOICE;
                break;
            case 'O':
                uflags |= UFLAG_AUTOOP;
                break;
            case 'V':
                uflags |= UFLAG_AUTOVOICE;
                break;
            case 'x':
                uflags |= UFLAG_ACL;
                break;
            case 'i':
                uflags |= UFLAG_INVITE;
                break;
            case 't':
                uflags |= UFLAG_TOPIC;
                break;
            case 'F':
                uflags |= UFLAG_OWNER;
                break;
            case 'f':
                uflags |= UFLAG_COOWNER;
                break;
            case 'h':
                uflags |= UFLAG_HALFOP;
                break;
            case 's':
                uflags |= UFLAG_SET;
                break;
            case 'H':
                uflags |= UFLAG_AUTOHALFOP;
                break;
            case 'p':
                uflags |= UFLAG_PROTECT;
                break;
            case 'P':
                uflags |= UFLAG_AUTOPROTECT;
                break;
            case 'N':
                uflags |= UFLAG_NOOP;
                break;
            case 'k':
                uflags |= UFLAG_AUTOKICK;
                break;
            case 'b':
                uflags |= UFLAG_AUTOKICKBAN;
                break;
            case 'w':
                uflags |= UFLAG_AUTOOWNER;
                break;
        }
    }

    return uflags;
}

int GetUFlagsFromLevel (int level)
{
    if (level == CHLEV_OWNER) return UFLAG_CHANOWNER;
    else if (level == CHLEV_COOWNER) return UFLAG_CHANCOOWNER;
    else if (level < CHLEV_COOWNER && level >= me.chlev_sadmin) return UFLAG_CHANSADMIN;
    else if (level < me.chlev_sadmin && level >= me.chlev_admin) return UFLAG_CHANADMIN;
    else if (level < me.chlev_admin && level >= me.chlev_op) return UFLAG_CHANOP;
    else if (level < me.chlev_op && level >= me.chlev_halfop) return UFLAG_CHANHALFOP;
    else if (level < me.chlev_halfop && level >= me.chlev_voice) return UFLAG_CHANVOICE;
    else if (level < me.chlev_voice && level >= me.chlev_invite) return UFLAG_INVITE;
    else if (level < 0 && level >= me.chlev_nostatus) return UFLAG_NOOP;
    else if (level < me.chlev_nostatus && level >= me.chlev_akick) return UFLAG_AUTOKICK;
    else return UFLAG_AUTOKICKBAN;
}

int IsFounder (User *uptr, Chan *chptr)
{
    Cflag *cflag;
    Nick *nptr;

    if (!uptr || !chptr)
        return 0;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return IsFounder(get_link_master(uptr), chptr);

    if ((nptr = find_nick(uptr->nick)) != NULL) {
        if (HasUmode(nptr, UMODE_SUPERADMIN))
            return 1;
    }

    if (HasOption(chptr, COPT_AXXFLAGS)) {
        if (cflag->uflags & UFLAG_OWNER || cflag->uflags & UFLAG_COOWNER)
            return 1;
        else
            return 0;
    }

    if (cflag->flags == CHLEV_OWNER || cflag->flags == CHLEV_COOWNER)
        return 1;

    return 0;
}

int IsTrueOwner (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return IsTrueOwner(get_link_master(uptr), chptr);

    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_OWNER);

    return (cflag->flags == CHLEV_OWNER);
}

int ChannelCanProtect (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanProtect(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;

    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_PROTECT || IsFounder(uptr, chptr));

    return (cflag->flags >= me.chlev_admin);
}

int ChannelCanOp (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanOp(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;

    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_OP || IsFounder(uptr, chptr));

    return (cflag->flags >= me.chlev_op);
}

int ChannelCanHalfop (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanHalfop(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;

    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_HALFOP || cflag->uflags & UFLAG_OP || IsFounder(uptr, chptr));

    return (cflag->flags >= me.chlev_halfop);
}

int ChannelCanVoice (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanVoice(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;
    
    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_VOICE || cflag->uflags & UFLAG_HALFOP || cflag->uflags & UFLAG_OP || IsFounder(uptr, chptr));

    return (cflag->flags >= me.chlev_voice);
}

int ChannelCanInvite (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanInvite(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;
    
    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_INVITE || IsFounder(uptr, chptr));

    return (cflag->flags >= me.chlev_invite);
}

int ChannelCanSet (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanSet(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;
    
    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_SET || IsFounder(uptr, chptr));

    return (cflag->flags >= me.chlev_sadmin);
}

int ChannelCanTopic (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanTopic(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;
    
    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_TOPIC || IsFounder(uptr, chptr));

    return (cflag->flags >= me.chlev_op);
}

int ChannelCanACL (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanACL(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;

    if (HasOption(chptr, COPT_AXXFLAGS))
        return (cflag->uflags & UFLAG_ACL || IsFounder(uptr, chptr));

    return (cflag->flags >= me.chlev_admin);
}

int ChannelCanReadACL (User *uptr, Chan *chptr)
{
    Cflag *cflag;

    if (!uptr || !chptr)
        return 0;

    if (IsSuperAdmin(uptr))
        return 1;

    if (IsAuthed(uptr) && uptr->level >= me.level_oper)
        return 1;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return ChannelCanReadACL(get_link_master(uptr), chptr);

    if (cflag->suspended == 1)
        return 0;

    if (cflag->uflags != 0 && !(cflag->uflags & UFLAG_AUTOKICK || cflag->uflags & UFLAG_AUTOKICKBAN ||
        cflag->uflags & UFLAG_NOOP))
        return 1;

    if (IsFounder(uptr, chptr))
        return 1;

    return 0;
}

int ChannelCanWriteACL (User *source, User *target, Chan *chptr)
{
    Cflag *cflag1, *cflag2;

    if (!source || !target || !chptr)
        return 0;

    if (IsSuperAdmin(source))
        return 1;

    if ((cflag1 = find_cflag(chptr, source)) == NULL)
        return ChannelCanWriteACL(get_link_master(source), target, chptr);

    if (cflag1->suspended == 1)
        return 0;

    if (IsFounder(source, chptr))
        return 1;

    if (!(cflag1->uflags & UFLAG_ACL))
        return 0;

    if ((cflag2 = find_cflag(chptr, target)) == NULL)
        return 1;

    if (cflag1->uflags & UFLAG_OWNER)
        return 1;

    if (cflag1->uflags & UFLAG_COOWNER && !(cflag2->uflags & UFLAG_OWNER))
        return 1;

    if (cflag1->uflags & UFLAG_PROTECT && !(cflag2->uflags & UFLAG_OWNER || cflag2->uflags & UFLAG_COOWNER || cflag2->uflags & UFLAG_PROTECT))
        return 1;

    if (!(cflag1->uflags & UFLAG_OWNER || cflag1->uflags & UFLAG_COOWNER || cflag1->uflags & UFLAG_PROTECT) && !(cflag2->uflags & UFLAG_OWNER || cflag2->uflags & UFLAG_COOWNER ||cflag2->uflags & UFLAG_PROTECT))
        return 1;

    if (cflag1 == cflag2)
        return 1;

    return 0;
}

int ChannelCanOverride (User *source, User *target, Chan *chptr)
{
    Cflag *cflag1, *cflag2;

    if (!source || !target || !chptr)
        return 0;

    if (IsSuperAdmin(source))
        return 1;

    if (!IsAuthed(target))
        return 1;

    if ((cflag1 = find_cflag(chptr, source)) == NULL)
        return ChannelCanOverride(get_link_master(source), target, chptr);

    if ((cflag2 = find_cflag(chptr, target)) == NULL)
        return 1;

    if (cflag1->suspended == 1)
        return 0;
    if (cflag2->suspended == 1)
        return 1;

    if (IsFounder(source, chptr))
        return 1;

    if (cflag1->uflags & UFLAG_OWNER)
        return 1;
    if (cflag1->uflags & UFLAG_COOWNER && !(cflag2->uflags & UFLAG_OWNER))
        return 1;
    if (cflag1->uflags & UFLAG_PROTECT && !(cflag2->uflags & UFLAG_OWNER || cflag2->uflags & UFLAG_COOWNER || cflag2->uflags & UFLAG_PROTECT))
        return 1;
    if (!(cflag1->uflags & UFLAG_OWNER || cflag1->uflags & UFLAG_COOWNER || cflag1->uflags & UFLAG_PROTECT) && !(cflag2->uflags & UFLAG_OWNER || cflag2->uflags & UFLAG_COOWNER || cflag2->uflags & UFLAG_PROTECT))
        return 1;

    return 0;
}

char *get_uflags_string (int uflags)
{
    char *str = (char *)malloc(64*sizeof(char));
    bzero(str, 64);
    strcpy(str, "+");

    if (uflags & UFLAG_OP) strcat(str, "o");
    if (uflags & UFLAG_AUTOOP) strcat(str, "O");
    if (uflags & UFLAG_VOICE) strcat(str, "v");
    if (uflags & UFLAG_AUTOVOICE) strcat(str, "V");
    if (uflags & UFLAG_ACL) strcat(str, "x");
    if (uflags & UFLAG_INVITE) strcat(str, "i");
    if (uflags & UFLAG_TOPIC) strcat(str, "t");
    if (uflags & UFLAG_OWNER) strcat(str, "F");
    if (uflags & UFLAG_COOWNER) strcat(str, "f");
    if (uflags & UFLAG_HALFOP) strcat(str, "h");
    if (uflags & UFLAG_SET) strcat(str, "s");
    if (uflags & UFLAG_AUTOHALFOP) strcat(str, "H");
    if (uflags & UFLAG_PROTECT) strcat(str, "p");
    if (uflags & UFLAG_AUTOPROTECT) strcat(str, "P");
    if (uflags & UFLAG_NOOP) strcat(str, "N");
    if (uflags & UFLAG_AUTOKICK) strcat(str, "k");
    if (uflags & UFLAG_AUTOKICKBAN) strcat(str, "b");
    if (uflags & UFLAG_AUTOOWNER) strcat(str, "w");

    return str;
}

int can_modify_uflag (User *uptr, Chan *chptr, int uflag)
{
    /* return 2: can modify uflag for anyone
     * return 1: can modify uflag for itself
     * return 0: cannot modify uflag
     */

    Cflag *cflag;

    if (IsSuperAdmin(uptr))
        return 2;

    if ((cflag = find_cflag(chptr, uptr)) == NULL)
        return 0;

    /*
     * Here we assume that ChannelCanWriteACL() has been checked
     */

    if (uflag & UFLAG_OWNER)
        return 0;

    if (uflag & (UFLAG_COOWNER | UFLAG_AUTOOWNER)) {
        if (cflag->uflags & UFLAG_OWNER) return 2;
        if (cflag->uflags & UFLAG_COOWNER) return 1;
        return 0;
    }

    if (uflag & UFLAG_PROTECT) {
        if (cflag->uflags & (UFLAG_OWNER | UFLAG_COOWNER)) return 2;
        return 0;
    }

    if (uflag & UFLAG_AUTOPROTECT) {
        if (cflag->uflags & (UFLAG_OWNER | UFLAG_COOWNER)) return 2;
        if (cflag->uflags & UFLAG_PROTECT) return 1;
        return 0;
    }

    if (uflag & UFLAG_ACL) {
        if (cflag->uflags & (UFLAG_OWNER | UFLAG_COOWNER | UFLAG_PROTECT)) return 2;
        return 0;
    }

    if (uflag & UFLAG_SET) {
        if (cflag->uflags & (UFLAG_OWNER | UFLAG_COOWNER | UFLAG_PROTECT)) return 2;
        return 0;
    }

    return 2;
}

User *get_coowner (Chan *chptr)
{
    Cflag *cflag;

    LLIST_FOREACH_ENTRY(&chptr->cflags, cflag, chan_head) {
        if (cflag->uflags & UFLAG_COOWNER || cflag->flags == CHLEV_COOWNER)
            return cflag->user;
    }

    return NULL;
}
