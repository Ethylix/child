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


#include "botserv.h"
#include "channel.h"
#include "child.h"   
#include "commands.h"
#include "core.h"
#include "hashmap.h"
#include "modules.h"
#include "net.h"
#include "server.h"
#include "string_utils.h"
#include "trust.h"
#include "user.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* parseline v2, greetz to wildcat for the idea */

int ParseLine(void)
{
    void *commands[] =
                {
                    "NICK", m_nick,
                    "KICK", m_kick,
                    "JOIN", m_join,
                    "QUIT", m_quit,
                    "KILL", m_kill,
                    "MODE", m_mode,
                    "UMODE2", m_umode,
                    "PART", m_part,
                    "SETHOST",  m_sethost,
                    "SETIDENT", m_setident,
                    "CHGHOST",  m_chghost,
                    "CHGIDENT", m_chgident,
                    "PRIVMSG",  m_privmsg,
                    "TOPIC", m_topic,
                    "UID", m_uid,
                    "SID", m_sid,
                    "SJOIN", m_sjoin,
                };
    void  *senders[] =
                {
                    "PING", m_ping,
                    "PROTOCTL", m_protoctl,
                    "TOPIC", m_stopic,
                    "NETINFO", m_eos,
                    "NICK", m_register_user_v3,
                    "SERVER", m_server,
                    "SQUIT", m_squit,
                };

    char *sender, *command, *tail;
    char *parv[3];

    sender = StripBlanks(indata.currentline);

    if (!sender || *sender == '\0')
        return 0;

    command = SeperateWord(sender);
    tail = SeperateWord(command);

    parv[0] = sender;
    parv[1] = command;
    parv[2] = tail;

    if (get_core()->vv) printf("%s %s %s\n",sender,command,tail);

    if (!command || *command == '\0')
        return 0;

    if (RunHooks(HOOK_RAW,NULL,NULL,NULL,parv) == MOD_STOP) return 0;

    unsigned int i;
    void (*func)();
    for (i=0;i<(sizeof(senders)/(sizeof(char *)))-1;i+=2) {
        if (!Strcmp(sender,senders[i])) {
            func = senders[i+1];
            func(command,tail);
            return 1;
        }   
    }

    for (i=0;i<(sizeof(commands)/(sizeof(char *)))-1;i+=2) {
        if (!Strcmp(command,commands[i])) {
            func = commands[i+1];
            func(sender,tail);
            return 1;
        }
    }

    return 1;
}

void m_chghost (char *sender __unused, char *tail)
{
    char *target,*newhost;
    Nick *nptr;

    target = tail;
    newhost = SeperateWord(target);
    SeperateWord(newhost);

    nptr = find_nick(target);
    if (!nptr) return;
    strncpy(nptr->hiddenhost,newhost,HOSTLEN);
}

void m_chgident (char *sender __unused, char *tail)
{
    char *target,*newident;
    Nick *nptr;

    target = tail;
    newident = SeperateWord(target);
    SeperateWord(newident);

    nptr = find_nick(target);
    if (!nptr) return;
    strncpy(nptr->ident,newident,NICKLEN);
}

void m_eos ()
{
    if (!get_core()->eos)
	    joinallchans();
    get_core()->eos = true;
}

