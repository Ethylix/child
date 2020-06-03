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


#include "botserv.h"
#include "channel.h"
#include "child.h"
#include "commands.h"
#include "core.h"
#include "hashmap.h"
#include "modules.h"
#include "net.h"
#include "string_utils.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

extern cflaglist cflag_list;
extern chanbotlist chanbot_list;
extern commandlist command_list;
extern memberlist member_list;
extern tblist tb_list;

extern int emerg;

void do_chan (Nick *, User *, char *);
void do_help (Nick *, User *, char *);
void chan_register (Nick *, User *, Chan *, char *);
void chan_drop (Nick *, User *, Chan *, char *);
void chan_access (Nick *, User *, Chan *, char *);
void chan_op (Nick *, User *, Chan *, char *);
void chan_deop (Nick *, User *, Chan *, char *);
void chan_voice (Nick *, User *, Chan *, char *);
void chan_halfop (Nick *, User *, Chan *, char *);
void chan_dehalfop (Nick *, User *, Chan *, char *);
void chan_devoice (Nick *, User *, Chan *, char *);
void chan_invite (Nick *, User *, Chan *, char *);
void chan_set (Nick *, User *, Chan *, char *);
void chan_info (Nick *, User *, Chan *, char *);
void chan_entrymsg (Nick *, User *, Chan *, char *);
void chan_unbanall (Nick *, User *, Chan *, char *);
void chan_clearmodes (Nick *, User *, Chan *, char *);
void chan_clearchan (Nick *, User *, Chan *, char *);
void chan_kick (Nick *, User *, Chan *, char *);
void chan_assign (Nick *, User *, Chan *, char *);
void chan_unassign (Nick *, User *, Chan *, char *);
void chan_botlist (Nick *);
void chan_addbot (Nick *, User *, Chan *, char *);
void chan_delbot (Nick *, User *, Chan *, char *);
void chan_set_founder (Nick *, User *, Chan *, char *);
void chan_set_avoice (Nick *, User *, Chan *, char *);
void chan_set_aop (Nick *, User *, Chan *, char *);
void chan_set_nojoin (Nick *, User *, Chan *, char *);
void chan_set_noauto (Nick *, User *, Chan *, char *);
void chan_set_private (Nick *, User *, Chan *, char *);
void chan_set_strictop (Nick *, User *, Chan *, char *);
void chan_set_protectops (Nick *, User *, Chan *, char *);
void chan_set_mlock (Nick *, User *, Chan *, char *);
void chan_set_autolimit (Nick *, User *, Chan *, char *);
void chan_set_secure (Nick *, User *, Chan *, char *);
void chan_set_enablemask (Nick *, User *, Chan *, char *);
void chan_set_mass (Nick *, User *, Chan *, char *);
void chan_set_enftopic (Nick *, User *, Chan *, char *);
void chan_set_axxflags (Nick *, User *, Chan *, char *);
void chan_resync (Nick *, User *, Chan *, char *);
void chan_suspend (Nick *, User *, Chan *, char *);
void chan_unsuspend (Nick *, User *, Chan *, char *);
void chan_clearbans (Nick *, User *, Chan *, char *);
void chan_banlist (Nick *, User *, Chan *, char *);
void chan_topic (Nick *, User *, Chan *, char *);
void chan_flags (Nick *, User *, Chan *, char *);

void child_init()
{
    addBaseCommand("chan",do_chan,0);

    addChanCommand("voice",chan_voice,1);
    addChanCommand("unsuspend",chan_unsuspend,1);
    addChanCommand("unbanall",chan_unbanall,1);
    addChanCommand("unassign",chan_unassign,1);
    addChanCommand("topic",chan_topic,1);
    addChanCommand("suspend",chan_suspend,1);
    addChanCommand("set",chan_set,1);
    addChanCommand("resync",chan_resync,1);
    addChanCommand("register",chan_register,1);
    addChanCommand("op",chan_op,1);
    addChanCommand("kick",chan_kick,1);
    addChanCommand("invite",chan_invite,1);
    addChanCommand("info",chan_info,1);
    addChanCommand("halfop",chan_halfop,1);
    addChanCommand("entrymsg",chan_entrymsg,1);
    addChanCommand("drop",chan_drop,1);
    addChanCommand("devoice",chan_devoice,1);
    addChanCommand("deop",chan_deop,1);
    addChanCommand("delbot",chan_delbot,me.level_oper);
    addChanCommand("dehalfop",chan_dehalfop,1);
    addChanCommand("clearmodes",chan_clearmodes,1);
    addChanCommand("clearchan",chan_clearchan,1);
    addChanCommand("clearbans",chan_clearbans,1);
    addChanCommand("botlist",chan_botlist,1);
    addChanCommand("banlist",chan_banlist,1);
    addChanCommand("assign",chan_assign,1);
    addChanCommand("addbot",chan_addbot,me.level_oper);
    addChanCommand("access",chan_access,1);
    addChanSetCommand("strictop",chan_set_strictop,1);
    addChanSetCommand("secure",chan_set_secure,1);
    addChanSetCommand("protectops",chan_set_protectops,1);
    addChanSetCommand("private",chan_set_private,1);
    addChanSetCommand("nojoin",chan_set_nojoin,1);
    addChanSetCommand("noauto",chan_set_noauto,1);
    addChanSetCommand("mlock",chan_set_mlock,1);
    addChanSetCommand("mass",chan_set_mass,1);
    addChanSetCommand("founder",chan_set_founder,1);
    addChanSetCommand("enftopic",chan_set_enftopic,1);
    addChanSetCommand("enablemask",chan_set_enablemask,1);
    addChanSetCommand("axxflags", chan_set_axxflags, 1);
    addChanSetCommand("avoice",chan_set_avoice,1);
    addChanSetCommand("autolimit",chan_set_autolimit,1);
    addChanSetCommand("aop",chan_set_aop,1);
}

