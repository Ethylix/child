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
#include "child.h"
#include "commands.h"
#include "core.h"
#include "hashmap.h"
#include "mem.h"
#include "net.h"
#include "modules.h"
#include "string_utils.h"
#include "user.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

extern commandlist command_list;

void do_nick (Nick *, User *, char *);
void do_help (Nick *, User *, char *);
void nick_link (Nick *, User *, char *);
void nick_unlink (Nick *, User *, char *);
void nick_ghost (Nick *, User *, char *);
void nick_identify (Nick *, User *, char *);
void nick_register (Nick *, User *, char *);
void nick_drop (Nick *, User *, char *);
void nick_info (Nick *, User *, char *);
void nick_set (Nick *, User *, char *);
void nick_requestpassword (Nick *, User *, char *);
void nick_set_password (Nick *, User *, char *);
void nick_set_email (Nick *, User *, char *);
void nick_set_private (Nick *, User *, char *);
void nick_set_protect (Nick *, User *, char *);
void nick_set_timeout (Nick *, User *, char *);
void nick_set_master (Nick *, User *, char *);
void nick_set_noauto (Nick *, User *, char *);
void nick_set_hidemail (Nick *, User *, char *);
void nick_set_cloak (Nick *, User *, char *);


void child_init()
{
    addBaseCommand("nick",do_nick,0);

    addNickCommand("unlink",nick_unlink,1);
    addNickCommand("set",nick_set,1);
    addNickCommand("requestpassword",nick_requestpassword,0);
    addNickCommand("register",nick_register,0);
    addNickCommand("link",nick_link,1);
    addNickCommand("info",nick_info,1);
    addNickCommand("identify",nick_identify,0);
    addNickCommand("ghost",nick_ghost,0);
    addNickCommand("drop",nick_drop,1);
    addNickSetCommand("timeout",nick_set_timeout,1);
    addNickSetCommand("protect",nick_set_protect,1);
    addNickSetCommand("private",nick_set_private,1);
    addNickSetCommand("password",nick_set_password,1);
    addNickSetCommand("noauto",nick_set_noauto,1);
    addNickSetCommand("master",nick_set_master,1);
    addNickSetCommand("hidemail",nick_set_hidemail,1);
    addNickSetCommand("email",nick_set_email,1);
    addNickSetCommand("cloak",nick_set_cloak,1);
}

void child_cleanup()
{
    deleteCommand("nick",CMD_BASE,0);

    delNickCommand("link");
    delNickCommand("unlink");
    delNickCommand("ghost");
    delNickCommand("identify");
    delNickCommand("register");
    delNickCommand("drop");
    delNickCommand("info");
    delNickCommand("set");
    delNickCommand("requestpassword");
    delNickSetCommand("password");
    delNickSetCommand("email");
    delNickSetCommand("private");
    delNickSetCommand("protect");
    delNickSetCommand("timeout");
    delNickSetCommand("master");
    delNickSetCommand("noauto");
    delNickSetCommand("hidemail");
    delNickSetCommand("cloak");
}

void do_nick (Nick *nptr, User *uptr, char *all)
{
    char *arg1,*arg2;
    char blah[1024];
    strncpy(blah,all,1024);

    arg1 = blah;
    arg2 = SeperateWord(arg1);
    SeperateWord(arg2);

    if (!arg2 || *arg2 == '\0') {
        NoticeToUser(nptr, "Type \2/msg %s help nick\2 for more informations",me.nick);
        return;
    }

    if (!Strcmp(arg2,"help")) {
        do_help(nptr,uptr,all);
        return;
    }

    all = SeperateWord(all);
    all = SeperateWord(all);

    Command *cmd;
    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_NICK)) {
        if (!Strcmp(cmd->name,arg2) && cmd->type == CMD_NICK) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr,"Unknown command");
}