void m_join (char *sender, char *tail)
{
    Nick *nptr;
    User *uptr;
    char *chanjoined;
    Chan *chptr;
    const char *bot;
    Cflag *member;
    Timeban *tb;
    char mask[256], mask2[256];

    sender++;
    chanjoined = tail;
    SeperateWord(chanjoined);

    if (!chanjoined || *chanjoined == '\0')
        return;

    if (*chanjoined == ':')
        chanjoined++;

    nptr = find_nick(sender);
    if (!nptr)
        return;

    uptr = find_user(nptr->nick);

    char chans_copied[1024];
    bzero(chans_copied,1024);
    strncpy(chans_copied,chanjoined,1024);
    char *str_ptr;
    str_ptr = strtok(chans_copied,",");
    Wchan *wchan;
    while (str_ptr) {
        wchan = find_wchan(str_ptr);
        if (!wchan) wchan = CreateWchan(str_ptr);
        AddUserToWchan(nptr,wchan);

        chptr = find_channel(str_ptr);

        char *parv[1];
        parv[0] = str_ptr;
        if (RunHooks(HOOK_PRE_JOIN,nptr,uptr,chptr,parv) == MOD_STOP) { str_ptr = strtok(NULL,","); continue; }

        if (!chptr) {
            str_ptr = strtok(NULL,",");
            continue;
        }

        bot = channel_botname(chptr);

        int hasaccess=0, flagged=0;

        // TODO(target0): improve this (functions etc).
        if (!uptr || !uptr->authed || HasOption(chptr, COPT_NOAUTO))
            goto skip_flags;

        member = find_cflag_recursive(chptr, uptr);
        if (!member)
            goto skip_flags;

        if (HasOption(chptr, COPT_AXXFLAGS)) {
            sync_cflag(member);
            goto skip_flags;
        }

        if (IsFounder(uptr,chptr)) {
            if (member->automode == CFLAG_AUTO_ON || member->automode == CFLAG_AUTO_OP) {
                SetStatus(nptr,str_ptr,CHFL_OWNER|CHFL_OP,1,bot);
                flagged = 1;
            }
            hasaccess = 1;
        } else if (GetFlag(uptr,chptr) >= me.chlev_admin) {
            if (member->automode == CFLAG_AUTO_ON || member->automode == CFLAG_AUTO_OP) {
                SetStatus(nptr,str_ptr,CHFL_PROTECT|CHFL_OP,1,bot);
                flagged = 1;
            }
            hasaccess = 1;
        } else if (GetFlag(uptr,chptr) >= me.chlev_op) {
            if (member->automode == CFLAG_AUTO_ON || member->automode == CFLAG_AUTO_OP) {
                SetStatus(nptr,str_ptr,CHFL_OP,1,bot);
                flagged = 1;
            }
            hasaccess = 1;
        } else if (GetFlag(uptr,chptr) >= me.chlev_halfop) {
            if (member->automode == CFLAG_AUTO_ON || member->automode == CFLAG_AUTO_OP) {
                SetStatus(nptr,str_ptr,CHFL_HALFOP,1,bot);
                flagged = 1;
            }
            hasaccess = 1;
        } else if (GetFlag(uptr,chptr) >= me.chlev_voice) {
            if (member->automode != CFLAG_AUTO_OFF) {
                SetStatus(nptr,str_ptr,CHFL_VOICE,1,bot);
                flagged = 1;
            }
            hasaccess = 1;
        } else if (GetFlag(uptr,chptr) == me.chlev_akick) {
            KickUser(bot,uptr->nick,str_ptr,"Get out of this chan !"); hasaccess = 1;
        } else if (GetFlag(uptr,chptr) == me.chlev_akb) {
            SendRaw(":%s MODE %s +b *!*@%s",bot,str_ptr,nptr->hiddenhost);
            KickUser(bot,uptr->nick,str_ptr,"Get out of this chan !"); hasaccess = 1;
        } else if (GetFlag(uptr,chptr) == me.chlev_nostatus)
            hasaccess = 1;

        if (hasaccess && !flagged && GetFlag(uptr,chptr) >= me.chlev_voice && member->automode != CFLAG_AUTO_OFF) {
            if (member->automode == CFLAG_AUTO_VOICE)
                SetStatus(nptr,str_ptr,CHFL_VOICE,1,bot);
        }

skip_flags:
        if (uptr && uptr->authed == 1) {
            if (IsFounder(uptr,chptr) || GetFlag(uptr,chptr) > 0)
                chptr->lastseen = time(NULL);
        }

        if (!HasOption(chptr, COPT_NOAUTO) && !hasaccess) {
            if ((uptr && uptr->authed == 1 && HasOption(chptr, COPT_SECURE)) || (!HasOption(chptr, COPT_SECURE))) {
                if (HasOption(chptr, COPT_AVOICE))
                    SetStatus(nptr,str_ptr,CHFL_VOICE,1,bot);
                if (HasOption(chptr, COPT_AOP))
                    SetStatus(nptr,str_ptr,CHFL_OP,1,bot);
            }
        }

        if (chptr->entrymsg[0] != '\0')
            FakeNotice(bot,nptr,"[%s] %s",chptr->channelname,chptr->entrymsg);

        RunHooks(HOOK_JOIN,nptr,uptr,chptr,NULL);

        bzero(mask,256);
        bzero(mask2,256);
        snprintf(mask, 256, "%s!%s@%s", nptr->nick, nptr->ident, nptr->hiddenhost);
        snprintf(mask2, 256, "%s!%s@%s", nptr->nick, nptr->ident, nptr->host);
        LLIST_FOREACH_ENTRY(&chptr->timebans, tb, chan_head) {
            if (match_mask(tb->mask, mask) || match_mask(tb->mask, mask2)) {
                SendRaw(":%s MODE %s +b *!*@%s", bot, str_ptr, nptr->hiddenhost);
                KickUser(bot, nptr->nick, str_ptr, "%s", tb->reason);
                break;
            }
        }

        str_ptr = strtok(NULL,",");
    }
}

void m_kick (char *sender, char *tail)
{
    char *chan;
    char *nick;
    Chan *chptr;

    chan = tail;
    nick = SeperateWord(chan);
    SeperateWord(nick);

    sender++;

    Nick *nptr;
    Wchan *wchan;

    wchan = find_wchan(chan);
    if (!wchan) return;

    if ((chptr = find_channel(chan)) != NULL) {
        const char *bot = channel_botname(chptr);

        if (Strcmp(nick, bot))
            goto skip_rejoin;

        JoinChannel(bot, chan);
        KickUser(bot,sender,chan,"are you mad ?");
        nptr = find_nick(sender);
        if (!nptr) return;
        DeleteUserFromWchan(nptr,wchan);
        return;
    }

skip_rejoin:
    nptr = find_nick(nick);
    if (!nptr) return;

    DeleteUserFromWchan(nptr,wchan);
}

void m_kill (char *sender, char *tail)
{
    struct hashmap_entry *entry;
    char *nick;
    Chan *chptr;
    Nick *nptr;
    User *uptr;
    Bot *bot;

    sender++;
    nick = tail;
    SeperateWord(nick);

    if (!nick || *nick == '\0')
        return;

    nptr = find_nick(nick);

    if (!nptr) {
        if (!Strcmp(nick, me.nick)) {
            generate_uid(me.uid);
            fakeuser(me.nick, me.ident, me.host, me.uid, MY_UMODES);
        } else if ((bot = find_bot(nick)) != NULL) {
            generate_uid(bot->uid);
            fakeuser(bot->nick, bot->ident, bot->host, bot->uid, BOTSERV_UMODES);
        } else {
            return;
        }

        HASHMAP_FOREACH_ENTRY_VALUE(core_get_chans(), entry, chptr) {
            if (Strcmp(channel_botname(chptr), nick))
                continue;

            if (!HasOption(chptr, COPT_NOJOIN) || chptr->chanbot != NULL)
                JoinChannel(channel_botname(chptr), chptr->channelname);
        }
        killuser(sender,"That is something not recommended...",me.nick);
        return;
    }

    uptr = find_user(nick);
    if (RunHooks(HOOK_KILL,nptr,uptr,NULL,NULL) == MOD_STOP)
        return;

    if (!nptr) return;

    userquit(nick);
}

