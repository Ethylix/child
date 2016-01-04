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


#include <globals.h>
#include <child.h>

Chan *find_channel (char *name)
{
    Chan *tmp;
    LIST_FOREACH(chan_list, tmp, HASH(name)) {
        if (!Strcmp(tmp->channelname, name))
            return tmp;
    }

    return NULL;
}

Wchan *find_wchan (char *name)
{
    Wchan *tmp;
    LIST_FOREACH(wchan_list, tmp, HASH(name)) {
        if (!Strcmp(tmp->chname,name))
            return tmp;
    }

    return NULL;
}

Cflag *find_cflag (char *nick, char *name)
{
    Cflag *tmp;
    LIST_FOREACH(cflag_list, tmp, HASH(name)) {
        if (!Strcmp(tmp->channel,name) && !Strcmp(tmp->nick,nick))
            return tmp;
    }

    return NULL;
}

Cflag *find_cflag_r (char *nick, char *chname)
{
    Cflag *cflag;
    Link *l;

    if ((cflag = find_cflag(nick, chname)) == NULL) {
        if ((l = find_link(nick)) == NULL)
            return NULL;
        return find_cflag_r(l->master, chname);
    }

    return cflag;
}

Limit *find_limit (char *channel)
{
    Limit *tmp;
    LIST_FOREACH(limit_list, tmp, HASH(channel)) {
        if (!Strcmp(tmp->channel,channel))
            return tmp;
    }

    return NULL;
}