void child_cleanup()
{
    deleteCommand("chan",CMD_BASE,0);

    delChanCommand("register");
    delChanCommand("drop");
    delChanCommand("access");
    delChanCommand("op");
    delChanCommand("deop");
    delChanCommand("voice");
    delChanCommand("devoice");
    delChanCommand("halfop");
    delChanCommand("dehalfop");
    delChanCommand("invite");
    delChanCommand("set");
    delChanCommand("info");
    delChanCommand("entrymsg");
    delChanCommand("clearchan");
    delChanCommand("clearmodes");
    delChanCommand("unbanall");
    delChanCommand("kick");
    delChanCommand("assign");
    delChanCommand("unassign");
    delChanCommand("botlist");
    delChanCommand("addbot");
    delChanCommand("delbot");
    delChanCommand("resync");
    delChanCommand("suspend");
    delChanCommand("unsuspend");
    delChanCommand("clearbans");
    delChanCommand("banlist");
    delChanCommand("topic");
    delChanSetCommand("founder");
    delChanSetCommand("avoice");
    delChanSetCommand("aop");
    delChanSetCommand("nojoin");
    delChanSetCommand("noauto");
    delChanSetCommand("private");
    delChanSetCommand("strictop");
    delChanSetCommand("protectops");
    delChanSetCommand("mlock");
    delChanSetCommand("autolimit");
    delChanSetCommand("secure");
    delChanSetCommand("enablemask");
    delChanSetCommand("mass");
    delChanSetCommand("enftopic");
    delChanSetCommand("axxflags");
}

void do_chan (Nick *nptr, User *uptr, char *all)
{
    if (!nptr) return;

    char *arg1,*arg2,*arg3;
    char blah[1024];
    strncpy(blah,all,1024);
    arg1 = blah;
    arg2 = SeperateWord(arg1);
    arg3 = SeperateWord(arg2);
    SeperateWord(arg3);

    Chan *chptr;
    chptr = find_channel(arg3);

    if (!arg2 || *arg2 == '\0') {
        NoticeToUser(nptr,"Type \2/msg %s help chan\2 for more informations",me.nick);
        return;
    }

    if (!Strcmp(arg2,"help")) {
        do_help(nptr,uptr,all);
        return;
    }

    all = SeperateWord(all);
    all = SeperateWord(all);

    Command *cmd;
    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_CHAN)) {
        if (!Strcmp(cmd->name,arg2) && cmd->type == CMD_CHAN) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,chptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr,"Unknown command");
}

void chan_register (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3;
    arg3 = all;
    SeperateWord(arg3);
    Chan *nchptr;

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN REGISTER \037#channel\037\2");
        return;
    }
        
    if (chptr) {
        NoticeToUser(nptr,"This channel is already registered");
        return;
    }
        
    if (emerg) {
        NoticeToUser(nptr,"Channels registrations are currently disabled. Please try again later.");
        return;
    }

    if (chansreg(nptr->nick) >= me.chanperuser && uptr->level < me.level_oper) {
        NoticeToUser(nptr,"You have registered too many channels (%i max).",me.chanperuser);
        return;
    }

    Wchan *wchan;
    wchan = find_wchan(arg3);
    if (!wchan) {
        NoticeToUser(nptr,"This channel is empty.");
        return;
    }

    if (!IsOp(nptr->nick,wchan)) {
        NoticeToUser(nptr,"You must be a channel operator to register a channel");
        return;
    }

    nchptr = CreateChannel(arg3,uptr->nick,0);
    nchptr->regtime = time(NULL);
    JoinChannel(me.nick,arg3);

    NoticeToUser(nptr,"Channel registered");
}

void chan_drop (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN DROP\2 \37#channel\37");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }
        
    if (!IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
        NoticeToUser(nptr,"Access denied"); 
        return;
    }   
        
    if (!IsFounder(uptr,chptr) && uptr->level >= me.level_oper && IsOper(nptr)) {
        globops("%s used \2DROP\2 on channel %s",nptr->nick,arg3);
        operlog("%s dropped channel %s",nptr->nick,arg3);
    }

    chandrop(chptr);
    NoticeToUser(nptr,"Channel dropped");
}