void m_umode (char *sender, char *tail)
{
    char *nick;
    char *umode;
    Nick *nptr = NULL;
    User *uptr = NULL;
    char *parv[1];

    nick =  strtok(sender,":");
    umode = tail;
    SeperateWord(umode);

    if (!nick || !umode || *nick == '\0' || *umode == '\0')
	return;

    nptr = find_nick(nick);
    if (!nptr)
	return;

    uptr = find_user(nptr->nick);
    if (umode[0] == '+') {
        if (IsCharInString('o',umode)) {
            SetOper(nptr);
            globops("\2%s\2 is now an IRC Operator",nptr->nick);
        }
        if (IsCharInString('a',umode)) SetSAdmin(nptr);
        if (IsCharInString('A',umode)) SetAdmin(nptr);
        if (IsCharInString('N',umode)) SetNAdmin(nptr);
        if (IsCharInString('B',umode)) SetBot(nptr);
        if (IsCharInString('S',umode)) SetService(nptr);
        if (IsCharInString('q',umode)) SetNokick(nptr);
    } else if (umode[0] == '-') {
        if (IsCharInString('o',umode)) ClearOper(nptr);
        if (IsCharInString('a',umode)) ClearSAdmin(nptr);
        if (IsCharInString('A',umode)) ClearAdmin(nptr);
        if (IsCharInString('N',umode)) ClearNAdmin(nptr);
        if (IsCharInString('B',umode)) ClearBot(nptr);
        if (IsCharInString('S',umode)) ClearService(nptr);
        if (IsCharInString('q',umode)) ClearNokick(nptr);
        if (IsCharInString('x',umode)) strncpy(nptr->hiddenhost,nptr->host,HOSTLEN);
    }

    parv[0] = umode;
    RunHooks(HOOK_UMODE,nptr,uptr,NULL,parv);
}

