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


#include "commands.h"
#include "channel.h"
#include "child.h"
#include "modules.h"
#include "net.h"
#include "string_utils.h"
#include "user.h"

#include <stdlib.h>

#define IsDigit(c) (c <= '9' && c >= '0')
#define set_perm(a) perm = (a);

extern chanlist chan_list;
extern int emerg;

char *modname = "secserv";
char *logchan = "#stats";
char *modes = "R";
int maxkills = 50;
int maxglines = 50;
int gline_expire = 60*60*24*7; /* 1 week */

void child_init();
void child_cleanup();
int (check_nick)();
int (reset_vars)();
void secserv_cmds (Nick *, User *, char *);
void secserv_help (Nick *);
int (emergchange)();
int kills,seclev,perm,old_maxclones,glines,rmode;
int set_seclev(int);
int sec_perm(int);

void child_init()
{
    kills = perm = rmode = 0;
    seclev = 1;
    if (emerg) {
        set_seclev(2);
        set_perm(1);
    }
    old_maxclones = me.maxclones;
    AddHook(HOOK_NICKCREATE,&check_nick,"check_nickcreate",modname);
    AddHook(HOOK_NICKCHANGE,&check_nick,"check_nickchange",modname);
    AddHook(HOOK_PING,&reset_vars,"reset_vars",modname);
    AddHook(HOOK_EMERG_ON,&emergchange,"emergchangeon",modname);
    AddHook(HOOK_EMERG_OFF,&emergchange,"emergechangeoff",modname);
    addOperCommand("seclev",secserv_cmds,me.level_admin);
    addHelpOperCommand("seclev",secserv_help,"Manage security level",me.level_admin);
    MsgToChan(logchan,"[SecServ] Hi !");
}

void child_cleanup()
{
    delOperCommand("seclev");
    delHelpOperCommand("seclev");
    MsgToChan(logchan,"[SecServ] Bye :(");
}

static int match_bad_pattern(char *string)
{
    int i;
    for (i=0;i<=NICKLEN && *(string+i+1);i++) {
        if (!IsDigit(*(string+i+1))) break;
    }

    return i;
}

static void set_mode_allchans()
{
    Chan *chan;

    if (rmode) return;

    LIST_FOREACH_ALL(chan_list, chan)
        SendRaw("MODE %s +%s",chan->channelname,modes);
    rmode = 1;
}

static void remove_mode_allchans()
{
    Chan *chan;

    if (!rmode) return;

    LIST_FOREACH_ALL(chan_list, chan)
        SendRaw("MODE %s -%s",chan->channelname,modes);
    rmode = 0;
}

int emergchange()
{
    if (emerg) {
        set_perm(1);
        set_seclev(2);
    } else {
        set_perm(0);
        set_seclev(1);
    }

    return MOD_CONTINUE;
}

int reset_vars()
{
    if (seclev > 1 && !perm) {
        set_seclev(seclev-1);
        MsgToChan(logchan,"Security level decreased (%d -> %d)",seclev+1,seclev);
    }

    if (kills > 0 && !perm) MsgToChan(logchan,"Kills counter reset");
    kills = 0;
    if (glines > 0 && !perm) MsgToChan(logchan,"Glines counter reset");
    glines = 0;
    return MOD_CONTINUE;
}

void secserv_help (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2seclev\2 [level] [perm]");
    NoticeToUser(nptr,"   Level 0:   disable protection");
    NoticeToUser(nptr,"   Level 1:   Kill bad nicks");
    NoticeToUser(nptr,"   Level 2:   Gline bad nicks, set maxclones limit to 2");
    NoticeToUser(nptr,"   Level 3:   Same as level 2 and set mode +%s on all chans",modes);
    NoticeToUser(nptr,"   Level 4:   Kill all new users (this level is NEVER automatically enabled)");
    NoticeToUser(nptr," ");
    NoticeToUser(nptr,"   Perm 0: Automatic increasing and decreasing allowed");
    NoticeToUser(nptr,"   Perm 1: Automatic increasing only");
    NoticeToUser(nptr,"   Perm 2: Permanent level");
}