void nick_link (Nick *nptr, User *uptr, char *all)
{
    User *user;
    char *arg3,*arg4;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!IsAuthed(uptr)) {
        NoticeToUser(nptr,"You are not identified");
        return;
    }

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK LINK \037username\037 \037password\037\2");
        return;
    }

    if (!Strcmp(arg3,nptr->nick)) {
        NoticeToUser(nptr,"You cannot link with yourself...");
        return;
    }

    user = find_user(arg3);
    if (!user) {
        NoticeToUser(nptr,"This user is not registered");
        return;
    }

    if (find_link(nptr->nick)) {
        NoticeToUser(nptr,"You are already linked with another nick.");
        return;
    }

    if (find_link2(nptr->nick,arg3)) {
        NoticeToUser(nptr,"You are already linked to this nick. If you want to change the master, use the SET MASTER command.");
        return;
    }

    char *pass = md5_hash(arg4);
    if (Strcmp(pass,user->md5_pass)) {
        NoticeToUser(nptr,"Wrong password.");
        free(pass);
        return;
    }
    free(pass);

    AddLink(arg3,nptr->nick);
    if (user->vhost[0] != '\0')
        strncpy(uptr->vhost,user->vhost,HOSTLEN);
    uptr->options = user->options;
    uptr->timeout = user->timeout;
    NoticeToUser(nptr,"Your nick is now linked with \2%s\2",user->nick);
}

void nick_unlink (Nick *nptr, User *uptr, char *all)
{
    User *user;
    char *arg3 = all;
    SeperateWord(arg3);

    if (!IsAuthed(uptr)) {
        NoticeToUser(nptr,"You are not identified");
        return;
    }   
        
    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK UNLINK \037username\037\2");
        return;
    }   
    
    if (!Strcmp(arg3,nptr->nick)) {
        NoticeToUser(nptr,"You cannot be linked with yourself ...");
        return;
    }   
        
    user = find_user(arg3);
    if (!user) {
        NoticeToUser(nptr,"This user is not registered");
        return;
    }   
    
    Link *link;
    if ((link = find_link2(nptr->nick,arg3)) == NULL) {
        if ((link = find_link2(arg3,nptr->nick)) == NULL) {
            NoticeToUser(nptr,"You are not linked to this nick.");
            return;
        }   
    }   
    DeleteLink(link->slave);
        
    NoticeToUser(nptr,"You are no longer linked to \2%s\2",user->nick);
}

void nick_ghost (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3,*arg4;
    User *user;
    Nick *nptr2;

    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK GHOST \037username\037 \037password\037\2");
        return;
    }

    if (!Strcmp(arg3, nptr->nick)) {
        NoticeToUser(nptr, "You want to ghost yourself ? Uh ..");
        return;
    }

    user = find_user(arg3);
    if (!user) {
        NoticeToUser(nptr,"This user is not registered");
        return;
    }   
    
    nptr2 = find_nick(arg3);
    if (!nptr2) {
        NoticeToUser(nptr,"This user is not online");
        return;
    }   
    
    char *pass = md5_hash(arg4);
    if (Strcmp(pass,user->md5_pass)) {
        NoticeToUser(nptr,"Wrong password");
        free(pass);
        return;
    }   
    free(pass);

    char ghostmsg[128];
    sprintf(ghostmsg,"GHOST command used by %s",nptr->nick);
    killuser(nptr2->nick,ghostmsg,me.nick);
    NoticeToUser(nptr,"Done.");
}

void nick_identify (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3;
    User *user;

    arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK IDENTIFY \037password\037\2");
        return;
    }   
        
    user = find_user(nptr->nick);
    if (!user) {
        NoticeToUser(nptr,"You are not registered");
        return;
    }   
        
    if (user->authed == 1) {
        NoticeToUser(nptr,"You are already identified");
        return;
    }   
        
    char *pass = md5_hash(arg3);
    if (Strcmp(pass,user->md5_pass)) {
        NoticeToUser(nptr,"Wrong password");
        free(pass);

        nptr->loginattempts++;
        if (nptr->lasttry == 0) nptr->lasttry = time(NULL);
        if (nptr->loginattempts >= me.maxloginatt && time(NULL) - nptr->lasttry < 60) {
            killuser(nptr->nick,"Max login attempts exceeded",me.nick);
        } else if (nptr->loginattempts < me.maxloginatt && time(NULL) - nptr->lasttry >= 60) {
            nptr->loginattempts = 1;
            nptr->lasttry = time(NULL);
        }

        return;
    }   
    free(pass);
        
    if (IsUserSuspended(user)) {
        NoticeToUser(nptr,"Your account has been suspended. Please contact a services administrator for more informations");
        return;
    }   
        
    user->authed = 1;
    NoticeToUser(nptr,"You are now identified");
    nptr->loginattempts = 0;
    nptr->lasttry = 0;

    if (user->email[0] == '\0')
        NoticeToUser(nptr,"Please set a valid email address with /msg C nick set email email@domain.xx");

    user->lastseen = time(NULL);
    SendRaw("SVS2MODE %s +r",nptr->nick);
    SendRaw("SVSLOGIN * %s %s",nptr->nick,nptr->nick);
    if (user->vhost[0] != '\0') {
        SendRaw("CHGHOST %s %s",nptr->nick,user->vhost);
        strncpy(nptr->hiddenhost, user->vhost, HOSTLEN);
        NoticeToUser(nptr,"Your vhost \2%s\2 has been activated",user->vhost);
    } else if (HasOption(user, UOPT_CLOAKED)) {
        SendRaw("CHGHOST %s %s%s", nptr->nick, user->nick, me.usercloak);
        char host[HOSTLEN + NICKLEN + 1];
        bzero(host, HOSTLEN+NICKLEN+1);
        snprintf(host, HOSTLEN + NICKLEN + 1, "%s%s", user->nick, me.usercloak);
        strncpy(nptr->hiddenhost, host, HOSTLEN);
        NoticeToUser(nptr,"Your cloak has been activated");
    }
    if (HasOption(user, UOPT_PROTECT)) DeleteGuest(nptr->nick);
    if (HasOption(user, UOPT_NOAUTO)) return;

    sync_user(user);
}

