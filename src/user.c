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

User *find_user(char *name)
{
    User *tmp;
    LIST_FOREACH(user_list, tmp, HASH(name)) {
        if (!Strcmp(tmp->nick,name))
            return tmp;
    }

    return NULL;
}

Nick *find_nick(char *name)
{
    Nick *tmp;
    LIST_FOREACH(nick_list, tmp, HASH(name)) {
        if (!Strcmp(tmp->nick,name))
            return tmp;
    }

    return NULL;
}

Guest *find_guest(char *name)
{
    Guest *tmp;
    LIST_FOREACH(guest_list, tmp, HASH(name)) {
        if (!Strcmp(tmp->nick,name))
            return tmp;
    }

    return NULL;
}

/* A nickname has only one master but can have several slaves */

Link *find_link (char *slave)
{
    Link *tmp;
    LIST_FOREACH(link_list, tmp, HASH(slave)) {
        if (!Strcmp(tmp->slave,slave))
            return tmp;
    }

    return NULL;
}

Link *find_link2 (char *master, char *slave)
{
    Link *tmp;
    LIST_FOREACH(link_list, tmp, HASH(slave)) {
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

    LIST_INSERT_HEAD(user_list, new_user, HASH(nick));
    return new_user;
}

Nick *AddNick(char *nick, char *ident, char *host, char *server, char *hiddenhost, long int umodes, char *reshost)
{
    Nick *new_nick;

    new_nick = (Nick *)malloc(sizeof(Nick));

    strncpy(new_nick->nick,nick,NICKLEN);
    strncpy(new_nick->ident,ident,NICKLEN);
    strncpy(new_nick->host,host,HOSTLEN);
    strncpy(new_nick->server,server,HOSTLEN);
    strncpy(new_nick->hiddenhost,hiddenhost,HOSTLEN);
    strncpy(new_nick->reshost,reshost,HOSTLEN);
    new_nick->umodes = umodes;
    new_nick->msgnb = 0;
    new_nick->msgtime = 0;
    new_nick->ignored = 0;
    new_nick->ignoretime = 0;
    new_nick->loginattempts = 0;
    new_nick->lasttry = 0;

    Clone *clone;
    if ((clone = find_clone(reshost)) != NULL)
        clone->count++;
    else {
        clone = (Clone *)malloc(sizeof(Clone));
        strncpy(clone->host, reshost, HOSTLEN);
        clone->count = 1;
        LIST_INSERT_HEAD(clones_list, clone, HASH(reshost));
    }

    LIST_INSERT_HEAD(nick_list, new_nick, HASH(nick));

    return new_nick;
}

Guest *AddGuest (char *nick, int timeout, int nickconn)
{
    Guest *new_guest;
    new_guest = (Guest *)malloc(sizeof(Guest));

    strncpy(new_guest->nick,nick,NICKLEN);
    new_guest->timeout = timeout;
    new_guest->nickconn = nickconn;

    LIST_INSERT_HEAD(guest_list, new_guest, HASH(nick));

    return new_guest;
}

Link *AddLink(char *master, char *slave)
{
    Link *new_link;
    new_link = (Link *)malloc(sizeof(Link));

    strncpy(new_link->master,master,NICKLEN);
    strncpy(new_link->slave,slave,NICKLEN);

    LIST_INSERT_HEAD(link_list, new_link, HASH(slave));
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

inline void DeleteAccount (User *user)
{
    LIST_REMOVE(user_list, user, HASH(user->nick));
    if (find_nick(user->nick) && eos)
        SendRaw("SVSMODE %s -r",user->nick);
    if (HasOption(user, UOPT_PROTECT)) DeleteGuest(user->nick);
    free(user);
}

void DeleteWildNick (Nick *nptr)
{
    Clone *clone;
    LIST_REMOVE(nick_list, nptr, HASH(nptr->nick));
    if ((clone = find_clone(nptr->reshost)) != NULL) {
        clone->count--;
        if (clone->count == 0) {
            LIST_REMOVE(clones_list, clone, HASH(nptr->reshost));
            free(clone);
        }
    }
    free(nptr);
}

void DeleteGuest (char *nick)
{
    Guest *guest = find_guest(nick);
    if (!guest) return;
    LIST_REMOVE(guest_list, guest, HASH(nick));
    free(guest);
}

void DeleteLink (char *slave)
{
    Link *link = find_link(slave);
    if (!link) return;
    LIST_REMOVE(link_list, link, HASH(slave));
    free(link);
}

void DeleteLinks (char *nick)
{
    Link *tmp,*next;
    for (tmp = LIST_HEAD(link_list); tmp; tmp = next) {
        next = LIST_LNEXT(tmp);
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
    Clone *tmp;
    LIST_FOREACH(clones_list, tmp, HASH(host)) {
        if (!Strcmp(tmp->host, host))
            return tmp;
    }

    return NULL;
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
    Guest *guest,*next;
    int gv;

    for (guest = LIST_HEAD(guest_list); guest; guest = next) {
        next = LIST_LNEXT(guest);
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
    Bot *bot;

    LIST_FOREACH_ALL(bot_list, bot)
        fakekill(bot->nick,"Exiting");
}

void loadallfakes()
{
    Bot *bot;

    LIST_FOREACH_ALL(bot_list, bot) {
        fakeuser(bot->nick,bot->ident,bot->host,BOTSERV_UMODES);
        SendRaw("SQLINE %s :Reserved for services",bot->nick);
    }
}

void userdrop (User *uptr)
{
    Chan *chptr, *next;
    User *uptr2;
    for (chptr = LIST_HEAD(chan_list); chptr; chptr = next) {
       next = LIST_LNEXT(chptr);
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