void m_mode (char *sender, char *tail)
{
    char *nick,*chan;
    char *umode,*modes;
    Nick *nptr,*nptr2=NULL;
    char *parv[1];
    char *args[50];
    User *uptr = NULL;
    Member *member = NULL;
    Wchan *wchan;
    Chan *chptr;
    int i=0;

    /* This basically checks if the MODE command is applied to a user.
     * In Unreal4, this is done via the UMODE2 command so it *should*
     * be safe to remove this part of code. However, I'm not sure that
     * a 3.2 leaf wouldn't still send a MODE command, so I'm leaving it there
     * for the moment(?)... -fab
     */
    if (strstr(tail,":")) {
        nick = tail;
        umode = SeperateWord(nick);
        SeperateWord(umode);

        if (!nick || !umode || *nick == '\0' || *umode == '\0')
            return;

        if (nick[0] == '#')
            return;

        nptr = find_nick(nick);
        if (!nptr)
            return;

        uptr = find_user(nptr->nick);
        umode++;
        if (umode[0] == '+') {
            if (IsCharInString('o',umode)) {
                SetOper(nptr);
                globops("\2%s\2 is now an IRC Operator",nptr->nick);
            }
            if (IsCharInString('a',umode)) SetSAdmin(nptr);
            if (IsCharInString('A',umode)) SetAdmin(nptr);
            if (IsCharInString('N',umode)) SetNAdmin(nptr);
            if (IsCharInString('B',umode)) SetBot(nptr);
            if (IsCharInString('S',umode)) SetService(nptr);
            if (IsCharInString('q',umode)) SetNokick(nptr);
        } else if (umode[0] == '-') {
            if (IsCharInString('o',umode)) ClearOper(nptr);
            if (IsCharInString('a',umode)) ClearSAdmin(nptr);
            if (IsCharInString('A',umode)) ClearAdmin(nptr);
            if (IsCharInString('N',umode)) ClearNAdmin(nptr);
            if (IsCharInString('B',umode)) ClearBot(nptr);
            if (IsCharInString('S',umode)) ClearService(nptr);
            if (IsCharInString('q',umode)) ClearNokick(nptr);
            if (IsCharInString('x',umode)) strncpy(nptr->hiddenhost,nptr->host,HOSTLEN);
        }

        parv[0] = umode;
        RunHooks(HOOK_UMODE,nptr,uptr,NULL,parv);
        return;
    } else {
        chan = tail;
        modes = SeperateWord(chan);
        if (!modes || *modes == '\0') return;

        args[0] = SeperateWord(modes);
        while ((args[i+1] = SeperateWord(args[i]))) i++;

        sender++;

        const char *bot;
        char modesbis[33];
        char *modesm,*modesm2,*modesm3 = NULL,*modesm4,*modesm5;
        char modesbis2[33];
        strncpy(modesbis,modes,32);

        wchan = find_wchan(chan);
        if (!wchan) return;
        chptr = find_channel(chan);

        /* Let's check for mlocks */

        if (chptr) {
            bot = channel_botname(chptr);
            unsigned int k;
            if (chptr->mlock[0] != '\0') {
                strncpy(modesbis2,chptr->mlock,32);
                switch(*modesbis) {
                    case '+':
                        modesm = strstr(modesbis,"-");
                        if (modesm) *modesm = '\0';

                        modesm2 = strstr(modesbis2,"-");
                        if (!modesm2) goto nextp;
                        modesm2++;

                        modesm3 = strstr(modesm2,"+");
                        if (modesm3) *modesm3 = '\0';

                        for (k=0;k<strlen(modesbis);k++) {
                            if (IsCharInString(*(modesbis+k),modesm2))
                                SendRaw(":%s MODE %s %s",bot,chptr->channelname,chptr->mlock);
                        }

                        nextp:
                        if (modesm) modesm++;
                        if (!modesm || *modesm == '\0') break;

                        if (modesm3 && *modesm3 == '\0') *modesm3 = '+';

                        modesm4 = strstr(modesbis2,"+");
                        if (!modesm4) break;
                        modesm4++;

                        modesm5 = strstr(modesm4,"-");
                        if (modesm5) *modesm5 = '\0';

                        for (k=0;k<strlen(modesm);k++) {
                            if (IsCharInString(*(modesm+k),modesm4))
                                SendRaw(":%s MODE %s %s",bot,chptr->channelname,chptr->mlock);
                        }

                        break;
                    case '-':
                        modesm = strstr(modesbis,"+");
                        if (modesm) *modesm = '\0';

                        modesm2 = strstr(modesbis2,"+");
                        if (!modesm2) goto nextm;
                        modesm2++;

                        modesm3 = strstr(modesm2,"-");
                        if (modesm3) *modesm3 = '\0';

                        for (k=0;k<strlen(modesbis);k++) {
                            if (IsCharInString(*(modesbis+k),modesm2))
                                SendRaw(":%s MODE %s %s",bot,chptr->channelname,chptr->mlock);
                        }

                        nextm:
                        if (modesm) modesm++;
                        if (!modesm || *modesm == '\0') break;

                        if (modesm3 && *modesm3 == '\0') *modesm3 = '-';

                        modesm4 = strstr(modesbis2,"-");
                        if (!modesm4) break;
                        modesm4++;

                        modesm5 = strstr(modesm4,"+");
                        if (modesm5) *modesm5 = '\0';

                        for (k=0;k<strlen(modesm);k++) {
                            if (IsCharInString(*(modesm+k),modesm4))
                                SendRaw(":%s MODE %s %s",bot,chptr->channelname,chptr->mlock);
                        }

                        break;
                }
            }
        }

        /* End of mlocks checking */

        bot = NULL;
        if (chptr)
            bot = channel_botname(chptr);

        int warg=0,len;
        switch(*modes) {
            case '+':
                cplus:
                modes++;
                len = strlen(modes);
                for (i=0;*modes && i<=len; i++,modes++) {
                    if (*modes == 'q' || *modes == 'a' || *modes == 'o' || *modes == 'h' || *modes == 'v') {
                        nptr2 = find_nick(args[warg]);
                        if (!nptr2) { warg++; continue; }
                        member = find_member(wchan, nptr2);
                        if (!member) { warg++; continue; }
                        uptr = find_user(nptr2->nick);
                        if (GetFlag(uptr,chptr) == me.chlev_nostatus && IsAuthed(uptr)) {
                            int w = 0;
                            switch (*modes) {
                                case 'q':
                                    w = CHFL_OWNER;
                                    break;
                                case 'a':
                                    w = CHFL_PROTECT;
                                    break;
                                case 'o':
                                    w = CHFL_OP;
                                    break;
                                case 'h':
                                    w = CHFL_HALFOP;
                                    break;
                                case 'v':
                                    w = CHFL_VOICE;
                                    break;
                            }
                            SetStatus(nptr2,chan,w,0,bot);
                            warg++;
                            continue;
                        }
                    }

                    switch(*modes) {
                        case 'q':
                            if (chptr) {
                                if ((!IsFounder(uptr,chptr) || !IsAuthed(uptr)) && HasOption(chptr, COPT_STRICTOP)) {
                                    SetStatus(nptr2,chan,CHFL_OWNER,0,bot);
                                    warg++;
                                    break;
                                }
                            }

                            SetOwner(member);
                            warg++;
                            break;
                        case 'a':
                            if (chptr) {
                                if ((GetFlag(uptr,chptr) < me.chlev_admin || !IsAuthed(uptr)) && HasOption(chptr, COPT_STRICTOP)) {
                                    SetStatus(nptr2,chan,CHFL_PROTECT,0,bot);
                                    warg++;
                                    break;
                                }
                            }

                            SetProtect(member);
                            warg++;
                            break;
                        case 'o':
                            if (chptr) {
                                if ((members_num(wchan) == 1 && (GetFlag(uptr,chptr) < me.chlev_op || !IsAuthed(uptr))) ||
                                    ((GetFlag(uptr,chptr) < me.chlev_op || !IsAuthed(uptr)) &&
                                    chptr->options & HasOption(chptr, COPT_STRICTOP))) {
                                    SetStatus(nptr2,chan,CHFL_OP,0,bot);
                                    warg++;
                                    break;
                                }
                            }

                            SetOp(member);
                            warg++;
                            break;
                        case 'h':
                            if (chptr) {
                                if ((GetFlag(uptr,chptr) < me.chlev_halfop || !IsAuthed(uptr)) && HasOption(chptr, COPT_STRICTOP)) {
                                    SetStatus(nptr2,chan,CHFL_HALFOP,0,bot);
                                    warg++;
                                    break;
                                }
                            }

                            SetHalfop(member);
                            warg++;
                            break;
                        case 'v':
                            SetVoice(member);
                            warg++;
                            break;
                        case '-':
                            goto cmin;
                        case 'b':
                        case 'e':
                        case 'I':
                        case 'f':
                        case 'j':
                        case 'k':
                        case 'l':
                        case 'L':
                            warg++;
                            break;
                    }
                }
                break;
            case '-':
                cmin:
                modes++;
                len = strlen(modes);
                for (i=0; *modes && i<=len; i++,modes++) {
                    if (*modes == 'q' || *modes == 'a' || *modes == 'o' || *modes == 'h' || *modes == 'v') {
                        // TODO(target0): add error handling.
                        nptr2 = find_nick(args[warg]);
                        member = find_member(wchan, nptr2);
                        uptr = find_user(nptr2->nick);
                    }

                    switch(*modes) {
                        case 'q':
                            if (!member) { warg++; break; }
                            if (chptr) {
                                if (Strcmp(uptr->nick, sender) && IsFounder(uptr,chptr) && IsAuthed(uptr) && HasOption(chptr, COPT_PROTECTOPS)) {
                                    SetStatus(nptr2,chan,CHFL_OWNER,1,bot);
                                    warg++;
                                    break;
                                }
                            }

                            ClearOwner(member);
                            warg++;
                            break;
                        case 'a':
                            if (chptr && !Strcmp(channel_botname(chptr), args[warg])) {
                                SendRaw(":%s MODE %s +a %s", channel_botname(chptr), chan, bot);
                                warg++;
                                break;
                            }

                            if (!member) { warg++; break; }
                            if (chptr) {
                                if (Strcmp(uptr->nick, sender) && GetFlag(uptr,chptr) >= me.chlev_admin && IsAuthed(uptr) && HasOption(chptr, COPT_PROTECTOPS)) {
                                    SetStatus(nptr2,chan,CHFL_PROTECT,1,bot);
                                    warg++;
                                    break;
                                }
                            }

                            ClearProtect(member);
                            warg++;
                            break;
                        case 'o':
                            if (chptr && !Strcmp(channel_botname(chptr), args[warg])) {
                                SendRaw(":%s MODE %s +o %s", channel_botname(chptr), chan, bot);
                                warg++;
                                break;
                            }

                            if (!member) { warg++; break; }
                            if (chptr) {
                                if (Strcmp(uptr->nick, sender) && GetFlag(uptr,chptr) >= me.chlev_op && IsAuthed(uptr) && HasOption(chptr, COPT_PROTECTOPS)) {
                                    SetStatus(nptr2,chan,CHFL_OP,1,bot);
                                    warg++;
                                    break;
                                }
                            }

                            ClearOp(member);
                            warg++;
                            break;
                        case 'h':
                            if (!member) { warg++; break; }
                            ClearHalfop(member);
                            warg++;
                            break;
                        case 'v':
                            if (!member) { warg++; break; }
                            ClearVoice(member);
                            warg++;
                            break;
                        case '+':
                            goto cplus;
                        case 'b':
                        case 'e':
                        case 'I':
                        case 'f':
                        case 'j':
                        case 'k':
                        case 'l':
                        case 'L':
                            warg++;
                            break;
                    }
                }
                break;
        }
    }
}