void nick_register (Nick *nptr, User *uptr, char *all)
{
    User *user;
    char *arg3 = all;
    char *arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (uptr) {
        NoticeToUser(nptr,"This nick is already registered");
        return;
    }   

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK REGISTER \037password\037 \037email\037\2");
        return;
    }   

    char *str1=NULL,*str2=NULL,*str3=NULL,*str4=NULL;
    str1 = strchr(arg4,'@');
    if (str1) {
        str4 = str1-1;
        str2 = strchr(str1,'.');
        if (str2)
            str3 = str2+2;
    }

    if (!str1 || *str1 == '\0' || !str2 || *str2 == '\0' || !str3 || *str3 == '\0' || !str4 || *str4 == '\0') {
        NoticeToUser(nptr,"Please set a valid email address");
        return;
    }

    if (!HASHMAP_EMPTY(core_get_users()))
        AddUser(nptr->nick,1);
    else {
        AddUser(nptr->nick,me.level_owner);
        NoticeToUser(nptr,"You have now the level \2%d\2",me.level_owner);
    }   
        
    user = find_user(nptr->nick);
    if (!user) return;
        
    memset(user->md5_pass,'\0',34);

    char newpass[16];
    char email[512];
    bzero(newpass, 16);
    char *pass;
    if (me.emailreg == 1) {
        gen_rand_string(newpass, "A-Za-z0-9", 12);
        pass = md5_hash(newpass);
    } else
        pass = md5_hash(arg3);

    strncpy(user->md5_pass,pass,32);
    free(pass);
    strncpy(user->email,arg4,EMAILLEN);
    user->regtime = time(NULL);
    user->lastseen = time(NULL);

    NoticeToUser(nptr, "You are now registered.");

    if (me.emailreg == 1) {
        bzero(email, 512);
        sprintf(email, "From: %s\r\nTo: %s\r\nSubject: GeekNode IRC account registration\r\n\r\nYour user info:\r\n\tLogin: %s\r\n\tGenerated password: %s\r\n\r\nYou can auth with the following command: /msg %s nick identify %s\r\n", me.sendfrom, user->email, user->nick, newpass, me.nick, newpass);        sendmail(user->email, email);
        NoticeToUser(nptr, "A password has been generated and sent to your specified e-mail address (this mean that the password you've specified doesn't work).");
    } else {        
        user->authed = 1;
        SendRaw("SVSMODE %s +r",nptr->nick);
    }
}

void nick_drop (Nick *nptr, User *uptr, char *all)
{
    User *uptr2;
    char *arg3 = all;
    SeperateWord(arg3);

    if (!IsAuthed(uptr)) {
        NoticeToUser(nptr,"You are not identified");
        return;
    }

    if (!arg3 || *arg3 == '\0') {
        userdrop(uptr);
        NoticeToUser(nptr, "Your nick has been dropped.");
        return;
    }

    if (uptr->level < me.level_oper || !IsOper(nptr)) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    uptr2 = find_user(arg3);
    if (!uptr2) {
        NoticeToUser(nptr,"This nick is not registered");
        return;
    }

    if (uptr2->level >= uptr->level) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    userdrop(uptr2);
    NoticeToUser(nptr,"The nick %s has been dropped",arg3);

    globops("%s used \2DROP\2 on %s",nptr->nick,arg3);
    operlog("%s dropped nick %s",nptr->nick,arg3);
}