void __chan_access (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4,*arg5,*arg6;
    User *uptr2;
    Cflag *cflag;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    arg5 = SeperateWord(arg4);
    arg6 = SeperateWord(arg5);
    SeperateWord(arg6);

    if (!arg3 || !arg4 || *arg3 == '\0' || *arg4 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN ACCESS \037#channel\037 {add|auto|del|list} [\037nick\037 [\037level]\037]\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (HasOption(chptr, COPT_AXXFLAGS)) {
        NoticeToUser(nptr, "The \2AXXFLAGS\2 option must be disabled in order to use this command.");
        return;
    }

    if (!Strcmp(arg4,"add")) {
        if (!arg5 || *arg5 == '\0' || !arg6 || *arg6 == '\0') {
            NoticeToUser(nptr,"Syntax: \2CHAN ACCESS \037#channel\037 {add|auto|del|list} [\037nick\037 [\037level\037]]\2");
            return;
        }

        if (GetFlag(uptr,chptr) < me.chlev_admin && !IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
            NoticeToUser(nptr,"Access denied");
            return;
        }

        if (IsMask(arg5) && HasOption(chptr, COPT_ENABLEMASK)) {
            int lev;
            lev = strtol(arg6,NULL,10);
            if (lev > me.chlev_op) {
                NoticeToUser(nptr,"You can't add a mask with a level > 5");
                return;
            }
            DeleteMaskFromChannel(arg5,chptr);
            AddMaskToChannel(arg5,chptr,lev);
            NoticeToUser(nptr,"Mask \2%s\2 added to channel \2%s\2",arg5,arg3);
            return;
        }

        uptr2 = find_user(arg5);
        if (!uptr2) {
            NoticeToUser(nptr,"The nick %s is not registered",arg5);
            return;
        }

        if (GetFlag(uptr2,chptr) >= GetFlag(uptr,chptr) && !IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
            NoticeToUser(nptr,"You cannot modify the access level of someone outranking you");
            return;
        }

        int newlevel = strtol(arg6,NULL,10);
        if (newlevel >= GetFlag(uptr,chptr) && !IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
            NoticeToUser(nptr,"You cannot add a user with a level higher than yours");
            return;
        }

        if (newlevel >= CHLEV_OWNER) {
            NoticeToUser(nptr,"Maximum level is %d, user not added",CHLEV_OWNER-1);
            return;
        }

        if (!Strcmp(chptr->owner,uptr2->nick)) {
            NoticeToUser(nptr,"%s is already the founder of %s",uptr2->nick,chptr->channelname);
            return;
        }

        if (newlevel == CHLEV_COOWNER) {
            LIST_FOREACH(cflag_list, cflag, HASH(chptr->channelname)) {
                if (!Strcmp(cflag->channel, chptr->channelname) && cflag->flags == CHLEV_COOWNER) {
                    NoticeToUser(nptr, "There is already a co-owner for this channel.");
                    return;
                }
            }
        }

        DeleteUserFromChannel(uptr2,chptr);
        AddUserToChannel(uptr2,chptr,newlevel,0);

        NoticeToUser(nptr,"The user \2%s\2 has been added to \2%s\2 with level \2%d\2",arg5,arg3,newlevel);
        return;
    } else if (!Strcmp(arg4,"auto")) {
        if (!arg5 || *arg5 == '\0' || !arg6 || *arg6 == '\0') {
            NoticeToUser(nptr, "Syntax: \2CHAN ACCESS \037#channel\037 {add|auto|del|list} [\037nick\037 [\037level\037]]\2");
            return;
        }
        if (GetFlag(uptr,chptr) < 3 && !IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
            NoticeToUser(nptr,"Access denied");
            return;
        }
        if ((uptr2 = find_user(arg5)) == NULL) {
            NoticeToUser(nptr,"User \2%s\2 does not exist.",arg5);
            return;
        }
        
        if (!Strcmp(arg5,nptr->nick) || (GetFlag(uptr,chptr) >= 10 && GetFlag(uptr2,chptr) < GetFlag(uptr,chptr))) {
            cflag = find_cflag(uptr2->nick, chptr->channelname);
            if (!cflag)
                return;
            if (!Strcmp(arg6, "op"))
                cflag->automode = CFLAG_AUTO_OP;
            else if (!Strcmp(arg6,"voice"))
                cflag->automode = CFLAG_AUTO_VOICE;
            else if (!Strcmp(arg6,"off"))
                cflag->automode = CFLAG_AUTO_OFF;
            else if (!Strcmp(arg6,"default"))
                cflag->automode = CFLAG_AUTO_ON;
            else {
                NoticeToUser(nptr,"Syntax: \2CHAN ACCESS \037#channel\037 auto [\037op\037|\037voice\037|\037default\037|\037off\037]\2");
                return;
            }
            if (GetFlag(uptr2,chptr) < me.chlev_halfop && cflag->automode == CFLAG_AUTO_OP)
                cflag->automode = CFLAG_AUTO_VOICE;
            NoticeToUser(nptr,"Automode successfully set.");
        } else
            NoticeToUser(nptr,"Access denied");
        return;
    } else if (!Strcmp(arg4,"del")) {
        if (!arg5) {
            NoticeToUser(nptr,"Syntax: \2CHAN ACCESS \037#channel\037 {add|auto|del|list} [\037nick\037 [\037level\037]]\2");
            return;
        }

        if (GetFlag(uptr,chptr) && !Strcmp(arg5,nptr->nick)) {
            DeleteUserFromChannel(uptr,chptr);
            NoticeToUser(nptr,"You have been removed from channel %s",arg3);
            return;
        }

        if (GetFlag(uptr,chptr) < me.chlev_admin && !IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
            NoticeToUser(nptr,"Access denied");
            return;
        }

        if (IsMask(arg5) && HasOption(chptr, COPT_ENABLEMASK) && find_cflag(arg5,arg3)) {
            DeleteMaskFromChannel(arg5,chptr);
            NoticeToUser(nptr,"Mask \2%s\2 removed from channel \2%s\2",arg5,arg3);
            return;
        }

        uptr2 = find_user(arg5);
        if (!uptr2) {
            NoticeToUser(nptr,"The user %s is not registered",arg5);
            return;
        }

        if (GetFlag(uptr2,chptr) == 0) {
            NoticeToUser(nptr,"The user %s is not on the access list of %s",arg5,arg3);
            return;
        }

        if (GetFlag(uptr2,chptr) >= GetFlag(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr)) && !IsFounder(uptr,chptr)) {
            NoticeToUser(nptr,"You cannot remove a user outranking you");
            return;
        }

        if (!Strcmp(uptr2->nick,chptr->owner)) {
            NoticeToUser(nptr,"You cannot remove the owner...");
            return;
        }
                
        DeleteUserFromChannel(uptr2,chptr);
        NoticeToUser(nptr,"The user %s has been removed from %s",arg5,arg3);
        return;
    } else if (!Strcmp(arg4,"list")) {
        if (GetFlag(uptr,chptr) < 1 && !IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
            NoticeToUser(nptr,"Access denied");
            return;
        }
            
        NoticeToUser(nptr,"Access list for %s:",arg3);
        NoticeToUser(nptr,"\2Status\2     \2Auto\2       \2Lev\2   \2Nick\2");

        char autom[12];
        char stat[12];
        LIST_FOREACH(cflag_list, cflag, HASH(arg3)) {
            if (!Strcmp(cflag->channel,arg3) && cflag->flags < CHLEV_OWNER) {
                switch(cflag->automode) {
                    case CFLAG_AUTO_ON:
                        strcpy(autom,"default");
                        break;
                    case CFLAG_AUTO_OFF:
                        strcpy(autom,"off");
                        break;
                    case CFLAG_AUTO_VOICE:
                        strcpy(autom,"voice");
                        break;
                    case CFLAG_AUTO_OP:
                        strcpy(autom,"op");
                        break;
                    default:
                        strcpy(autom,"unknown");
                }

                switch(cflag->suspended) {
                    case 0:
                        strcpy(stat, "Normal");
                        break;
                    case 1:
                        strcpy(stat, "Suspended");
                        break;
                    default:
                        strcpy(stat, "Unknown");
                }
                NoticeToUser(nptr, "%s   %s   %d    %s",stat,autom,cflag->flags,cflag->nick);
           }
        }

        NoticeToUser(nptr,"End of access list");
        return;
    } else  
        NoticeToUser(nptr,"Syntax: \2CHAN ACCESS \037#channel\037 {add|auto|del|list} [\037nick\037 [\037level\037]]\2");
}

void chan_access (Nick *nptr, User *uptr, Chan *chptr, char *all)
{   
    if (!chptr) {
        NoticeToUser(nptr, "This channel does not exist");
        return;
    }

    if (HasOption(chptr, COPT_AXXFLAGS))
        chan_flags(nptr, uptr, chptr, all);
    else
        __chan_access(nptr, uptr, chptr, all);
}