void m_nick (char *sender, char *tail)
{
    char *newnick;
    char oldnick[NICKLEN];
    Nick *nptr;
    User *uptr,*uptr2 = NULL;
    char *parv[1];

    sender++;

    newnick = tail;
    SeperateWord(newnick);

    nptr = find_nick(sender);

    strncpy(oldnick,nptr->nick,NICKLEN - 1);
    oldnick[NICKLEN - 1] = '\0';

    if (!nptr)
        return;

    DeleteGuest(nptr->nick);

    // TODO(target0): handle error cases.
    HASHMAP_ERASE(core_get_nicks(), nptr->nick);

    strncpy(nptr->nick,newnick,NICKLEN - 1);
    nptr->nick[NICKLEN - 1] = '\0';

    // TODO(target0): handle error cases.
    HASHMAP_INSERT(core_get_nicks(), nptr->nick, nptr, NULL);

    uptr = find_user(oldnick);

    if (!Strcmp(newnick,oldnick)) return;

    uptr2 = find_user(newnick);
    if (uptr2) {
        if (uptr && (find_link2(oldnick,newnick) || find_link2(newnick,oldnick)) && uptr->authed == 1) {
            uptr->authed = 0;
            uptr->lastseen = time(NULL);
            uptr2->authed = 1;
            SendRaw("SVSMODE %s +r",newnick);
            uptr2->lastseen = time(NULL);
        } else {
            if (uptr && uptr->authed == 1)
                SendRaw("SVSMODE %s -r",newnick);
            NoticeToUser(nptr,"This nick is registered. Please identify yourself or take another nick.");
            if (HasOption(uptr2,UOPT_PROTECT)) AddGuest(newnick,uptr2->timeout,time(NULL));
        }
    }

    if (uptr) {
        uptr->authed = 0;
        uptr->lastseen = time(NULL);
    }

    nptr = find_nick(sender);
    parv[0] = oldnick;

    RunHooks(HOOK_NICKCHANGE,nptr,uptr2,NULL,parv);
}

void m_part (char *sender, char *tail)
{
    char *chan;
    Nick *nptr;
    Wchan *wchan;

    sender++;
    chan = tail;
    SeperateWord(chan);

    nptr = find_nick(sender);
    if (!nptr) return;
    wchan = find_wchan(chan);
    if (!wchan) return;
    DeleteUserFromWchan(nptr,wchan);
}

void m_ping (char *command)
{
    SendRaw("PONG %s",command+1);
    RunHooks(HOOK_PING,NULL,NULL,NULL,NULL);
}

void m_protoctl(char *command, char *tail)
{
    char *sid = NULL;
    char *str_ptr;

    // TODO(target0): improve this ugly parsing
    if (!strncasecmp(command, "SID=", 4)) {
        sid = strchr(command, '=');
        sid++;
    }

    if (!sid) {
        for (str_ptr = strtok(tail, " "); str_ptr; str_ptr = strtok(NULL, " ")) {
            if (!strncasecmp(str_ptr, "SID=", 4)) {
                sid = strchr(str_ptr, '=');
                sid++;
                break;
            }
        }
    }

    if (sid) {
        strncpy(me.remote_sid, sid, SIDLEN);
        if (*me.remote_server && !find_server(me.remote_server)) {
            if (!add_server(me.remote_server, me.remote_sid, /*hub=*/NULL)) {
                operlog("Failed to create server instance for remote %s (%s)", me.remote_server, me.remote_sid);
            }
        }

        loadallfakes();
        RunHooks(HOOK_CONNECTED,NULL,NULL,NULL,NULL);
    }
}