void nick_info (Nick *nptr, User *uptr, char *all)
{
    struct hashmap_entry *entry;
    User *user;
    Link *link;
    Cflag *cflag;
    Chan *chptr;
    char *uflags_str;
    char *arg3 = all;
    SeperateWord(arg3);

    if (!IsAuthed(uptr)) {
        NoticeToUser(nptr,"You are not identified");
        return;
    }   
        
    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK INFO \037nick\037\2");
        return;
    }   
        
    user = find_user(arg3);
    if (!user) {
        NoticeToUser(nptr,"This user does not exist");
        return;
    }   
        
    if (HasOption(user, UOPT_PRIVATE) && Strcmp(user->nick,uptr->nick) && (uptr->level < me.level_oper || !IsOper(nptr))) {
        NoticeToUser(nptr,"The nick %s is private.",user->nick);
        return;
    }   
        
    NoticeToUser(nptr,"Informations for \2%s\2 :",user->nick);
    NoticeToUser(nptr,"   Level :             %d",user->level);
    NoticeToUser(nptr,"   Online:             %s",user->authed == 1 ? "yes" : "no");
    if (!Strcmp(nptr->nick,arg3) || (uptr->level >= me.level_oper && IsOper(nptr)) || !HasOption(user, UOPT_HIDEMAIL))
        NoticeToUser(nptr,"   Email:              %s",user->email);
    char opt[512];
    memset(opt,0x00,512);
    if (HasOption(user, UOPT_PROTECT)) strcat(opt,"Protect ");
    if (HasOption(user, UOPT_PRIVATE)) strcat(opt,"Private ");
    if (IsUserNoexpire(user) || user->level >= me.level_oper) strcat(opt,"Noexpire ");
    if (HasOption(user, UOPT_NOAUTO)) strcat(opt,"Noauto ");
    if (HasOption(user, UOPT_HIDEMAIL)) strcat(opt,"Hidemail ");
    if (*opt == '\0') sprintf(opt,"None");
    NoticeToUser(nptr,"   Options:            %s",opt);
    if (HasOption(user, UOPT_PROTECT))
        NoticeToUser(nptr,"   Timeout:            %d",user->timeout);
    if (IsUserSuspended(user))
        NoticeToUser(nptr,"   Account suspended");
    time_t blah=0;
    blah = user->lastseen;
    NoticeToUser(nptr,"   Last seen: %s",ctime(&blah));
    blah = user->regtime;
    NoticeToUser(nptr,"   Registration time: %s", blah ? ctime(&blah) : "Unknown");
    if (!Strcmp(nptr->nick,arg3) || (uptr->level >= me.level_oper && IsOper(nptr))) {
        NoticeToUser(nptr,"   Linked nicks:");
        HASHMAP_FOREACH_ENTRY_VALUE(core_get_links(), entry, link) {
            if (!Strcmp(link->master,arg3))
                NoticeToUser(nptr,"       %s (Master: %s)",link->slave,link->master);
            if (!Strcmp(link->slave,arg3))
                NoticeToUser(nptr,"       %s (Master)",link->master);
        }

        NoticeToUser(nptr,"   Has access on :");
        LLIST_FOREACH_ENTRY(&user->cflags, cflag, user_head) {
            chptr = cflag->chan;
            if (HasOption(chptr, COPT_AXXFLAGS)) {
                uflags_str = get_uflags_string(cflag->uflags);
                NoticeToUser(nptr, "      %s (%s)", chptr->channelname, uflags_str);
                free(uflags_str);
            } else
                NoticeToUser(nptr, "      %s (%d)", chptr->channelname, cflag->flags);
        }
    }
}

void nick_set (Nick *nptr, User *uptr, char *all)
{
    char *arg3,*arg4;
    char allbis[512];
    if (all) strncpy(allbis,all,512);
    arg3 = allbis;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK SET \037option\037 \037value\037\2");
        return;
    }

    all = SeperateWord(all);

    Command *cmd;
    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_NICK+CMD_NICK_SET)) {
        if (!Strcmp(cmd->name,arg3) && cmd->type == CMD_NICK && cmd->subtype == CMD_NICK_SET) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr,"Unknown option");
}