void chan_op (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4;
    Nick *nptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN OP #channel [\037nick\037]\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }
        
    if (!ChannelCanOp(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }   
        
    if (!arg4 || *arg4 == '\0')
        SetStatus(nptr,arg3,CHFL_OP,1,whatbot(arg3));
    else {
        nptr2 = find_nick(arg4);
        if (!nptr2) return;
        SetStatus(nptr2,arg3,CHFL_OP,1,whatbot(arg3));
    }   
}

void chan_deop (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4;
    Nick *nptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN DEOP #channel [\037nick\037]\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!ChannelCanOp(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    if (!arg4 || *arg4 == '\0')
        SetStatus(nptr,arg3,CHFL_OP,0,whatbot(arg3));
    else {
        nptr2 = find_nick(arg4);
        if (!nptr2) return;
        SetStatus(nptr2,arg3,CHFL_OP,0,whatbot(arg3));
    }
}

void chan_voice (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4;
    Nick *nptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN VOICE #channel [\037nick\037]\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!ChannelCanVoice(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    if (!arg4 || *arg4 == '\0')
        SetStatus(nptr,arg3,CHFL_VOICE,1,whatbot(arg3));
    else {
        if (!ChannelCanHalfop(uptr, chptr) && !IsFounder(uptr, chptr)) {
            NoticeToUser(nptr, "Access denied");
            return;
        }
        nptr2 = find_nick(arg4);
        if (!nptr2) return;
        SetStatus(nptr2,arg3,CHFL_VOICE,1,whatbot(arg3));
    }
}

void chan_halfop (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4;
    Nick *nptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN HALFOP #channel [\037nick\037]\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!ChannelCanHalfop(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied"); 
        return;
    }   
        
    if (!arg4 || *arg4 == '\0')
        SetStatus(nptr,arg3,CHFL_HALFOP,1,whatbot(arg3));
    else {
        if (!ChannelCanOp(uptr, chptr) && !IsFounder(uptr, chptr)) {
            NoticeToUser(nptr, "Access denied");
            return;
        }
        nptr2 = find_nick(arg4);
        if (!nptr2) return;
        SetStatus(nptr2,arg3,CHFL_HALFOP,1,whatbot(arg3));
    }   
}

void chan_dehalfop (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4;
    Nick *nptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);
    
    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN DEHALFOP #channel [\037nick\037]\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }
        
    if (!ChannelCanHalfop(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied"); 
        return;
    }   
        
    if (!arg4 || *arg4 == '\0')
        SetStatus(nptr,arg3,CHFL_HALFOP,0,whatbot(arg3));
    else {
        if (!ChannelCanOp(uptr, chptr) && !IsFounder(uptr, chptr)) {
            NoticeToUser(nptr, "Access denied");
            return;
        }
        nptr2 = find_nick(arg4);
        if (!nptr2) return;
        SetStatus(nptr2,arg3,CHFL_HALFOP,0,whatbot(arg3));
    }
}

void chan_devoice (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4;
    Nick *nptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);
        
    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN DEVOICE #channel [\037nick\037]\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }
   
    if (!ChannelCanVoice(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied"); 
        return;
    }   
        
    if (!arg4 || *arg4 == '\0')
        SetStatus(nptr,arg3,CHFL_VOICE,0,whatbot(arg3));
    else {
        if (!ChannelCanHalfop(uptr, chptr) && !IsFounder(uptr, chptr)) {
            NoticeToUser(nptr, "Access denied");
            return;
        }
        nptr2 = find_nick(arg4);
        if (!nptr2) return;
        SetStatus(nptr2,arg3,CHFL_VOICE,0,whatbot(arg3));
    }
}

void chan_invite (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN INVITE #channel\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }
       
    if (!ChannelCanInvite(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }   
        
    InviteUser(nptr->nick,arg3);
}

void chan_set (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4,*arg5;
    char allbis[512];
    if (all) strncpy(allbis,all,512);
    arg3 = allbis;
    arg4 = SeperateWord(arg3);
    arg5 = SeperateWord(arg4);
    SeperateWord(arg5);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0' || !arg5 || *arg5 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 \037option\037 \037value\037\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    all = SeperateWord(all);
    all = SeperateWord(all);

    Command *cmd;
    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_CHAN+CMD_CHAN_SET)) {
        if (!Strcmp(cmd->name,arg4) && cmd->type == CMD_CHAN && cmd->subtype == CMD_CHAN_SET) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,chptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr,"Unknown option");
}

void chan_set_nojoin (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    if (find_chanbot(chptr->channelname)) {
        NoticeToUser(nptr,"You cannot change NOJOIN option if a bot is assigned to the channel.");
        return;
    }

    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_NOJOIN);
        NoticeToUser(nptr,"The option \2NOJOIN\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
        SendRaw(":%s PART %s :I won't idle any more on this lame channel.",me.nick,chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_NOJOIN); 
        NoticeToUser(nptr,"The option \2NOJOIN\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
        JoinChannel(me.nick,chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel nojoin {on|off}\2");
}

void chan_set_private (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_PRIVATE);
        NoticeToUser(nptr,"The option \2PRIVATE\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_PRIVATE);
        NoticeToUser(nptr,"The option \2PRIVATE\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel private {on|off}\2");
}

void chan_set_noauto (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_NOAUTO);
        NoticeToUser(nptr,"The option \2NOAUTO\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_NOAUTO);
        NoticeToUser(nptr,"The option \2NOAUTO\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel noauto {on|off}\2");
}

void chan_set_avoice (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_AVOICE);
        NoticeToUser(nptr,"The option \2AVOICE\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_AVOICE);
        NoticeToUser(nptr,"The option \2AVOICE\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel avoice {on|off}\2");
}

void chan_set_aop (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_AOP);
        NoticeToUser(nptr,"The option \2AOP\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_AOP);
        NoticeToUser(nptr,"The option \2AOP\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel aop {on|off}\2");
}

void chan_set_secure (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);
       
    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_SECURE);
        NoticeToUser(nptr,"The option \2SECURE\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_SECURE);
        NoticeToUser(nptr,"The option \2SECURE\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel secure {on|off}\2");
}

void chan_set_autolimit (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(all);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    int autolimit;
    autolimit = atoi(arg1);
    if (autolimit > 0) { 
        chptr->autolimit = autolimit;
        NoticeToUser(nptr,"The autolimit for \2%s\2 set to \2%d\2",chptr->channelname,autolimit);
        AddLimit(chptr->channelname);
    } else {
        chptr->autolimit = 0;
        NoticeToUser(nptr,"The autolimit for \2%s\2 disabled.",chptr->channelname);
    }
}

void chan_set_strictop (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(all);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_STRICTOP);
        NoticeToUser(nptr,"The option \2STRICTOP\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_STRICTOP);
        NoticeToUser(nptr,"The option \2STRICTOP\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel strictop {on|off}\2");
}

void chan_set_mass (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(all);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied");
        return;
    }

    if (!Strcmp(arg1, "on")) {
        SetOption(chptr, COPT_MASS);
        NoticeToUser(nptr, "The option \2MASS\2 has been set to \2on\2 for \2%s\2", chptr->channelname);
    } else if (!Strcmp(arg1, "off")) {
        ClearOption(chptr, COPT_MASS);
        NoticeToUser(nptr, "The option \2MASS\2 has been set to \2off\2 for \2%s\2", chptr->channelname);
    } else
        NoticeToUser(nptr, "Syntax: \2CHAN SET #channel mass {on|off}\2");
}