void m_privmsg (char *sender, char *tail)
{
    char *target,*ch_ptr;
    char gnuh[1024];
    Nick *nptr;
    User *uptr;
    Wchan *wchan=NULL;
    Chan *chptr=NULL;
    char *args[2];

    if (!tail || *tail == '\0')
        return;

    ch_ptr = strchr(tail,':');
    if (!ch_ptr)
        return;
    ch_ptr++;

    sender++;
    target = tail;
    SeperateWord(target);

    strncpy(gnuh,ch_ptr,1024-1);
    gnuh[1024-1] = '\0';

    ch_ptr = StripBlanks(ch_ptr);
    char tmp_[1024];
    strncpy(tmp_,ch_ptr,1024);
    char *arg1;
    arg1 = tmp_;
    SeperateWord(arg1);

    if (!ch_ptr || *ch_ptr == '\0')
        return;

    nptr = find_nick(sender);
    if (!nptr) return;

    if (target[0] != '#' && !IsOper(nptr)) {
        if (nptr->ignored) {
            if (time(NULL) - nptr->ignoretime > me.ignoretime) {
                nptr->ignored = 0;
                nptr->ignoretime = 0;
            } else
                return;
        } else if (nptr->msgtime == 0 || nptr->msgnb == 0) {
            nptr->msgtime = time(NULL);
            nptr->msgnb = 1;
        } else if (time(NULL) - nptr->msgtime <= me.maxmsgtime && nptr->msgnb >= me.maxmsgnb) {
            nptr->ignored = 1;
            nptr->ignoretime = time(NULL);
            return;
        } else if (time(NULL) - nptr->msgtime > me.maxmsgtime && nptr->msgnb <= me.maxmsgnb) {
            nptr->msgtime = time(NULL);
            nptr->msgnb = 1;
        } else
            nptr->msgnb++;
    }

    if (target[0] == '#') {
        chptr = find_channel(target);
        wchan = find_wchan(target);
    }

    uptr = find_user(nptr->nick);

    args[0] = target;
    char parv_tab[1024];
    args[1] = parv_tab;
    bzero(args[1],1024-1);
    strncpy(args[1],gnuh,1024-1);
    if (RunHooks(HOOK_PRE_PRIVMSG,nptr,uptr,chptr,args) == MOD_STOP)
        return;

    Command *cmd;

    if (target[0] == '#') {
        if (!chptr || !wchan) return;
        ch_ptr = SeperateWord(ch_ptr);
        LLIST_FOREACH_ENTRY(core_get_commands(), cmd, list_head) {
            if (!Strcmp(cmd->name,arg1) && cmd->type == CMD_BOT) {
                if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                    cmd->func(nptr,uptr,chptr,wchan,ch_ptr);
                return;
            }
        }
        return;
    }

    char mymask[256];
    snprintf(mymask,256,"%s@%s",me.nick,me.name);

    if (Strcmp(target,me.nick) && Strcmp(target,mymask) && Strcmp(target, me.uid))
        return;

    if (!Strcmp(arg1,"\1VERSION\1")) {
        NoticeToUser(nptr,"\1VERSION Child v%s written by target0, compiled on %s at %s\1",VERSION,__DATE__,__TIME__);
        return;
    }

    LLIST_FOREACH_ENTRY(core_get_commands(), cmd, list_head) {
        if (!Strcmp(cmd->name,arg1) && cmd->type == CMD_BASE) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,ch_ptr);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    if (RunHooks(HOOK_PRIVMSG,nptr,uptr,chptr,args) == MOD_STOP)
        return;

    NoticeToUser(nptr,"Unknown command");
}

void m_quit (char *sender)
{
    Nick *nptr;
    User *uptr;

    sender++;
    nptr = find_nick(sender);
    if (!nptr)
        return;

    uptr = find_user(nptr->nick);

    if (RunHooks(HOOK_QUIT,nptr,uptr,NULL,NULL) == MOD_STOP)
        return;

    userquit(nptr->nick);
}