void nick_set_protect (Nick *nptr, User *uptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!Strcmp(arg1,"on")) {
        SetOption(uptr, UOPT_PROTECT);
        NoticeToUser(nptr,"The option \2PROTECT\2 has been set.");
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(uptr, UOPT_PROTECT);
        NoticeToUser(nptr,"The option \2PROTECT\2 has been unset.");
    } else
        NoticeToUser(nptr,"Syntax: \2NICK SET \037protect\037 [\037on\037|\037off\037]\2");
}   

void nick_set_timeout (Nick *nptr, User *uptr, char *all)
{
    char *arg1 = all;
    SeperateWord(all);

    uptr->timeout = strtol(arg1,NULL,10);
    if (uptr->timeout == 0) {
        uptr->timeout = TIMEOUT_DFLT;
        NoticeToUser(nptr,"The timeout has been set to the default value (%d seconds)",TIMEOUT_DFLT);
    } else
        NoticeToUser(nptr,"The timeout has been set to %d seconds",uptr->timeout);
}

void nick_set_private (Nick *nptr, User *uptr, char *all)
{
    char *arg1 = all;
    SeperateWord(all);

    if (!Strcmp(arg1,"on")) {
        SetOption(uptr, UOPT_PRIVATE);
        NoticeToUser(nptr,"The option \2PRIVATE\2 has been set");
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(uptr, UOPT_PRIVATE);
        NoticeToUser(nptr,"The option \2PRIVATE\2 has been unset");
    } else
        NoticeToUser(nptr,"Syntax: \2NICK SET \037private\037 [\037on\037|\037off\037]\2");
}

void nick_set_noauto (Nick *nptr, User *uptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!Strcmp(arg1,"on")) {
        SetOption(uptr, UOPT_NOAUTO);
        NoticeToUser(nptr,"The option \2NOAUTO\2 has been set");
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(uptr, UOPT_NOAUTO);
        NoticeToUser(nptr,"The option \2NOAUTO\2 has been unset");
    } else
        NoticeToUser(nptr,"Syntax: \2NICK SET \037noauto\037 [\037on\037|\037off\037]\2");
}

void nick_set_hidemail (Nick *nptr, User *uptr, char *all)
{
    char *arg1 = all;
    SeperateWord(all);

    if (!Strcmp(arg1,"on")) {
        SetOption(uptr, UOPT_HIDEMAIL);
        NoticeToUser(nptr,"The option \2HIDEMAIL\2 has been set");
    } else if (!Strcmp(arg1,"off")) {
        ClearOption(uptr, UOPT_HIDEMAIL);
        NoticeToUser(nptr,"The option \2HIDEMAIL\2 has been unset");
    } else
        NoticeToUser(nptr,"Syntax: \2NICK SET \037hidemail\037 [\037on\037|\037off\037]\2");
}

void nick_set_cloak (Nick *nptr, User *uptr, char *all)
{
    char *arg1 = all;
    char host[HOSTLEN + NICKLEN + 1];
    SeperateWord(all);

    if (!Strcmp(arg1,"on")) {
        SetOption(uptr, UOPT_CLOAKED);
        SendRaw("CHGHOST %s %s%s", uptr->nick, uptr->nick, me.usercloak);
        bzero(host, HOSTLEN+NICKLEN+1);
        snprintf(host, HOSTLEN+NICKLEN+1, "%s%s", uptr->nick, me.usercloak);
        strncpy(nptr->hiddenhost, host, HOSTLEN);
        NoticeToUser(nptr,"Your host is now cloaked");
    } else if (!Strcmp(arg1, "off")) {
        ClearOption(uptr, UOPT_CLOAKED);
        NoticeToUser(nptr,"Cloak removed, but you still need to /umode -x to completely remove it (you can then do /umode +x to set the normal cloak)");
    } else
        NoticeToUser(nptr,"Syntax: \2NICK SET \037cloak\037 [\037on\037|\037off\037]\2");
}