int set_seclev (int level)
{
    switch(level) {
        case 0:
            if (!seclev)
                return MOD_STOP;

            if (seclev > 2) {
                remove_mode_allchans();
                Global("[Security] Back to normal security level.");
            }
            seclev = 0;
            me.maxclones = old_maxclones;

            MsgToChan(logchan,"Security level \0033disabled\003");
            break;
        case 1:
            if (seclev == 1)
                return MOD_STOP;

            if (seclev > 2) {
                remove_mode_allchans();
                Global("[Security] Back to normal security level.");
            }
            seclev = 1;
            me.maxclones = old_maxclones;

            MsgToChan(logchan,"Security level set to \2%d\2",seclev);
            break;
        case 2:
            if (seclev == 2)
                return MOD_STOP;

            if (seclev > 2) {
                 remove_mode_allchans();
                 Global("[Security] Back to normal security level.");
            }

            seclev = 2;
            me.maxclones = 2;

            MsgToChan(logchan,"Security level set to\0034 %d\3",seclev);
            break;
        case 3:
            if (seclev == 3)
                return MOD_STOP;

            if (seclev == 4)
                Global("[Security] Decreasing security level, new connections now allowed.");
            else
                Global("[Security] Possible clones attack, protecting channels.");
            set_mode_allchans();
            seclev = 3;
            me.maxclones = 2;

            MsgToChan(logchan,"Security level set to\0034 %d\3",seclev);
            break;
        case 4:
            if (seclev == 4)
                return MOD_STOP;

            Global("[Security] Possible huge clones attack, new connections currently denied.");
            seclev = 4;
            me.maxclones = 1;

            MsgToChan(logchan,"[\2\0034CRITICAL\2\3] Security level set to \0034\2%d\2\3",seclev);
            break;
    }
    return 0;
}

void secserv_cmds (Nick *nptr, User *uptr, char *all)
{
    char *arg2,*arg3;
    arg2 = all;
    arg3 = SeperateWord(arg2);
    SeperateWord(arg3);

    if (!arg2 || *arg2 == '\0') {
        NoticeToUser(nptr,"Current security level : \2%d\2. Perm level : %d",seclev,perm);
        return;
    }

    if (emerg) {
        NoticeToUser(nptr,"Cannot change seclev or perm while emergency status enabled");
         return;
    }

    if ((uptr->level < me.level_root || uptr->authed != 1) && atoi(arg2) == 4) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    if (uptr->level < me.level_owner && atoi(arg2) == 4 && perm == 2) {
        NoticeToUser(nptr,"Access denied, only the owner can do that");
        return;
    }

    if (arg3 && *arg3) {
        if (uptr->level < me.level_root && atoi(arg3) == 2) {
            NoticeToUser(nptr,"Access denied");
            return;
        }

        if (uptr->level < me.level_owner && atoi(arg3) == 2 && atoi(arg2) == 4) {
            NoticeToUser(nptr,"Access denied, only the owner can do that");
            return;
        }
        set_perm(atoi(arg3));
    }

    set_seclev(atoi(arg2));
    NoticeToUser(nptr,"Security level set to %d",seclev);
    operlog("%s changed security level to %d and perm to %d",nptr->nick,seclev,perm);
}

int check_nick(Nick *nptr)
{
    if (!nptr) return MOD_CONTINUE;
    char *nick = nptr->nick;
    if (!nick || *nick == '\0') return MOD_CONTINUE;

    /*if (!Strcmp(nptr->server,"research.geeknode.org")) return MOD_CONTINUE;*/

    if (seclev == 4) {
        killuser(nick,"We are undergoing a clones attack. New connections are currently denied, please try again later.",me.nick);
        kills++;
        return MOD_STOP;
    }

    if (match_bad_pattern(nick) >= 3) {
        switch(seclev) {
            case 1:
                killuser(nick,"Your nick may be an evil bot nick, please change it.",me.nick);
                MsgToChan(logchan,"\2User %s!%s@%s killed (Bad nick)\2",nick,nptr->ident,nptr->host);
                kills++;
                if (kills >= maxkills && perm < 2) {
                    MsgToChan(logchan,"\2Maxkills limit exceeded, switching to security level\0034 2\3\2");
                    set_seclev(2);
                }

                return MOD_STOP;
            case 2:
            case 3:
                if (!Strcmp(nptr->host,"127.0.0.1") || !Strcmp(nptr->host,"localhost")) break;
                glineuser(nptr->ident,nptr->host,gline_expire,"Possible clones attack");
                MsgToChan(logchan,"\2User %s!%s@%s \0034G-Lined\003 (Possible clones attack)\2",nick,nptr->ident,nptr->host);
                glines++;
                if (glines >= maxglines && seclev == 2 && perm < 2) {
                    MsgToChan(logchan,"\2Maxglines limit exceeded, switching to security level\0034 3\3\2");
                    set_seclev(3);
                }

                return MOD_STOP;
            default:
                return MOD_CONTINUE;
        }
    }

    return MOD_CONTINUE;
}