void m_register_user_v3 (char *command, char *tail)
{
    char *nick;
    char *umode;
    char *ident;
    char *host;
    char *uid;
    char *hiddenhost;
    char *nickip;
    long int modes=0;
    Nick *nptr;

    nick = command;
    ident = tail;
    ident = SeperateWord(ident);
    ident = SeperateWord(ident);
    host = SeperateWord(ident);
    uid = SeperateWord(host);
    umode = SeperateWord(uid);
    umode = SeperateWord(umode);
    hiddenhost = SeperateWord(umode);
    nickip = SeperateWord(hiddenhost);
    SeperateWord(nickip);

    if (!nick || !ident || !host || !umode || *nick == '\0' || *ident == '\0' || *host == '\0'
        || *umode == '\0' || !uid || *uid == '\0'|| !hiddenhost || *hiddenhost == '\0'
        || !nickip || *nickip == '\0') {
        return;
    }

    char clientip[HOSTLEN+1];
    bzero(clientip,HOSTLEN);
    if (*nickip == '*')
        strncpy(clientip,host,HOSTLEN);
    else
        strncpy(clientip,decode_ip(nickip),HOSTLEN);

    Trust *trust;
    int clones;
    trust = find_trust_strict(host);
    if (!trust)
        trust = find_trust(clientip);
    clones = howmanyclones(clientip);
    if (trust) {
        if (clones >= trust->limit) {
            _killuser(nick,"Max clones limit exceeded",me.nick);
            return;
        }
    } else {
        if (clones >= me.maxclones) {
            _killuser(nick,"Max clones limit exceeded",me.nick);
            return;
        }
    }

    if (IsCharInString('o',umode)) modes |= UMODE_OPER;
    if (IsCharInString('a',umode)) modes |= UMODE_SADMIN;
    if (IsCharInString('A',umode)) modes |= UMODE_ADMIN;
    if (IsCharInString('r',umode)) modes |= UMODE_REGISTERED;
    if (IsCharInString('N',umode)) modes |= UMODE_NADMIN;
    if (IsCharInString('B',umode)) modes |= UMODE_BOT;
    if (IsCharInString('S',umode)) modes |= UMODE_SERVICE;
    if (IsCharInString('q',umode)) modes |= UMODE_NOKICK;
    if (IsCharInString('z',umode)) modes |= UMODE_SSL;

    nptr = AddNick(nick,ident,host,uid,hiddenhost,modes,clientip);

    User *uptr;
    uptr = find_user(nptr->nick);
    if (uptr) {
        if (IsRegistered(nptr))
            uptr->authed = 1;
        else {
            uptr->authed = 0;
            if (!IsUserSuspended(uptr)) {
                NoticeToUser(nptr,"This nick is registered. Please identify yourself or take another nick.");
                if (uptr->options & UOPT_PROTECT)
                    AddGuest(nptr->nick,uptr->timeout,time(NULL));
            }
        }
    }

    RunHooks(HOOK_NICKCREATE,nptr,uptr,NULL,NULL);
    return;
}

void m_uid (char *sender, char *tail)
{
    char *nick;
    char *umode;
    char *ident;
    char *host;
    char *uid;
    char *hiddenhost;
    char *nickip;
    long int modes=0;
    Nick *nptr;
    Server *server;

    server = find_server(sender+1);
    if (!server) {
        operlog("Failed to find server entry for sender %s (tail: %s), not registering user", sender+1, tail);
        return;
    }

    nick = tail;
    ident = SeperateWord(nick);
    ident = SeperateWord(ident);
    ident = SeperateWord(ident);
    host = SeperateWord(ident);
    uid = SeperateWord(host);
    umode = SeperateWord(uid);
    umode = SeperateWord(umode);
    hiddenhost = SeperateWord(umode);
    hiddenhost = SeperateWord(hiddenhost);
    nickip = SeperateWord(hiddenhost);
    SeperateWord(nickip);

    if (!nick || !ident || !host || !umode || *nick == '\0' || *ident == '\0' || *host == '\0'
        || *umode == '\0' || !uid || *uid == '\0'|| !hiddenhost || *hiddenhost == '\0'
        || !nickip || *nickip == '\0') {
        return;
    }

    char clientip[HOSTLEN+1];
    bzero(clientip,HOSTLEN);
    if (*nickip == '*')
        strncpy(clientip,host,HOSTLEN);
    else
        strncpy(clientip,decode_ip(nickip),HOSTLEN);

    Trust *trust;
    int clones;
    trust = find_trust_strict(host);
    if (!trust)
        trust = find_trust(clientip);
    clones = howmanyclones(clientip);
    if (trust) {
        if (clones >= trust->limit) {
            _killuser(nick,"Max clones limit exceeded",me.nick);
            return;
        }
    } else {
        if (clones >= me.maxclones) {
            _killuser(nick,"Max clones limit exceeded",me.nick);
            return;
        }
    }

    if (IsCharInString('o',umode)) modes |= UMODE_OPER;
    if (IsCharInString('a',umode)) modes |= UMODE_SADMIN;
    if (IsCharInString('A',umode)) modes |= UMODE_ADMIN;
    if (IsCharInString('r',umode)) modes |= UMODE_REGISTERED;
    if (IsCharInString('N',umode)) modes |= UMODE_NADMIN;
    if (IsCharInString('B',umode)) modes |= UMODE_BOT;
    if (IsCharInString('S',umode)) modes |= UMODE_SERVICE;
    if (IsCharInString('q',umode)) modes |= UMODE_NOKICK;
    if (IsCharInString('z',umode)) modes |= UMODE_SSL;

    nptr = AddNick(nick,ident,host,uid,hiddenhost,modes,clientip);
    LLIST_INSERT_TAIL(&server->nicks, &nptr->server_head);

    User *uptr;
    uptr = find_user(nptr->nick);
    if (uptr) {
        if (IsRegistered(nptr))
            uptr->authed = 1;
        else {
            uptr->authed = 0;
            if (!IsUserSuspended(uptr)) {
                NoticeToUser(nptr,"This nick is registered. Please identify yourself or take another nick.");
                if (uptr->options & UOPT_PROTECT)
                    AddGuest(nptr->nick,uptr->timeout,time(NULL));
            }
        }
    }

    RunHooks(HOOK_NICKCREATE,nptr,uptr,NULL,NULL);
    return;
}

void m_sethost (char *sender, char *tail)
{
    char *newhost;
    Nick *nptr;

    sender++;
    newhost = tail;
    SeperateWord(newhost);

    nptr = find_nick(sender);
    if (!nptr) return;
    strncpy(nptr->hiddenhost,newhost,HOSTLEN);
}

void m_setident (char *sender, char *tail)
{
    char *newident;
    Nick *nptr;

    sender++;
    newident = tail;
    SeperateWord(newident);

    nptr = find_nick(sender);
    if (!nptr) return;
    strncpy(nptr->ident,newident,NICKLEN);
}