void nick_set_master (Nick *nptr, User *uptr __unused, char *all)
{
    Link *link;
    char *arg1 = all;
    SeperateWord(all);

    if (!Strcmp(nptr->nick,arg1)) {
        link = find_link(nptr->nick);
        if (!link) {
            NoticeToUser(nptr,"You are already the master or you are not linked");
            return;
        }

        strncpy(link->slave,link->master,NICKLEN);
        strncpy(link->master,arg1,NICKLEN);
        NoticeToUser(nptr,"The master has been set to %s",arg1);
    } else {
        link = find_link2(nptr->nick,arg1);
        if (!link) {
            NoticeToUser(nptr,"You are not linked to %s",arg1);
            return;
        }
        strncpy(link->slave,link->master,NICKLEN);
        strncpy(link->master,arg1,NICKLEN);
        NoticeToUser(nptr,"The master has been set to %s",arg1);
    }
}

void nick_set_email (Nick *nptr, User *uptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    char *str1=NULL,*str2=NULL,*str3=NULL,*str4=NULL;
    str1 = strchr(arg1,'@');
    if (str1) {
        str4 = str1-1;
        str2 = strchr(str1,'.');
        if (str2)
            str3 = str2+2;
    }

    if (!str1 || *str1 == '\0' || !str2 || *str2 == '\0' || !str3 || *str3 == '\0' || !str4 || *str4 == '\0') {
        NoticeToUser(nptr,"Please set a valid email address");
        return;
    }

    strncpy(uptr->email,arg1,EMAILLEN);
    NoticeToUser(nptr,"Email address set to %s",arg1);
}

void nick_set_password (Nick *nptr, User *uptr, char *all)
{
    char *arg1,*arg2;
    arg1 = all;
    arg2 = SeperateWord(arg1);
    SeperateWord(arg2);

    if (!arg2 || *arg2 == '\0') {
        memset(uptr->md5_pass,'\0',32);
        char *pass = md5_hash(arg1);
        strncpy(uptr->md5_pass,pass,32);
        free(pass);
        NoticeToUser(nptr,"Your password has been changed");
    } else {
        if (uptr->level < me.level_oper || !IsOper(nptr)) {
            NoticeToUser(nptr,"Access denied");
            return;
        }   
             
        User *uptr2 = find_user(arg1);
        if (!uptr2) {
            NoticeToUser(nptr,"The nick \2%s\2 is not registered",arg1);
            return;
        }   

        if (uptr2->level >= uptr->level) {
            NoticeToUser(nptr,"You cannot change password of a user outranking you");
            return;
        }

        memset(uptr2->md5_pass,'\0',32);
        char *pass = md5_hash(arg2);
        strncpy(uptr2->md5_pass,pass,32);
        NoticeToUser(nptr,"Password changed.");
        free(pass);
        globops("%s used \2PASSWORD\2 on nick %s",nptr->nick,uptr2->nick);
    }
}

void nick_requestpassword (Nick *nptr, User *uptr, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);

    if (!arg1 || *arg1 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK REQUESTPASSWORD \037username\037\2");
        return;
    }

    uptr = find_user(arg1);
    if (!uptr) {
        NoticeToUser(nptr,"This user does not exist");
        return;
    }

    if (IsAuthed(uptr)) {
        NoticeToUser(nptr,"This user is authed. You can change your password by typing /msg C nick set password \2newpass\2");
        return;
    }

    if (uptr->email[0] == '\0') {
        NoticeToUser(nptr,"Email address is not set");
        return;
    }

    char newpass[16];
    bzero(newpass,16);
    gen_rand_string(newpass,"A-Za-z0-9",12);
    char *pass = md5_hash(newpass);
    strncpy(uptr->md5_pass,pass,32);
    free(pass);

    char email[1024];
    bzero(email, 1024);

    operlog("%s (%s@%s) requested new password for %s\n",nptr->nick,nptr->ident,nptr->host,uptr->nick);
    snprintf(email, 1023, "From: Child <%s>\r\nTo: %s <%s>\r\nSubject: Request of password\r\n\r\n%s (%s@%s) requested new password.\r\nYour user info:\r\n\tLogin: %s\r\n\tPassword: %s\r\n\r\nYou can auth with the following command: /msg %s nick identify %s\r\n", me.sendfrom, uptr->nick, uptr->email, nptr->nick, nptr->ident, nptr->host, uptr->nick, newpass, me.nick, newpass);
    sendmail(uptr->email, email);

    NoticeToUser(nptr,"New password generated and sent to your email address");
}