TB *find_tb (Chan *chptr, char *mask)
{
    TB *tmp;
    LIST_FOREACH(tb_list, tmp, HASH(chptr->channelname)) {
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
    cflag = find_cflag(uptr->nick,chptr->channelname);

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

    LIST_INSERT_HEAD(chan_list, new_chan, HASH(name));

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

    LIST_INSERT_HEAD(wchan_list, new_chan, HASH(name));

    return new_chan;
}

Limit *AddLimit (char *name)
{
    Limit *new_limit;
    Limit *limit;

    limit = find_limit(name);
    if (limit) {
        limit->time = time(NULL)+me.limittime;
        return limit;
    }

    new_limit = (Limit *)malloc(sizeof(Limit));

    strncpy(new_limit->channel,name,CHANLEN);
    new_limit->time = time(NULL)+me.limittime;

    LIST_INSERT_HEAD(limit_list, new_limit, HASH(name));

    return new_limit;
}

TB *AddTB (Chan *chan, char *mask, int duration, char *reason)
{
    TB *new_tb;

    new_tb = (TB *)malloc(sizeof(TB));
    strncpy(new_tb->channel, chan->channelname, CHANLEN);
    strncpy(new_tb->mask, mask, MASKLEN);
    strncpy(new_tb->reason, reason, 256);
    new_tb->duration = duration;
    new_tb->setat = time(NULL);

    LIST_INSERT_HEAD(tb_list, new_tb, HASH(chan->channelname));

    return new_tb;
}

void DeleteUsersFromChannel (Chan *chan)
{
    Cflag *cflag, *next;

    for (cflag = cflag_list.table[HASH(chan->channelname)]; cflag; cflag = next) {
        next = cflag->next;
        if (!Strcmp(cflag->channel,chan->channelname))
            DeleteCflag(cflag);
    }
}

void DeleteChannel (Chan *chan)
{
    Chanbot *chanbot;

    LIST_REMOVE(chan_list, chan, HASH(chan->channelname));

    chanbot = find_chanbot(chan->channelname);
    if (!chanbot) {
        if (eos)
            PartChannel(chan->channelname);
    } else {
        if (eos)
            SendRaw(":%s PART %s",chanbot->bot,chanbot->name);
        delChanbot(chanbot);
    }

    free(chan);
}

void DeleteWchan (Wchan *wchan)
{
    LIST_REMOVE(wchan_list, wchan, HASH(wchan->chname));
    free(wchan);
}

void DeleteLimit (Limit *limit)
{
    LIST_REMOVE(limit_list, limit, HASH(limit->channel));
    free(limit);
}

void DeleteTB (TB *tb)
{
    LIST_REMOVE(tb_list, tb, HASH(tb->channel));
    free(tb);
}

Cflag *AddMaskToChannel (char *mask, Chan *chan, int flags)
{
    Cflag *new_cflag;
    new_cflag = (Cflag *)malloc(sizeof(Cflag));

    strncpy(new_cflag->channel,chan->channelname,CHANLEN);
    strncpy(new_cflag->nick,mask,NICKLEN+MASKLEN);
    new_cflag->flags = flags;
    new_cflag->automode = 1;
    new_cflag->suspended = 0;

    LIST_INSERT_HEAD(cflag_list, new_cflag, HASH(chan->channelname));

    return new_cflag;
}

void DeleteMaskFromChannel (char *mask, Chan *chan)
{
    Cflag *cflag;

    cflag = find_cflag(mask,chan->channelname);
    if (!cflag) return;

    LIST_REMOVE(cflag_list, cflag, HASH(chan->channelname));
    free(cflag);
}

Cflag *AddUserToChannel (User *user, Chan *chan, int level, int uflags)
{
    Cflag *new_cflag;
    new_cflag = (Cflag *)malloc(sizeof(Cflag));

    strncpy(new_cflag->channel,chan->channelname,CHANLEN);
    strncpy(new_cflag->nick,user->nick,NICKLEN);
    new_cflag->flags = level;
    new_cflag->automode = CFLAG_AUTO_ON;
    new_cflag->suspended = 0;

    if (uflags)
        new_cflag->uflags = uflags;
    else
        new_cflag->uflags = GetUFlagsFromLevel(level);

    LIST_INSERT_HEAD(cflag_list, new_cflag, HASH(chan->channelname));

    return new_cflag;
}

void DeleteUserFromChannel (User *user, Chan *chan)
{
    Cflag *cflag = find_cflag(user->nick,chan->channelname);
    if (!cflag) return;
    DeleteCflag(cflag);
}

void DeleteCflag (Cflag *cflag)
{
    LIST_REMOVE(cflag_list, cflag, HASH(cflag->channel));
    free(cflag);
}

Member *AddUserToWchan (Nick *nptr, Wchan *chan)
{
    Member *member;
    member = (Member *)malloc(sizeof(Member));

    strncpy(member->nick,nptr->nick,NICKLEN);
    strncpy(member->channel,chan->chname,CHANLEN);
    member->flags = 0;

    LIST_INSERT_HEAD(member_list, member, HASH(chan->chname));

    Chan *chptr = find_channel(chan->chname);
    if (chptr && chptr->autolimit > 0)
        AddLimit(chan->chname);

    return member;
}

void DeleteUserFromWchan (Nick *nptr, Wchan *chan)
{
    Member *member = find_member(chan->chname,nptr->nick);
    if (!member) return;

    DeleteMember(member);

    if (!member_exists(chan)) DeleteWchan(chan);

    Chan *chptr = find_channel(chan->chname);
    if (chptr && chptr->autolimit > 0)
        AddLimit(chan->chname);
}

void DeleteMember (Member *member)
{
    LIST_REMOVE(member_list, member, HASH(member->channel));
    free(member);
}

void KickUser (char *who, char *nick, char *chan, char *reason, ...)
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

void JoinChannel (char *who, char *name)
{
    Chan *chptr = find_channel(name);
    if (!chptr) return;
    if (chptr->mlock[0] != '\0')
        SendRaw(":%s MODE %s %s", who, name, chptr->mlock);
    if (!Strcmp(whatbot(name),me.nick) && HasOption(chptr, COPT_NOJOIN))
        return;

    SendRaw(":%s JOIN %s",who,name);
    SendRaw(":%s MODE %s +ao %s %s",who,name,who,who);
}

Member *find_member (char *chname, char *name)
{
    Member *tmp;
    LIST_FOREACH(member_list, tmp, HASH(chname)) {
        if (!Strcmp(tmp->nick,name) && !Strcmp(tmp->channel,chname))
            return tmp;
    }

    return NULL;
}

void SetStatus (Nick *nptr, char *chan, long int flag, int what, char *who)
{
    Member *member;

    if (!nptr) return;

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

    member = find_member(chan,nptr->nick);
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
    Cflag *cflag, *next;

    for (cflag = LIST_HEAD(cflag_list); cflag; cflag = next) {
        next = LIST_LNEXT(cflag);
        if (!Strcmp(cflag->nick,user->nick))
            DeleteCflag(cflag);
    }
}

void DeleteUserFromWchans (Nick *nptr)
{
    Member *member, *next;
    Wchan *wchan;

    for (member = LIST_HEAD(member_list); member; member = next) {
        next = LIST_LNEXT(member);
        if (!Strcmp(member->nick,nptr->nick)) {
            wchan = find_wchan(member->channel);
            DeleteMember(member);
            if (wchan) {
                if (!member_exists(wchan))
                    DeleteWchan(wchan);
            }
        }
    }
}

int member_exists (Wchan *chan)
{
    Member *tmp;
    LIST_FOREACH(member_list, tmp, HASH(chan->chname)) {
        if (!Strcmp(tmp->channel,chan->chname))
            return 1;
    }

    return 0;
}

int members_num (Wchan *wchan)
{
    int i=0;
    Member *tmp;
    LIST_FOREACH(member_list, tmp, HASH(wchan->chname)) {
        if (!Strcmp(tmp->channel,wchan->chname))
            i++;
    }

    return i;
}

int IsAclOnChan (Chan *chptr)
{
    Wchan *wchan;
    Member *member;
    User *uptr;
    Cflag *cflag;

    if ((wchan = find_wchan(chptr->channelname)) == NULL)
        return 0;

    LIST_FOREACH(member_list, member, HASH(wchan->chname)) {
        if ((uptr = find_user(member->nick)) == NULL)
            continue;
        if (!IsAuthed(uptr))
            continue;
        if (HasOption(chptr, COPT_AXXFLAGS)) {
            if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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
    User *uptr, *unext;
    Chan *chptr, *cnext;

    for (uptr = LIST_HEAD(user_list); uptr; uptr = unext) {
        unext = LIST_LNEXT(uptr);
        if (((time(NULL) - uptr->lastseen) >= 60*60*24*me.nick_expire) && uptr->authed != 1
                && uptr->level < me.level_oper && !(IsUserNoexpire(uptr)))
            userdrop(uptr);
    }

    for (chptr = LIST_HEAD(chan_list); chptr; chptr = cnext) {
        cnext = LIST_LNEXT(chptr);
        if (((time(NULL) - chptr->lastseen) >= 60*60*24*me.chan_expire) && !(IsChanNoexpire(chptr)) && !(IsAclOnChan(chptr)))
            chandrop(chptr);
    }
}

void CheckLimits()
{
    Limit *limit, *next;
    Wchan *wchan;
    Chan *chptr;

    for (limit = LIST_HEAD(limit_list); limit; limit = next) {
        next = LIST_LNEXT(limit);
        if (time(NULL) >= limit->time) {
            wchan = find_wchan(limit->channel);
            if (!wchan) {
                DeleteLimit(limit);
                continue;
            }
            chptr = find_channel(limit->channel);
            if (!chptr) {
                DeleteLimit(limit);
                continue;
            }
            int ag = HasOption(chptr, COPT_NOJOIN) ? 0 : 1;
            SendRaw(":%s MODE %s +l %d",whatbot(limit->channel),limit->channel,members_num(wchan)+chptr->autolimit+ag);
            DeleteLimit(limit);
        }
    }
}

int IsChanFlag (char *nick, Wchan *wchan, int flag)
{
    Member *member;

    if (!find_nick(nick))
        return 0;

    member = find_member(wchan->chname,nick);
    if (!member)
        return 0;

    if (HasChanFlag(member, flag))
        return 1;

    return 0;
}

int IsMember (char *nick, char *chan)
{
    Member *member;
    LIST_FOREACH(member_list, member, HASH(chan)) {
        if (!Strcmp(member->nick,nick) && !Strcmp(member->channel,chan))
            return 1;
    }

    return 0;
}

int chansreg (char *nick)
{
    int count = 0;
    Chan *chan;
    LIST_FOREACH_ALL(chan_list, chan) {
        if (!Strcmp(chan->owner,nick))
            count++;
    }

    return count;
}

char *whatbot (char *name)
{
    Chanbot *chanbot;
    if ((chanbot = find_chanbot(name)) != NULL)
        return chanbot->bot;
    return me.nick;
}

void joinallchans()
{
    Chanbot *chanbot;
    Chan *chptr;
    Wchan *wchan;

    LIST_FOREACH_ALL(chanbot_list, chanbot)
        JoinChannel(chanbot->bot,chanbot->name);

    LIST_FOREACH_ALL(chan_list, chptr) {
        wchan = find_wchan(chptr->channelname);
        if (!HasOption(chptr, COPT_NOJOIN))
            JoinChannel(me.nick,chptr->channelname);
        if (chptr->mlock[0] != '\0')
            SendRaw(":%s MODE %s %s",whatbot(chptr->channelname),chptr->channelname,chptr->mlock);
        if (chptr->topic[0] != '\0' && (wchan && strcmp(chptr->topic, wchan->topic)))
            SendRaw(":%s TOPIC %s :%s", whatbot(chptr->channelname), chptr->channelname, chptr->topic);
    }
}

inline void chandrop (Chan *chptr)
{
    if (eos)
        PartChannel(chptr->channelname);
    DeleteUsersFromChannel(chptr);
    DeleteChannel(chptr);
}

void CheckTB()
{
    TB *tb, *next;

    for (tb = LIST_HEAD(tb_list); tb; tb = next) {
        next = LIST_LNEXT(tb);
        if (tb->duration == -1 || time(NULL) < tb->setat + tb->duration)
            continue;

        SendRaw(":%s MODE %s -b %s", whatbot(tb->channel), tb->channel, tb->mask);
        DeleteTB(tb);
    }
}

void acl_resync (Chan *chptr)
{
    Cflag *cflag;
    Member *member;
    Nick *nptr;
    User *uptr;

    if (chptr->options & COPT_NOAUTO)
        return;

    LIST_FOREACH(member_list, member, HASH(chptr->channelname)) {
        if (!Strcmp(member->channel, chptr->channelname)) {
            if (((nptr = find_nick(member->nick)) != NULL) && ((uptr = find_user(member->nick)) != NULL)) {
                cflag = find_cflag(member->nick, member->channel);
                if (!cflag || uptr->authed == 0 || (cflag && cflag->suspended == 1)) {
                    SetStatus(nptr, chptr->channelname, member->flags, 0, whatbot(chptr->channelname));
                    continue;
                }

                sync_cflag(cflag, nptr);
            }
        }
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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

    if ((cflag1 = find_cflag(source->nick, chptr->channelname)) == NULL)
        return ChannelCanWriteACL(get_link_master(source), target, chptr);

    if (cflag1->suspended == 1)
        return 0;

    if (IsFounder(source, chptr))
        return 1;

    if (!(cflag1->uflags & UFLAG_ACL))
        return 0;

    if ((cflag2 = find_cflag(target->nick, chptr->channelname)) == NULL)
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

/*    if ((cflag1->uflags & UFLAG_COOWNER || cflag1->uflags & UFLAG_ACL) && !(cflag2->uflags & UFLAG_OWNER || cflag2->uflags & UFLAG_COOWNER))
        return 1;*/

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

    if ((cflag1 = find_cflag(source->nick, chptr->channelname)) == NULL)
        return ChannelCanOverride(get_link_master(source), target, chptr);

    if ((cflag2 = find_cflag(target->nick, chptr->channelname)) == NULL)
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

    if ((cflag = find_cflag(uptr->nick, chptr->channelname)) == NULL)
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
    LIST_FOREACH(cflag_list, cflag, HASH(chptr->channelname)) {
        if (!Strcmp(cflag->channel, chptr->channelname) && (cflag->uflags & UFLAG_COOWNER || cflag->flags == CHLEV_COOWNER))
            return find_user(cflag->nick);
    }

    return NULL;
}