void m_topic (char *sender, char *tail)
{
    Chan *chptr;
    char *topic, *chan;

    sender++;
    chan = tail;
    topic = SeperateWord(chan);
    if ((topic = strchr(topic, ':')) == NULL)
        return;

    topic++;
    if ((chptr = find_channel(chan)) == NULL)
        return;

    if (chptr->options & COPT_ENFTOPIC)
        SendRaw(":%s TOPIC %s :%s", channel_botname(chptr), chan, chptr->topic);
    else
        strncpy(chptr->topic, topic, TOPICLEN);
}

void m_stopic (char *command, char *tail)
{
    Wchan *wchan;
    Chan *chptr;
    char *chname, *ts, *topic;
    chname = command;
    ts = SeperateWord(tail);
    if ((topic = strchr(ts, ':')) != NULL)
        topic++;

    wchan = find_wchan(chname); /* assuming not NULL, debugging purpose */
    strncpy(wchan->topic, topic, TOPICLEN);
    if ((chptr = find_channel(chname)) != NULL) {
        if (chptr->topic[0] == '\0')
            strncpy(chptr->topic, topic, TOPICLEN);
    }
}

void m_squit(char *command, char *tail __unused)
{
    Server *server;
    char *sname;

    sname = command;

    server = find_server(sname);
    if (!server) {
        operlog("Cannot find server %s for SQUIT", sname);
        return;
    }

    detach_server_recursive(server);
}

void m_sid(char *sender, char *tail)
{
    Server *sptr;
    char *sname;
    char *sid;

    sname = tail;
    sid = SeperateWord(sname);
    sid = SeperateWord(sid);
    SeperateWord(sid);

    sender++;
    sptr = find_server(sender);
    if (!sptr) {
        operlog("Cannot find source server %s for new server %s (%s)\n", sender, sname, sid);
        return;
    }

    if (find_server(sname)) {
        operlog("Duplicate server name in SID: %s (%s)", sname, sid);
        return;
    }

    if (find_server(sid)) {
        operlog("Duplicate server sid in SID: %s (%s)", sname, sid);
        return;
    }

    if (!add_server(sname, sid, sptr)) {
        operlog("Failed to create new server %s (%s)", sname, sid);
    }
}

void m_server(char *command, char *tail __unused)
{
    strncpy(me.remote_server, command, SERVERNAMELEN);

    if (*me.remote_sid && !find_server(me.remote_sid)) {
        if (!add_server(me.remote_server, me.remote_sid, /*hub=*/NULL)) {
            operlog("Failed to create server instance for remote %s (%s)", me.remote_server, me.remote_sid);
        }
    }
}

void m_sjoin(char *sender, char *tail)
{
    char *chname, *modes, *sjoinbuf;
    char *sjbuf_elem;
    Wchan *wchan;

    chname = tail;
    chname = SeperateWord(chname);
    modes = SeperateWord(chname);

    // Lookup " :" string to find the sjoin buffer. This should work for weird edge cases such as
    // :042 SJOIN 1463647041 #Troll +ntTSf [5j#R2,10m#M2,10t]:2  :050T8AL7X 0504QF86V
    // :042 SJOIN 1470200804 #geekfault :038FZ03TS &*!HS-157@2a00:5884:8369:0:0:0:0:1
    sjoinbuf = strstr(modes, " :");
    if (sjoinbuf) {
        *sjoinbuf++ = 0;
    } else {
        // Modes not present, such as
        // :042 SJOIN 1514128801 #cafai :038F53WGM @~C
        sjoinbuf = strchr(modes, ':');
    }
    *sjoinbuf++ = 0;

    wchan = find_wchan(chname);
    if (!wchan) {
        wchan = CreateWchan(chname);
    }

    // Apply modes to enforce mlock.
    if (modes) {
        char buf[1024];

        snprintf(buf, 1023, "%s %s", chname, modes);
        m_mode(sender, buf);
    }

    // Add all users to the channel.
    for (sjbuf_elem = strtok(sjoinbuf, " "); sjbuf_elem; sjbuf_elem = strtok(NULL, " ")) {
        int flags = 0;
        Member *member;
        Nick *nptr = NULL;
        User *uptr;
        Cflag *cflag;
        Chan *chptr;

        for (; *sjbuf_elem; sjbuf_elem++) {
            switch (*sjbuf_elem) {
            case '*':
                flags |= CHFL_OWNER;
                continue;
            case '~':
                flags |= CHFL_PROTECT;
                continue;
            case '@':
                flags |= CHFL_OP;
                continue;
            case '%':
                flags |= CHFL_HALFOP;
                continue;
            case '+':
                flags |= CHFL_VOICE;
                continue;
            case '&': // +b
            case '"': // +e
            case '\'': // +I
                break;
            default:
                nptr = find_nick(sjbuf_elem);
                if (!nptr) {
                    operlog("Failed to resolve nick/uid %s in SJOIN buffer for chan %s", sjbuf_elem, chname);
                }
            }

            // We got a nick/uid or an unsupported mode, break
            break;
        }

        if (!nptr) {
            continue;
        }

        if (find_member(wchan, nptr) != NULL) {
            operlog("User %s already present in channel %s during SJOIN, ignoring", nptr->nick, chname);
            continue;
        }

        member = AddUserToWchan(nptr, wchan);
        member->flags = flags;

        chptr = find_channel(chname);
        if (!chptr)
            continue;

        uptr = find_user(nptr->nick);
        if (!uptr || !uptr->authed)
            continue;

        cflag = find_cflag_recursive(chptr, uptr);
        if (!cflag)
            continue;

        sync_cflag(cflag);
    }
}