void chan_set_protectops (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_PROTECTOPS);
        NoticeToUser(nptr,"The option \2PROTECTOPS\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_PROTECTOPS);
        NoticeToUser(nptr,"The option \2PROTECTOPS\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel protectops {on|off}\2");
}

void chan_set_enablemask (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    if (!Strcmp(arg1,"on")) {
        SetOption(chptr, COPT_ENABLEMASK);
        NoticeToUser(nptr,"The option \2ENABLEMASK\2 has been set to \2on\2 for \2%s\2",chptr->channelname);
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(chptr, COPT_ENABLEMASK);
        NoticeToUser(nptr,"The option \2ENABLEMASK\2 has been set to \2off\2 for \2%s\2",chptr->channelname);
    } else
        NoticeToUser(nptr,"Syntax: \2CHAN SET #channel enablemask {on|off}\2");
}

void chan_set_axxflags (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied");
        return;
    }

    if (!Strcmp(arg1, "on")) {
        SetOption(chptr, COPT_AXXFLAGS);
        NoticeToUser(nptr, "The option \2AXXFLAGS\2 has been set to \2on\2 for \2%s\2", chptr->channelname);
    } else if (!Strcmp(arg1, "off")) {
        ClearOption(chptr, COPT_AXXFLAGS);
        NoticeToUser(nptr, "The option \2AXXFLAGS\2 has been set to \2off\2 for \2%s\2", chptr->channelname);
    } else
        NoticeToUser(nptr, "Syntax: \2CHAN SET #channel axxflags {on|off}\2");
}

void chan_set_enftopic (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied");
        return;
    }

    if (!Strcmp(arg1, "on")) {
        SetOption(chptr, COPT_ENFTOPIC);
        NoticeToUser(nptr, "The option \2ENFTOPIC\2 has been set to \2on\2 for \2%s\2", chptr->channelname);
    } else if (!Strcmp(arg1, "off")) {
        ClearOption(chptr, COPT_ENFTOPIC);
        NoticeToUser(nptr, "The option \2ENFTOPIC\2 has been set to \2off\2 for \2%s\2", chptr->channelname);
    } else
        NoticeToUser(nptr, "Syntax: \2CHAN SET #channel enftopic {on|off}\2");
}

void chan_set_mlock (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    if (!all || *all == '\0') {
        bzero(chptr->mlock,50);
        NoticeToUser(nptr,"Mlock removed for channel %s",chptr->channelname);
        return;
    }
            
    if (*all != '+' && *all != '-') {
        NoticeToUser(nptr,"The modes param must start by '+' or '-'");
        return;
    }
            
    if (*(all+1) == '\0') {
        bzero(chptr->mlock,50);
        NoticeToUser(nptr,"Mlock removed for channel %s",chptr->channelname);
        return;
    }
            
    strncpy(chptr->mlock,all,50);
    NoticeToUser(nptr,"Mlock for channel %s set to \2%s\2",chptr->channelname,chptr->mlock);
    SendRaw(":%s MODE %s %s",whatbot(chptr->channelname),chptr->channelname,chptr->mlock);
}

void chan_set_founder (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1 = all;
    User *uptr2;
    SeperateWord(arg1);

    if (!IsTrueOwner(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
        NoticeToUser(nptr,"Access denied");
        return;
    }
            
    uptr2 = find_user(arg1);
    if (!uptr2) {
        NoticeToUser(nptr,"The user %s is not registered",arg1);
        return;
    }
           
    User *uptr3 = find_user(chptr->owner);
    DeleteUserFromChannel(uptr3,chptr);
    strncpy(chptr->owner,arg1,NICKLEN);
    chptr->owner[NICKLEN] = '\0';
    DeleteUserFromChannel(uptr2, chptr);
    AddUserToChannel(uptr2,chptr,CHLEV_OWNER,UFLAG_CHANOWNER);
            
    NoticeToUser(nptr,"Founder of %s changed to \2%s\2",chptr->channelname,arg1);
}

void chan_info (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    time_t blah = 0;
    char *arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN INFO #channel\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (HasOption(chptr, COPT_PRIVATE) && !ChannelCanReadACL(uptr,chptr) && !IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
        NoticeToUser(nptr,"The channel %s is private.",arg3); 
        return;
    }   
        
    NoticeToUser(nptr,"Informations for channel \2%s\2:",arg3);
    NoticeToUser(nptr,"Founder: %s",chptr->owner);
    char opt[512];
    memset(opt,0x00,512);
    if (HasOption(chptr, COPT_NOJOIN))
        strcat(opt,"Nojoin ");
    if (HasOption(chptr, COPT_NOAUTO))
        strcat(opt,"Noauto ");
    if (HasOption(chptr, COPT_AVOICE))
        strcat(opt,"Avoice ");
    if (HasOption(chptr, COPT_AOP))
        strcat(opt,"Aop ");
    if (HasOption(chptr, COPT_PRIVATE))
        strcat(opt,"Private ");
    if (HasOption(chptr, COPT_STRICTOP))
        strcat(opt,"Strictop ");
    if (HasOption(chptr, COPT_SECURE))
        strcat(opt,"Secure ");
    if (HasOption(chptr, COPT_ENABLEMASK))
        strcat(opt,"Enablemask ");
    if (HasOption(chptr, COPT_PROTECTOPS))
        strcat(opt,"Protectops ");
    if (HasOption(chptr, COPT_MASS))
        strcat(opt,"Mass ");
    if (HasOption(chptr, COPT_ENFTOPIC))
        strcat(opt, "Enftopic ");
    if (HasOption(chptr, COPT_AXXFLAGS))
        strcat(opt, "Axxflags ");
    if (HasOption(chptr, COPT_NOEXPIRE))
        strcat(opt,"Noexpire ");
            
    if (*opt == '\0') sprintf(opt,"None");
    NoticeToUser(nptr,"Options: %s",opt);
        
    if (chptr->mlock[0] != '\0')
        NoticeToUser(nptr,"Mlock: %s",chptr->mlock);
            
    if (chptr->autolimit > 0)
        NoticeToUser(nptr,"Autolimit: %d",chptr->autolimit);

    blah = chptr->regtime;
    NoticeToUser(nptr, "Registration time: %s", blah ? ctime(&blah) : "Unknown");

    if (IsChanSuspended(chptr))
        NoticeToUser(nptr,"Channel suspended");
}

void chan_entrymsg (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3 = all;
    all = SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN ENTRYMSG \037#channel\037 [\037entrymsg\037]\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!ChannelCanSet(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }   
        
    if (!all || *all == '\0') {
        memset(chptr->entrymsg,0x00,250);
        NoticeToUser(nptr,"Entrymsg removed for %s",arg3);
    } else {
        strncpy(chptr->entrymsg,all,250-1);
        NoticeToUser(nptr,"Entrymsg set for %s",arg3);
    }
}

void chan_clearchan (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN CLEARCHAN \037#channel\037\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!ChannelCanProtect(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    SendRaw("MODE %s -cfijklmnprstzACGMKLNOQRSTVu",arg3);
    SendRaw("SVSMODE %s -beIqaohv",arg3);

    NoticeToUser(nptr,"Done.");
}

void chan_clearmodes (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);
    
    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN CLEARMODES \037#channel\037\2");
        return;
    }   

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }
    
    if (!ChannelCanProtect(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    SendRaw("MODE %s -cfijklmnprstzACGMKLNOQRSTVu",arg3);

    NoticeToUser(nptr,"Done.");
}

void chan_unbanall (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);
    
    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN UNBANALL \037#channel\037\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!ChannelCanOp(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    SendRaw("SVSMODE %s -b",arg3);

    NoticeToUser(nptr,"Done.");
}

void chan_kick (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg3,*arg4;
    arg3 = all;
    arg4 = SeperateWord(all);
    all = SeperateWord(arg4);
    char *bot;
    User *uptr2;

    if (!arg3 || *arg3 == '\0' || *arg3 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN KICK \037#channel\037 \037nick\037 [\037reason\037]\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!ChannelCanHalfop(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    bot = whatbot(arg3);
    if (!arg4 || *arg4 == '\0') {
        KickUser(bot,nptr->nick,chptr->channelname,"(%s) Requested", nptr->nick);
    } else {
        if (!Strcmp(arg4,bot)) {
            KickUser(bot,nptr->nick,chptr->channelname,"Don't touch !");
            return;
        }

        if (!Strcmp(arg4,"*")) {
            Member *member,*next;
            Nick *blah;
            for (member = member_list.table[HASH(chptr->channelname)]; member; member = next) {
                next = member->next;
                if (Strcmp(member->nick,nptr->nick) && !Strcmp(member->channel,chptr->channelname)) {
                    blah = find_nick(member->nick);
                    if (!blah) return;
                    if (!IsOper(blah))
                        KickUser(bot,member->nick,chptr->channelname,"(%s) Kicking all users", nptr->nick);
                }
            }
            return;
        }

        uptr2 = find_user(arg4);
        if (uptr2 && IsAuthed(uptr2)) {
            if (HasOption(chptr, COPT_AXXFLAGS)) {
                if (!ChannelCanOverride(uptr, uptr2, chptr)) {
                    NoticeToUser(nptr, "Cannot kick \2%s\2 (insufficient access level)", arg4);
                    return;
                }
            } else {
                if (GetFlag(uptr, chptr) < GetFlag(uptr2, chptr)) {
                    NoticeToUser(nptr, "Cannot kick \2%s\2 (insufficient access level)", arg4);
                    return;
                }
            }
        }

        if (!all || *all == '\0')
            KickUser(bot,arg4,chptr->channelname,"(%s) Requested", nptr->nick);
        else
            KickUser(bot,arg4,chptr->channelname,"(%s) %s", nptr->nick, all);
    }
}

void chan_assign (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1, *arg2;

    arg1 = all;
    arg2 = SeperateWord(arg1);
    SeperateWord(arg2);

    if (!arg1 || *arg1 == '\0' || *arg1 != '#' || !arg2 || *arg2 == '\0') {
        NoticeToUser(nptr,"Syntax: \2CHAN ASSIGN \037#channel\037 \037botnick\037\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    if (find_chanbot(arg1)) {
        NoticeToUser(nptr,"There is already a bot assigned to this channel. Please unassign it before.");
        return;
    }

    if (!find_bot(arg2)) {
        NoticeToUser(nptr,"This bot does not exist");
        return;
    }

    PartChannel(arg1);
    addChanbot(arg1,arg2);
    SetOption(chptr, COPT_NOJOIN);

    JoinChannel(arg2, arg1);
    NoticeToUser(nptr,"Bot \2%s\2 assigned to channel \2%s\2",arg2,arg1);
}

void chan_unassign (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1;
    Chanbot *chanbot;

    arg1 = all;
    SeperateWord(all);

    if (!arg1 || *arg1 == '\0' || *arg1 != '#') {
        NoticeToUser(nptr,"Syntax: \2CHAN UNASSIGN \037#channel\037\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr,"This channel does not exist");
        return;
    }

    if (!IsFounder(uptr,chptr) && (uptr->level < me.level_oper || !IsOper(nptr))) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    chanbot = find_chanbot(arg1);
    if (!chanbot) {
        NoticeToUser(nptr,"There is no bot assigned to this channel");
        return;
    }

    SendRaw(":%s PART %s :Channel unassigned",chanbot->bot,arg1);
    delChanbot(chanbot);
    NoticeToUser(nptr,"Channel unassigned. You can now either let the channel without bot or make %s join it with command /msg %s chan set %s nojoin off",me.nick,me.nick,arg1);
}

void chan_botlist (Nick *nptr)
{
    struct hashmap_entry *entry;
    Bot *bot;

    NoticeToUser(nptr,"List of available bots :");
    HASHMAP_FOREACH_ENTRY_VALUE(get_core()->bots, entry, bot) {
        NoticeToUser(nptr,"     \2%s\2 (%s@%s)",bot->nick,bot->ident,bot->host);
    }
    NoticeToUser(nptr,"End of list (%d entries).", hashmap_size(ACCESS_HASHMAP(get_core()->bots)));
}

void chan_addbot (Nick *nptr, User *uptr __unused, Chan *chptr __unused, char *all)
{
    char *arg1,*arg2,*arg3;
    arg1 = all;
    arg2 = SeperateWord(arg1);
    arg3 = SeperateWord(arg2);
    SeperateWord(arg3);

    if (!arg1 || *arg1 == '\0' || !arg2 || *arg2 == '\0' || !arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2CHAN ADDBOT \037nick\037 \037ident\037 \037host\037\2");
        return;
    }

    if (find_bot(arg1)) {
        NoticeToUser(nptr,"This bot already exists");
        return;
    }

    add_bot(arg1, arg2, arg3);
    fakeuser(arg1,arg2,arg3,BOTSERV_UMODES);
    SendRaw("SQLINE %s :Reserved for services",arg1);
    NoticeToUser(nptr,"Done.");
}

void chan_delbot (Nick *nptr, User *uptr __unused, Chan *chptr __unused, char *all)
{
    char *arg1;
    Bot *bot;
    Chanbot *chanbot,*next;

    arg1 = all;
    SeperateWord(arg1);

    bot = find_bot(arg1);
    if (!bot) {
        NoticeToUser(nptr,"This bot does not exist");
        return;
    }

    for (chanbot = LIST_HEAD(chanbot_list); chanbot; chanbot = next) {
        next = LIST_LNEXT(chanbot);
        if (!Strcmp(chanbot->bot,bot->nick))
            delChanbot(chanbot);
    }

    fakekill(bot->nick,"Bot deleted");
    remove_bot(bot);

    NoticeToUser(nptr,"Done.");
}

void chan_resync (Nick *nptr, User *uptr __unused, Chan *chptr, char *all)
{
    char *ch = all;
    SeperateWord(ch);

    if (!ch || *ch == '\0') {
        NoticeToUser(nptr, "Syntax: \2CHAN RESYNC \037#channel\037\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr, "Channel %s does not exist.", ch);
        return;
    }

    if (!ChannelCanProtect(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied");
        return;
    }

    acl_resync(chptr);
    NoticeToUser(nptr, "Channel resynced.");
}

void chan_suspend (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1, *ch;
    User *uptr2;
    Cflag *cflag;

    ch = all;
    arg1 = SeperateWord(ch);
    SeperateWord(arg1);

    if (!ch || *ch == '\0' || !arg1 || *arg1 == '\0') {
        NoticeToUser(nptr, "Syntax: CHAN SUSPEND \2#channel\2 \2username\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr, "Channel %s does not exist.", ch);
        return;
    }

    if (!ChannelCanACL(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }

    uptr2 = find_user(arg1);
    if (!uptr2) {
        NoticeToUser(nptr, "User %s does not exist.",arg1);
        return;
    }

    if (HasOption(chptr, COPT_AXXFLAGS)) {
        if ((find_cflag(uptr2->nick, chptr->channelname)) == NULL) {
            NoticeToUser(nptr, "User %s is not on channel access list.", arg1);
            return;
        }

        if (!ChannelCanWriteACL(uptr, uptr2, chptr) || uptr == uptr2) {
            NoticeToUser(nptr, "User %s outranks you.", arg1);
            return;
        }
    } else {
        if (GetFlag(uptr2, chptr) == 0) {
            NoticeToUser(nptr, "User %s is not on channel access list or already suspended.", arg1);
            return;
        }

        if (GetFlag(uptr2, chptr) >= GetFlag(uptr, chptr)) {
            NoticeToUser(nptr, "User %s outranks you.", arg1);
            return;
        }
    }

    cflag = find_cflag(arg1, chptr->channelname);
    if (!cflag) {
        NoticeToUser(nptr, "An error occured.");
        return;
    }

    if (cflag->suspended == 1) {
        NoticeToUser(nptr, "The access of %s is already suspended.",arg1);
        return;
    }

    cflag->suspended = 1;
    NoticeToUser(nptr, "The access of %s is now suspended.",arg1);
}

void chan_unsuspend (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1, *ch;
    User *uptr2;
    Cflag *cflag;

    ch = all;
    arg1 = SeperateWord(ch);
    SeperateWord(arg1);

    if (!ch || *ch == '\0' || !arg1 || *arg1 == '\0') {
        NoticeToUser(nptr, "Syntax: CHAN UNSUSPEND \2#channel\2 \2username\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr, "Channel %s does not exist.", ch);
        return;
    }

    if (!ChannelCanACL(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }   
    
    uptr2 = find_user(arg1);
    if (!uptr2) {
        NoticeToUser(nptr, "User %s does not exist.", arg1);
        return;
    }   

    cflag = find_cflag(arg1, chptr->channelname);

    if (!cflag) {
        NoticeToUser(nptr, "User %s is not on channel access list.", arg1);
        return;
    }   

    if (HasOption(chptr, COPT_AXXFLAGS)) {
        if (!ChannelCanWriteACL(uptr, uptr2, chptr)) {
            NoticeToUser(nptr, "User %s outranks you.", arg1);
            return;
        }
    } else {    
        if (cflag->flags >= GetFlag(uptr, chptr)) {
            NoticeToUser(nptr, "User %s outranks you.", arg1);
            return;
        }   
    }
    
    if (cflag->suspended == 0) {
        NoticeToUser(nptr, "The access of %s is not suspended.",arg1);
        return;
    }

    cflag->suspended = 0;
    NoticeToUser(nptr, "The access of %s is now restored.",arg1);
}

void chan_banlist (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    TB *tb;
    time_t blah1 = 0, blah2 = 0;
    char *t1, *t2, *ch;

    ch = all;
    SeperateWord(ch);

    if (!ch || *ch == '\0') {
        NoticeToUser(nptr, "Syntax: \2CHAN BANLIST \037#channel\037\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr, "Channel %s does not exist.", ch);
        return;
    }

    if (!ChannelCanHalfop(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }

    NoticeToUser(nptr, "List of timebans for channel %s :", chptr->channelname);

    LIST_FOREACH(tb_list, tb, HASH(chptr->channelname)) {
        blah1 = tb->setat;
        t1 = strdup(ctime(&blah1));
        *(strchr(t1, '\n')) = '\0';
        if (tb->duration == -1) {
            t2 = (char *)malloc(8*sizeof(char));
            strcpy(t2, "never");
        } else {
            blah2 = tb->setat + tb->duration;
            t2 = strdup(ctime(&blah2));
            *(strchr(t2, '\n')) = '\0';
        }
        NoticeToUser(nptr, "\2%s\2, Set at: %s, Expire at: %s, Reason: %s", tb->mask, t1, t2, tb->reason);
        free(t1);
        free(t2);
    }
    NoticeToUser(nptr, "End of list.");
}

void chan_clearbans (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    TB *tb, *next;
    char *ch = all;
    SeperateWord(ch);

    if (!ch || *ch == '\0') {
        NoticeToUser(nptr, "Syntax: \2CHAN CLEARBANS \037#channel\037\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr, "Channel %s does not exist.", ch);
        return;
    }

    if (!ChannelCanProtect(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }

    for (tb = LIST_HEAD(tb_list); tb; tb = next) {
        next = LIST_LNEXT(tb);

        if (!Strcmp(tb->channel, chptr->channelname))
            DeleteTB(tb);
    }

    NoticeToUser(nptr, "Banlist cleared.");
}

void chan_topic (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *topic;
    char mask[512];
    char *ch = all;
    topic = SeperateWord(ch);

    if (!ch || *ch == '\0') {
        NoticeToUser(nptr, "Syntax \2CHAN TOPIC \037#channel\037 [\037topic\037]\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr, "Channel %s does not exist.", ch);
        return;
    }

    if (!ChannelCanTopic(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }

    bzero(mask, 512);
    sprintf(mask, "%s!%s@%s", nptr->nick, nptr->ident, nptr->hiddenhost);
    if (!topic || *topic == '\0') {
        bzero(chptr->topic, TOPICLEN);
        SendRaw(":%s TOPIC %s %s %ld :", whatbot(ch), ch, mask, time(NULL));
        NoticeToUser(nptr, "Topic cleared.");
    } else {
        strncpy(chptr->topic, topic, TOPICLEN);
        SendRaw(":%s TOPIC %s %s %ld :%s", whatbot(ch), ch, mask, time(NULL), topic);
        NoticeToUser(nptr, "Done.");
    }
}

void chan_flags (Nick *nptr, User *uptr, Chan *chptr, char *all)
{
    char *arg1, *arg2, *arg3;
    User *uptr2;
    Cflag *cflag;
    int flags;
    arg1 = all; /* channel */
    arg2 = SeperateWord(arg1); /* user */
    arg3 = SeperateWord(arg2); /* flags */
    SeperateWord(arg3);

    if (!arg1 || *arg1 == '\0') {
        NoticeToUser(nptr, "Syntax: \2CHAN ACCESS \037#channel\037 [\037username\037 {+|-}\037flags\037]\2");
        return;
    }

    if (!chptr) {
        NoticeToUser(nptr, "Channel %s does not exist.", arg1);
        return;
    }

    if (!HasOption(chptr, COPT_AXXFLAGS)) {
        NoticeToUser(nptr, "The \2AXXFLAGS\2 option must be enabled in order to use this command.");
        return;
    }

    if (!arg2 || *arg2 == '\0') {
        if (!ChannelCanReadACL(uptr, chptr)) {
            NoticeToUser(nptr, "Access denied.");
            return;
        }

        NoticeToUser(nptr, "Access list for %s :", arg1);
        NoticeToUser(nptr, "\2Status\2       \2Flags\2       \2Nick\2");

        char stat[12];
        char *uflags_str;
        LIST_FOREACH(cflag_list, cflag, HASH(arg1)) {
            if (!Strcmp(cflag->channel, arg1)) {
                switch (cflag->suspended) {
                    case 0:
                        strcpy(stat, "Normal");
                        break;
                    case 1:
                        strcpy(stat, "Suspended");
                        break;
                    default:
                        strcpy(stat, "Unknown");
                }

                uflags_str = get_uflags_string(cflag->uflags);
                NoticeToUser(nptr, "%s   %s    %s", stat, uflags_str, cflag->nick);
                free(uflags_str);
            }
        }

        NoticeToUser(nptr, "End of access list.");
        return;
    }

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr, "Syntax: \2CHAN ACCESS \037#channel\037 [\037username\037 {+|-}\037flags\037]\2");
        return;
    }

    if (!ChannelCanACL(uptr, chptr) && !IsFounder(uptr, chptr)) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }

    if ((uptr2 = find_user(arg2)) == NULL) {
        NoticeToUser(nptr, "This user does not exist.");
        return;
    }

    if (*arg3 != '+' && *arg3 != '-') {
        NoticeToUser(nptr, "Syntax: \2CHAN FLAGS \037#channel\037 [\037username\037 {+|-}\037flags\037]\2");
        return;
    }

    if ((flags = parse_uflags(arg3+1)) == 0) {
        NoticeToUser(nptr, "No modification will be done.");
        return;
    }

    if (!ChannelCanWriteACL(uptr, uptr2, chptr)) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }

    int ret = can_modify_uflag(uptr, chptr, flags);
    if (ret == 0) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }
    if (ret == 1 && uptr != uptr2) {
        NoticeToUser(nptr, "Access denied.");
        return;
    }
    if (*arg3 == '+') {
        if (flags & UFLAG_COOWNER && get_coowner(chptr) != NULL) {
            NoticeToUser(nptr, "Cannot set more than one co-owner.");
            return;
        }
        if (flags & UFLAG_COOWNER && IsTrueOwner(uptr2, chptr)) {
            NoticeToUser(nptr, "You are already the owner of this channel. Why the hell do you want to be co-owner ?");
            return;
        }
        if ((cflag = find_cflag(arg2, arg1)) == NULL)
            AddUserToChannel(uptr2, chptr, 0, flags);
        else
            cflag->uflags |= flags;
    } else {
        if ((cflag = find_cflag(arg2, arg1)) == NULL) {
            NoticeToUser(nptr, "Cannot remove flags from a non existent user");
            return;
        }

        cflag->uflags &= ~flags;
        if (cflag->uflags == 0)
            DeleteUserFromChannel(uptr2, chptr);
    }

    NoticeToUser(nptr, "Done.");
}
