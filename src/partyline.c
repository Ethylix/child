/*          
Child, Internet Relay Chat Services
Copyright (C) 2005-2007  David Lebrun (target0@geeknode.org)
            
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
USA.
*/  

    
#include "partyline.h"

#include "channel.h"
#include "child.h"
#include "db.h"
#include "mem.h"
#include "modules.h"
#include "string_utils.h"
#include "user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

extern eclientlist eclient_list;
extern modulelist module_list;
extern nicklist nick_list;

extern int emerg, emerg_req;

int ReadPChunk(Eclient *ecl)
{
    int oldbytes;
    int readbytes;
    char *dest;

    if (*ecl->pnextline != '\0') {
        oldbytes = strlen(ecl->pnextline);
        dest = ecl->pchunkbuf;
        while (*ecl->pnextline) {
            *dest++ = *ecl->pnextline++;
        }
        ecl->pchunkbufentry = ecl->pchunkbuf + oldbytes;
    } else {
        oldbytes = 0;
        ecl->pchunkbufentry = ecl->pchunkbuf;
    }

    readbytes = read(ecl->fd, ecl->pchunkbufentry, (CHUNKSIZE - oldbytes) - 10);
    if (readbytes == 0 || readbytes == -1)
        return 0;

    *(ecl->pchunkbufentry + readbytes) = '\0';
    ecl->pnextline = ecl->pchunkbuf;

    return 1;
}

int GetLineFromPChunk(int fd)
{
    Eclient *ecl = find_eclient(fd);
    if (!ecl)
        return 0;

    ecl->pcurrentline = ecl->pnextline;
    while (*ecl->pnextline != '\n' && *ecl->pnextline != '\r' && *ecl->pnextline != '\0') {
        ecl->pnextline++;
    }

    if (*ecl->pnextline == '\0') {
        ecl->pnextline = ecl->pcurrentline;
        return 0;
    }

    if (*ecl->pnextline == '\n' || *ecl->pnextline == '\r') {
        *ecl->pnextline = '\0';
        ecl->pnextline++;
    }

    if (*ecl->pnextline == '\n' || *ecl->pnextline == '\r') {
        *ecl->pnextline = '\0';
        ecl->pnextline++;
    }

    return 1;
}

void ParseEclient (Eclient *eclient)
{
    void *pl_commands[] =
                    {
                        ".auth", p_auth,
                        ".canpl", p_canpl,
                        ".close", p_close,
                        ".deletenick", p_deletenick,
                        ".die", p_die,
                        ".dropchan", p_dropchan,
                        ".dropuser", p_dropuser,
                        ".eject", p_eject,
                        ".emerg", p_emerg,
                        ".exec", p_exec,
                        ".getchaninfo", p_getchaninfo,
                        ".getuserinfo", p_getuserinfo,
                        ".gline", p_gline,
                        ".help", p_help,
                        ".kill", p_kill,
                        ".modlist", p_modlist,
                        ".modload", p_modload,
                        ".modunload", p_modunload,
                        ".nicklist", p_nicklist,
                        ".quit", p_quit,
                        ".raw", p_raw,
                        ".rehash", p_rehash,
                        ".restart", p_restart,
                        ".savedb", p_savedb,
                        ".set", p_set,
                        ".setchan", p_setchan,
                        ".setchanopt", p_setchanopt,
                        ".setnickflag", p_setnickflag,
                        ".setuser", p_setuser,
                        ".setuseropt", p_setuseropt,
                        ".who", p_who,
                    };

    char *command,*tail;

    command = StripBlanks(eclient->pcurrentline);
    tail = SeperateWord(command);

    if (!command || *command == '\0')
        return;

    User *user;
    user = find_user(eclient->nick);

    unsigned int i;
    void (*func)();
    if (*command == '.') {
        for (i=0;i<(sizeof(pl_commands)/(sizeof(char *)))-1;i+=2) {
            if (!Strcmp(command,pl_commands[i])) {
                func = pl_commands[i+1];
                func(eclient,user,command,tail);
                return;
            }
        }
        CheckPAuth();
        send_to(eclient,"Unknown command \"%s\"",command);
        return;
    }

    CheckPAuth();

    if (!command) command = "";
    if (!tail) tail = "";
    pllog("<%s> %s %s",eclient->nick,command,tail);
    sendto_all("<%s> %s %s",eclient->nick,command,tail);
}

void p_auth (Eclient *eclient, User *user, char *command __unused, char *tail)
{
    char *nick,*pass;
    nick = tail;
    pass = SeperateWord(nick);
    SeperateWord(pass);

    if (!nick || *nick == '\0' || !pass || *pass == '\0')
        return;

    if (eclient->authed == 1) {
        send_to(eclient,"You are already authed");
        return;
    }

    user = find_user(nick);
    if (!user)
        return;

    char *pass2 = md5_hash(pass);
    if (Strcmp(pass2,user->md5_pass)) {
        free(pass2);
        return;
    }
    free(pass2);

    if (IsUserSuspended(user))
        return;

    if ((user->level < me.level_owner) && !CanPl(user))
        return;

    if (user->level < me.level_owner && emerg)
        return;

    eclient->authed = 1;
    strncpy(eclient->nick,user->nick,NICKLEN);
    send_to(eclient,"You are now identified");
    send_to(eclient,"PLEASE REMEMBER THAT FROM NOW, *ALL* COMMANDS ARE LOGGED");
    sendto_all_butone(eclient,"*** User %s (%s) logged in",eclient->nick,eclient->host);
}

void p_canpl (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *nick = tail;
    char *what = SeperateWord(nick);
    SeperateWord(what);
    
    if (!nick || *nick == '\0' || !what || *what == '\0') {
        send_to(eclient,"Syntax: canpl username <0|1>");
        return;
    }   
    
    User *user2 = find_user(nick);
    if (!user2) {
        send_to(eclient,"This user does not exist");
        return;
    }   
    
    switch (atoi(what)) {
        case 0:
            ClearOption(user2, UOPT_CANPL);
            break;
        case 1:
            SetOption(user2, UOPT_CANPL);
            break;
        default:
            send_to(eclient,"Syntax: canpl username <0|1>");
            return;
    }       
    
    send_to(eclient,"CanPl flag set to %s for %s",what,nick);
}

void p_close (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    Eclient *ecl,*next;
    for (ecl = LIST_HEAD(eclient_list); ecl; ecl = next) {
        next = LIST_LNEXT(ecl);
        if (ecl->authed != 1) {
            close(ecl->fd);
            DeleteEclient(ecl);
        }
    }

    send_to(eclient,"Done.");
}

void p_deletenick (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *nick = tail;
    SeperateWord(nick);
    if (!nick || *nick == '\0') {
        send_to(eclient,"Syntax: deletenick nick");
        return;
    }   
    
    Nick *nptr;
    User *uptr;
    nptr = find_nick(nick);
    if (!nptr) {
        send_to(eclient,"This nick does not exist");
        return;
    }   
    
    uptr = find_user(nick);
    if (uptr) {
        if (uptr->authed == 1) {
            uptr->authed = 0;
            uptr->lastseen = time(NULL);
        }   
    }   
    
    DeleteUserFromWchans(nptr);
    DeleteWildNick(nptr);
    
    send_to(eclient,"Done.");
}

void p_die (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_root) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    SendRaw(":%s QUIT :Quit ordered by %s",me.nick,eclient->nick);
    operlog("%s executed DIE from partyline",eclient->nick);
    sendto_all("%s DIED the services!",eclient->nick);
    child_die(1);
}

void p_dropchan (Eclient *eclient, User *user __unused, char *command, char *tail)
{
    CheckAndLog();
    char *chan = tail;
    SeperateWord(chan);
    
    if (!chan || *chan == '\0') {
        send_to(eclient,"Syntax: dropchan channel");
        return;
    }   
    
    Chan *chptr;
    chptr = find_channel(chan);
    if (!chptr) {
        send_to(eclient,"This channel does not exist");
        return;
    }

    chandrop(chptr);
    send_to(eclient,"Done.");
}

void p_dropuser (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *nick = tail;
    SeperateWord(nick);
    
    if (!nick || *nick == '\0') {
        send_to(eclient,"Syntax: dropuser username");
        return;
    }   
    
    User *uptr;
    uptr = find_user(nick);
    if (!uptr) {
        send_to(eclient,"This user does not exist");
        return;
    }

    userdrop(uptr);
    send_to(eclient,"Done.");
}

void p_eject (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *nick = tail;
    SeperateWord(nick);
    
    if (!nick || *nick == '\0') {
        send_to(eclient,"Syntax: eject nick");
        return;
    }   
    
    Eclient *ecl;
    
    ecl = find_eclient_name(nick);
    if (!ecl) {
        send_to(eclient,"This user is not connected");
        return;
    }   
    
    send_to(ecl,"You have been ejected by %s",eclient->nick);
    sendto_all_butone(ecl,"*** User %s (%s) logged out (Ejected by %s)",ecl->nick,ecl->host,eclient->nick);
    close(ecl->fd);
    DeleteEclient(ecl);
}

void p_emerg (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *code = tail;
    SeperateWord(code);
    
    if (!code || *code == '\0') {
        send_to(eclient,"Wrong syntax");
        return;
    }   
    
    if (!emerg_req) {
        send_to(eclient,"No request");
        return;
    }   
    
    if (atoi(code) == emerg_req) {
        send_to(eclient,"EMERGENCY STATUS ENABLED");
        MsgToChan(OPERCHAN,"\2\0034%s \037ENABLED\037 emergency status\2",user->nick);
        globops("\2\0034%s \037ENABLED\037 emergency status\2",user->nick);
        operlog("%s enabled emergency status",user->nick);
        emerg_req = 0;
        emerg = 1;
        RunHooks(HOOK_EMERG_ON,NULL,NULL,NULL,NULL);
    } else
        send_to(eclient,"Wrong code");
}

void p_exec (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    if (me.enable_exec == 0) {
        send_to(eclient,"Command disabled");
        return;
    }   
    
    if (!tail || *tail == '\0') {
        send_to(eclient,"Syntax: exec command");
        return;
    }   
    
    char pbuff[65535];
    bzero(pbuff,65535);
    FILE *pp;
    char *str;
    pp = popen(tail,"r");
    while (fgets(pbuff,sizeof(pbuff),pp)) {
        str = strstr(pbuff,"\r");
        if (str) *str = '\0';
        str = strstr(pbuff,"\n");
        if (str) *str = '\0';
        send_to(eclient,"%s",pbuff);
    }   
    pclose(pp);
}

void p_getchaninfo (Eclient *eclient, User *user __unused, char *command, char *tail)
{
    CheckAndLog();
    char *chan = tail;
    SeperateWord(chan);
    
    if (!chan || *chan == '\0') {
        send_to(eclient,"Syntax: getchaninfo channel");
        return;
    };  
    
    Chan *chptr;
    chptr = find_channel(chan);
    if (!chptr) {
        send_to(eclient,"This channel does not exist");
        return;
    }   
    
    send_to(eclient,"Informations for channel %s :",chptr->channelname);
    send_to(eclient,"\tchannelname:\t%s",chptr->channelname);
    send_to(eclient,"\towner:\t%s",chptr->owner);
    send_to(eclient,"\tentrymsg:\t%s",chptr->entrymsg);
    send_to(eclient,"\tmlock:\t%s",chptr->mlock);
    send_to(eclient,"\tautolimit:\t%d",chptr->autolimit);
    send_to(eclient,"\tlastseen:\t%d",chptr->lastseen);
    char opt[512];
    bzero(opt,512);
    if (HasOption(chptr, COPT_NOJOIN)) strcat(opt,"Nojoin ");
    if (HasOption(chptr, COPT_NOAUTO)) strcat(opt,"Noauto ");
    if (HasOption(chptr, COPT_AVOICE)) strcat(opt,"Avoice ");
    if (HasOption(chptr, COPT_PRIVATE)) strcat(opt,"Private ");
    if (HasOption(chptr, COPT_STRICTOP)) strcat(opt,"Strictop ");
    if (HasOption(chptr, COPT_AOP)) strcat(opt,"Aop ");
    if (HasOption(chptr, COPT_SECURE)) strcat(opt,"Secure ");
    if (HasOption(chptr, COPT_SUSPENDED)) strcat(opt,"Suspended ");
    if (HasOption(chptr, COPT_NOEXPIRE)) strcat(opt,"Noexpire");
    send_to(eclient,"\toptions:\t%s",opt);
    send_to(eclient,"End of informations");
}

void p_getuserinfo (Eclient *eclient, User *user __unused, char *command, char *tail)
{
    CheckAndLog();
    char *nick = tail;
    SeperateWord(nick);
    
    if (!nick || *nick == '\0') {
        send_to(eclient,"Syntax: getuserinfo username");
        return;
    }   
    
    User *uptr;
    uptr = find_user(nick);
    if (!uptr) {
        send_to(eclient,"This user does not exist");
        return;
    }   
    
    send_to(eclient,"Informations about %s :",uptr->nick);
    send_to(eclient,"\tnick:\t%s",uptr->nick);
    send_to(eclient,"\temail:\t%s",uptr->email);
    send_to(eclient,"\tauthed:\t%d",uptr->authed);
    send_to(eclient,"\tlevel:\t%d",uptr->level);
    send_to(eclient,"\tmd5_pass:\t%s",uptr->md5_pass);
    send_to(eclient,"\tlastseen:\t%d",uptr->lastseen);
    send_to(eclient,"\ttimeout:\t%d",uptr->timeout);
    send_to(eclient,"\tvhost:\t%s",uptr->vhost);
    char opt[512];
    bzero(opt,512);
    if (HasOption(uptr, UOPT_PROTECT)) strcat(opt,"Protect ");
    if (HasOption(uptr, UOPT_PRIVATE)) strcat(opt,"Private ");
    if (HasOption(uptr, UOPT_SUSPENDED)) strcat(opt,"Suspended ");
    if (HasOption(uptr, UOPT_NOEXPIRE)) strcat(opt,"Noexpire ");
    if (HasOption(uptr, UOPT_NOAUTO)) strcat(opt,"Noauto ");
    if (HasOption(uptr, UOPT_HIDEMAIL)) strcat(opt,"Hidemail ");
    if (HasOption(uptr, UOPT_CANPL)) strcat(opt,"CanPl");
    send_to(eclient,"\toptions:\t%s",opt);
    send_to(eclient,"End of informations.");
}

void p_gline (Eclient *eclient, User *user __unused, char *command, char *tail)
{
    CheckAndLog();
    char *what,*ident,*host,*tm,*reason;
    what = tail;
    ident = SeperateWord(tail);
    host = SeperateWord(ident);
    tm = SeperateWord(host);
    reason = SeperateWord(tm);
    
    if (!what || *what == '\0' || !ident || *ident == '\0' || !host || *host == '\0') {
        send_to(eclient,"Syntax: gline <+|-> ident host [time] [reason]");
        return;
    }   
    
    if ((!tm || *tm == '\0' || !reason || *reason == '\0') && *what == '+') {
        send_to(eclient,"Syntax: gline <+|-> ident host [time] [reason]");
        return;
    }   
    
    if ((*what != '+' && *what != '-') || strlen(what) > 1) {
        send_to(eclient,"Syntax: gline <+|-> ident host [time] [reason]");
        return;
    }   
    
    if (*what == '+')
        glineuser(ident,host,atoi(tm),reason);
    else
        unglineuser(ident,host);
        
    send_to(eclient,"Done.");
}

void p_help (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    send_to(eclient,"Available commands (all commands must start by a .) :");
    send_to(eclient,"\tauth\tAuth yourself");
    send_to(eclient,"\twho\tList connected users");
    send_to(eclient,"\tquit\tQuit the partyline");
    send_to(eclient,"\tnicklist\tList all users connected to the irc network");
    send_to(eclient,"\tgetuserinfo\tGive all informations about a user");
    send_to(eclient,"\tgetchaninfo\tGive all informations about a channel");
    send_to(eclient,"\tkill\tKill a user");
    send_to(eclient,"\tgline\tAdd/remove a gline");
    send_to(eclient,"\tdropchan\tDrop a channel");
    if (user->level >= me.level_root) {
    send_to(eclient,"\tmodload\tLoad a module");
    send_to(eclient,"\tmodunload\tUnload a module");
    send_to(eclient,"\tmodlist\tList loaded modules");
    send_to(eclient,"\trehash\tRehash configuration");
    send_to(eclient,"\tsavedb\tSave databases");
    send_to(eclient,"\trestart\tRestart the services");
    send_to(eclient,"\tdie\tKill the services");
    }
    if (user->level == me.level_owner) {
    send_to(eclient,"\tset\tSet or display configuration options");
    send_to(eclient,"\tsetuser\tSet user informations");
    send_to(eclient,"\tsetchan\tSet channel informations");
    send_to(eclient,"\tsetuseropt\tSet user options");
    send_to(eclient,"\tsetchanopt\tSet channel options");
    send_to(eclient,"\tdeletenick\tRemove a nick from the memory");
    send_to(eclient,"\tdropuser\tDrop a user account");
    send_to(eclient,"\tsetnickflag\tSet a \"fake\" umode to a nick");
    send_to(eclient,"\teject\tEject someone from the partyline");
    send_to(eclient,"\tclose\tClose all unknown connections");
    send_to(eclient,"\tcanpl\tSet/remove canpl flag");
    send_to(eclient,"\traw\tSend a raw message to the irc server");
    send_to(eclient,"\texec\tExecute a command");
    }
    send_to(eclient,"End of help.");
}

void p_kill (Eclient *eclient, User *user __unused, char *command, char *tail)
{
    CheckAndLog();
    char *nick,*reason;
    nick = tail;
    reason = SeperateWord(nick);
    
    if (!nick || *nick == '\0' || !reason || *reason == '\0') {
        send_to(eclient,"Syntax: kill nick reason"); 
        return;
    }   
    
    killuser(nick,reason,me.nick);
    send_to(eclient,"Done.");
}

void p_modlist (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_root) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    Module *mod;
    send_to(eclient,"Modules list :");
    LIST_FOREACH_ALL(module_list, mod)
        send_to(eclient,"   %s",mod->modname);
    send_to(eclient,"End of list.");
}

void p_modload (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_root) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *mod = tail;
    SeperateWord(mod);
    
    if (!mod || *mod == '\0') {
        send_to(eclient,"Syntax: modload modname");
        return;
    }   
    
    if (find_module(mod)) {
        send_to(eclient,"Module %s already loaded",mod);
        return;
    }   
    
    if (!loadmodule(mod)) {
        send_to(eclient,"Cannot load module %s",mod);
        return;
    }   
    
    send_to(eclient,"Module %s loaded",mod);
    globops("%s loaded module \2%s\2",user->nick,mod);
    operlog("Module %s loaded by %s",mod,user->nick);
}

void p_modunload (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_root) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *mod = tail;
    SeperateWord(mod);
    
    if (!mod || *mod == '\0') {
        send_to(eclient,"Syntax: modunload modname");
        return;
    }   
    
    if (!unloadmodule(mod)) {
        send_to(eclient,"Cannot unload %s",mod);
        return;
    }   
    
    send_to(eclient,"Module %s unloaded",mod);
    globops("%s unloaded module \2%s\2",user->nick,mod);
    operlog("Module %s unloaded by %s",mod,user->nick);
}

void p_nicklist (Eclient *eclient, User *user __unused, char *command, char *tail)
{
    CheckAndLog();
    char *pattern = tail;
    SeperateWord(pattern);
    
    int count=0;
    Nick *nptr;
    LIST_FOREACH_ALL(nick_list, nptr) {
        if (pattern && *pattern != '\0') {
            if (Strstr(nptr->nick,pattern) || Strstr(nptr->ident,pattern) || Strstr(nptr->host,pattern)) {
                send_to(eclient,"\t%s\t%s@%s",nptr->nick,nptr->ident,nptr->host);
                count++;
            }
        } else {
            send_to(eclient,"\t%s\t%s@%s",nptr->nick,nptr->ident,nptr->host);
            count++;
        }
    }
    send_to(eclient,"End of list (%d entries).",count);
}

void p_quit (Eclient *eclient)
{
    CheckPAuth();
    send_to(eclient,"Bye");
    sendto_all_butone(eclient,"*** User %s (%s) logged out (Client Quit)",eclient->nick,eclient->host);
    close(eclient->fd);
    DeleteEclient(eclient);
}

void p_raw (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    if (!tail || *tail == '\0') {
        send_to(eclient,"Syntax: raw message");
        return;
    }   
    
    SendRaw(tail);
    send_to(eclient,"Done");
}

void p_rehash (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_root) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    loadconf(1);
    send_to(eclient,"Configuration rehashed");
}

void p_restart (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_root) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    SendRaw(":%s QUIT :Restart ordered by %s",me.nick,eclient->nick);
    operlog("%s executed RESTART from partyline",eclient->nick);
    sendto_all("*** %s RESTARTED the services!",eclient->nick);
    child_restart(1);
}

void p_savedb (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_root) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    savealldb();
    send_to(eclient,"Databases saved");
}

void p_set (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *name,*param;
    name = tail;
    param = SeperateWord(name);
    SeperateWord(param);
    
    if (!name || *name == '\0') {
        send_to(eclient,"Configuration variables list :");
        send_to(eclient,"\tnick:\t%s",me.nick);
        send_to(eclient,"\tname:\t%s",me.name);
        send_to(eclient,"\tident:\t%s",me.ident);
        send_to(eclient,"\thost:\t%s",me.host);
        send_to(eclient,"\tserver:\t%s",me.server);
        send_to(eclient,"\tport:\t%d",me.port);
        send_to(eclient,"\tlinkpass:\t%s",me.linkpass);
        send_to(eclient,"\tmaxclones:\t%d",me.maxclones);
        send_to(eclient,"\tbindip:\t%s",me.bindip);
        send_to(eclient,"\tnick_expire:\t%d",me.nick_expire);
        send_to(eclient,"\tchan_expire:\t%d",me.chan_expire);
        send_to(eclient,"\tlevel_oper:\t%d",me.level_oper);
        send_to(eclient,"\tlevel_admin:\t%d",me.level_admin);
        send_to(eclient,"\tlevel_root:\t%d",me.level_root);
        send_to(eclient,"\tlevel_owner:\t%d",me.level_owner);
        send_to(eclient,"\tmysql_host:\t%s",me.mysql_host);
        send_to(eclient,"\tmysql_db:\t%s",me.mysql_db);
        send_to(eclient,"\tmysql_login:\t%s",me.mysql_login);
        send_to(eclient,"\tmysql_passwd:\t%s",me.mysql_passwd);
        send_to(eclient,"\tlogfile:\t%s",me.logfile);
        send_to(eclient,"\tlimittime:\t%d",me.limittime);
        send_to(eclient,"\tsavedb_interval:\t%d",me.savedb_interval);
#ifdef USE_GNUTLS
        send_to(eclient,"\tssl:\t%d",me.ssl);
#endif          
        send_to(eclient,"\tguest_prefix:\t%s",me.guest_prefix);
        send_to(eclient,"\tlisten_port:\t%d",me.listen_port);
        send_to(eclient,"\tpl_logfile:\t%s",me.pl_logfile);
        send_to(eclient,"\tanonymous_global:\t%d",me.anonymous_global);
        send_to(eclient,"\tsendmail:\t%s",me.sendmail);
        send_to(eclient,"\tsendfrom:\t%s",me.sendfrom);
        send_to(eclient,"\tmaxmsgtime:\t%d",me.maxmsgtime);
        send_to(eclient,"\tmaxmsgnb:\t%d",me.maxmsgnb);
        send_to(eclient,"\tignoretime:\t%d",me.ignoretime);
        send_to(eclient,"\tmaxloginatt:\t%d",me.maxloginatt);
        send_to(eclient,"\tchlev_sadmin:\t%d",me.chlev_sadmin);
        send_to(eclient,"\tchlev_admin:\t%d",me.chlev_admin);
        send_to(eclient,"\tchlev_op:\t%d",me.chlev_op);
        send_to(eclient,"\tchlev_halfop:\t%d",me.chlev_halfop);
        send_to(eclient,"\tchlev_voice:\t%d",me.chlev_voice);
        send_to(eclient,"\tchlev_nostatus:\t%d",me.chlev_nostatus);
        send_to(eclient,"\tchlev_akick:\t%d",me.chlev_akick);
        send_to(eclient,"\tchlev_akb:\t%d",me.chlev_akb);
        send_to(eclient,"\tchlev_invite:\t%d",me.chlev_invite);
#ifdef USE_FILTER
        send_to(eclient,"\tfilter:\t%d",me.filter);
#endif
        send_to(eclient,"End of list");

        return;
    }

    if (!param || *param == '\0') {
        send_to(eclient,"Syntax: set [varname param]");
        return;
    }

    if (!Strcmp(name,"nick"))
        strncpy(me.nick,param,32);
    else if (!Strcmp(name,"name"))
        strncpy(me.name,param,32);
    else if (!Strcmp(name,"ident"))
        strncpy(me.ident,param,32);
    else if (!Strcmp(name,"host"))
        strncpy(me.host,param,32);
    else if (!Strcmp(name,"server"))
        strncpy(me.server,param,32);
    else if (!Strcmp(name,"port"))
        me.port = atoi(param);
    else if (!Strcmp(name,"linkpass"))
        strncpy(me.linkpass,param,32);
    else if (!Strcmp(name,"maxclones"))
        me.maxclones = atoi(param);
    else if (!Strcmp(name,"bindip"))
        strncpy(me.bindip,param,32);
    else if (!Strcmp(name,"nick_expire"))
        me.nick_expire = atoi(param);
    else if (!Strcmp(name,"chan_expire"))
        me.chan_expire = atoi(param);
    else if (!Strcmp(name,"level_oper"))
        me.level_oper = atoi(param);
    else if (!Strcmp(name,"level_admin"))
        me.level_admin = atoi(param);
    else if (!Strcmp(name,"level_root"))
        me.level_root = atoi(param);
    else if (!Strcmp(name,"level_owner"))
        me.level_owner = atoi(param);
    else if (!Strcmp(name,"mysql_host"))
        strncpy(me.mysql_host,param,32);
    else if (!Strcmp(name,"mysql_db"))
        strncpy(me.mysql_db,param,32);
    else if (!Strcmp(name,"mysql_login"))
        strncpy(me.mysql_login,param,32);
    else if (!Strcmp(name,"mysql_passwd"))
        strncpy(me.mysql_passwd,param,32);
    else if (!Strcmp(name,"limittime"))
        me.limittime = atoi(param);
    else if (!Strcmp(name,"savedb_interval"))
        me.savedb_interval = atoi(param);
#ifdef USE_GNUTLS
    else if (!Strcmp(name,"ssl"))
        me.ssl = atoi(param);
#endif
    else if (!Strcmp(name,"guest_prefix"))
        strncpy(me.guest_prefix,param,32);
    else if (!Strcmp(name,"listen_port"))
        me.listen_port = atoi(param);
    else if (!Strcmp(name,"anonymous_global"))
        me.anonymous_global = atoi(param);
    else if (!Strcmp(name,"sendmail"))
        strncpy(me.sendmail,param,128);
    else if (!Strcmp(name,"sendfrom"))
        strncpy(me.sendfrom,param,128);
    else if (!Strcmp(name,"maxmsgtime"))
        me.maxmsgtime = atoi(param);
    else if (!Strcmp(name,"maxmsgnb"))
        me.maxmsgnb = atoi(param);
    else if (!Strcmp(name,"ignoretime"))
        me.ignoretime = atoi(param);
    else if (!Strcmp(name,"maxloginatt"))
        me.maxloginatt = atoi(param);
    else if (!Strcmp(name,"chlev_sadmin"))
        me.chlev_sadmin = atoi(param);
    else if (!Strcmp(name,"chlev_admin"))
        me.chlev_admin = atoi(param);
    else if (!Strcmp(name,"chlev_op"))
        me.chlev_op = atoi(param);
    else if (!Strcmp(name,"chlev_halfop"))
        me.chlev_halfop = atoi(param);
    else if (!Strcmp(name,"chlev_voice"))
        me.chlev_voice = atoi(param);
    else if (!Strcmp(name,"chlev_nostatus"))
        me.chlev_nostatus = atoi(param);
    else if (!Strcmp(name,"chlev_akick"))
        me.chlev_akick = atoi(param);
    else if (!Strcmp(name,"chlev_akb"))
        me.chlev_akb = atoi(param);
    else if (!Strcmp(name,"chlev_invite"))
        me.chlev_invite = atoi(param);
#ifdef USE_FILTER
    else if (!Strcmp(name,"filter"))
        me.filter = atoi(param);
#endif
    else {
        send_to(eclient,"Unknown varname %s",name);
        return;
    }

    send_to(eclient,"Done.");
}

void p_setchan (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *chan,*name,*param;
    chan = tail;
    name = SeperateWord(chan);
    param = SeperateWord(name);
    char blah[1024];
    bzero(blah,1024);
    SeperateWord(param);
    
    if (!chan || *chan == '\0' || !name || *name == '\0' || !param || *param == '\0') {
        send_to(eclient,"Syntax: setchan channel varname param");
        return;
    }

    strncpy(blah,param,1024);

    Chan *chptr;
    chptr = find_channel(chan);
    if (!chptr) {
        send_to(eclient,"This channel does not exist");
        return;
    }   
    
    if (!Strcmp(name,"channelname"))
        strncpy(chptr->channelname,param,CHANLEN);
    else if (!Strcmp(name,"owner"))
        strncpy(chptr->owner,param,CHANLEN);
    else if (!Strcmp(name,"entrymsg"))
        strncpy(chptr->entrymsg,blah,250);
    else if (!Strcmp(name,"mlock"))
        strncpy(chptr->mlock,blah,50);
    else if (!Strcmp(name,"autolimit"))
        chptr->autolimit = atoi(param);
    else if (!Strcmp(name,"lastseen"))
        chptr->lastseen = atoi(param);
    else {
        send_to(eclient,"Unknown varname %s",name);
        return;
    }

    send_to(eclient,"Done.");
}

void p_setchanopt (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *chan,*name,*what;
    chan = tail;
    name = SeperateWord(chan);
    what = SeperateWord(name);
    SeperateWord(what);
    
    if (!chan || *chan == '\0' || !name || *name == '\0' || !what || *what == '\0') {
        send_to(eclient,"Syntax: setchanopt channel varname <0|1>"); 
        return;
    }   
    
    Chan *chptr;
    chptr = find_channel(chan);
    if (!chptr) {
        send_to(eclient,"This channel does not exist");
        return;
    }   
    
    if (!Strcmp(name,"nojoin")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_NOJOIN);
        else
            ClearOption(chptr, COPT_NOJOIN);
    } else if (!Strcmp(name,"noauto")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_NOAUTO);
        else
            ClearOption(chptr, COPT_NOAUTO);
    } else if (!Strcmp(name,"avoice")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_AVOICE);
        else
            ClearOption(chptr, COPT_AVOICE);
    } else if (!Strcmp(name,"private")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_PRIVATE);
        else
            ClearOption(chptr, COPT_PRIVATE);
    } else if (!Strcmp(name,"strictop")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_STRICTOP);
        else
            ClearOption(chptr, COPT_STRICTOP);
    } else if (!Strcmp(name,"aop")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_AOP);
        else
            ClearOption(chptr, COPT_AOP);
    } else if (!Strcmp(name,"secure")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_SECURE);
        else
            ClearOption(chptr, COPT_SECURE);
    } else if (!Strcmp(name,"suspended")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_SUSPENDED);
        else
            ClearOption(chptr, COPT_SUSPENDED);
    } else if (!Strcmp(name,"noexpire")) {
        if (atoi(what) == 1)
            SetOption(chptr, COPT_NOEXPIRE);
        else
            ClearOption(chptr, COPT_NOEXPIRE);
    } else {
        send_to(eclient,"Unknown varname %s",name);
        return;
    }

    send_to(eclient,"Done.");
}

void p_setnickflag (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *nick,*flag,*what;
    nick = tail;
    flag = SeperateWord(nick);
    what = SeperateWord(flag);
    SeperateWord(what);
    
    if (!nick || *nick == '\0' || !flag || *flag == '\0' || !what || *what == '\0') {
        send_to(eclient,"Syntax: setnickflag nick flag <0|1>");
        return;
    }   
    
    Nick *nptr;
    nptr = find_nick(nick);
    if (!nptr) {
        send_to(eclient,"This nick does not exist");
        return;
    }   
    
    if (!Strcmp(flag,"oper")) {
        if (atoi(what) == 1)
            SetOper(nptr);
        else
            ClearOper(nptr);
    } else if (!Strcmp(flag,"admin")) {
        if (atoi(what) == 1)
            SetAdmin(nptr);
        else
            ClearAdmin(nptr);
    } else if (!Strcmp(flag,"sadmin")) {
        if (atoi(what) == 1)
            SetSAdmin(nptr);
        else
            ClearSAdmin(nptr);
    } else if (!Strcmp(flag,"nadmin")) {
        if (atoi(what) == 1)
            SetNAdmin(nptr);
        else
            ClearNAdmin(nptr);
    } else if (!Strcmp(flag,"bot")) {
        if (atoi(what) == 1)
            SetBot(nptr);
        else
            ClearBot(nptr);
    } else if (!Strcmp(flag,"service")) {
        if (atoi(what) == 1)
            SetService(nptr);
        else
            ClearService(nptr);
    } else if (!Strcmp(flag,"registered")) {
        if (atoi(what) == 1)
            SetRegistered(nptr);
        else
            ClearRegistered(nptr);
    } else {
        send_to(eclient,"Unknown flag %s",flag);
        return;
    }

    send_to(eclient,"Done.");
}

void p_setuser (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *nick,*name,*param;
    nick = tail;
    name = SeperateWord(nick);
    param = SeperateWord(name);
    SeperateWord(param);
    
    if (!nick || *nick == '\0' || !name || *name == '\0' || !param || *param == '\0') {
        send_to(eclient,"Syntax: setuser username varname param"); 
        return;
    }   
    
    User *uptr;
    uptr = find_user(nick);
    if (!uptr) {
        send_to(eclient,"This user does not exist");
        return;
    }   
    
    if (!Strcmp(name,"nick"))
        strncpy(uptr->nick,param,NICKLEN);
    else if (!Strcmp(name,"email"))
        strncpy(uptr->email,param,EMAILLEN);
    else if (!Strcmp(name,"authed"))
        uptr->authed = atoi(param);
    else if (!Strcmp(name,"level"))
        uptr->level = atoi(param);
    else if (!Strcmp(name,"md5_pass"))
        strncpy(uptr->md5_pass,param,NICKLEN);
    else if (!Strcmp(name,"lastseen"))
        uptr->lastseen = atoi(param);
    else if (!Strcmp(name,"timeout"))
        uptr->timeout = atoi(param);
    else if (!Strcmp(name,"vhost"))
        strncpy(uptr->vhost,param,HOSTLEN);
    else {
        send_to(eclient,"Unknown varname %s",name);
        return;
    }   
    
    send_to(eclient,"Done.");
}

void p_setuseropt (Eclient *eclient, User *user, char *command, char *tail)
{
    CheckAndLog();
    if (user->level < me.level_owner) {
        send_to(eclient,"Access denied");
        return;
    }   
    
    char *nick,*name,*what;
    nick = tail;
    name = SeperateWord(nick);
    what = SeperateWord(name);
    SeperateWord(what);
    
    if (!nick || *nick == '\0' || !name || *name == '\0' || !what || *what == '\0') {
        send_to(eclient,"Syntax: setuseropt username optname <0|1>");
        return;
    }   
    
    User *uptr;
    uptr = find_user(nick);
    if (!uptr) {
        send_to(eclient,"This user does not exist");
        return;
    }   
    
    if (!Strcmp(name,"protect")) {
        if (atoi(what) == 1)
            SetOption(uptr, UOPT_PROTECT);
        else
            ClearOption(uptr, UOPT_PROTECT);
    } else if (!Strcmp(name,"private")) {
        if (atoi(what) == 1)
            SetOption(uptr, UOPT_PRIVATE);
        else
            ClearOption(uptr, UOPT_PRIVATE);
    } else if (!Strcmp(name,"suspended")) {
        if (atoi(what) == 1)
            SetOption(uptr, UOPT_SUSPENDED);
        else
            ClearOption(uptr, UOPT_SUSPENDED);
    } else if (!Strcmp(name,"noexpire")) {
        if (atoi(what) == 1)
            SetOption(uptr, UOPT_NOEXPIRE);
        else
            ClearOption(uptr, UOPT_NOEXPIRE);
    } else if (!Strcmp(name,"noauto")) {
        if (atoi(what) == 1)
            SetOption(uptr, UOPT_NOAUTO);
        else
            ClearOption(uptr, UOPT_NOAUTO);
    } else if (!Strcmp(name,"canpl")) {
        if (atoi(what) == 1)
            SetOption(uptr, UOPT_CANPL);
        else
            ClearOption(uptr, UOPT_CANPL);
    } else if (!Strcmp(name,"hidemail")) {
        if (atoi(what) == 1)
            SetOption(uptr, UOPT_HIDEMAIL);
        else
            ClearOption(uptr, UOPT_HIDEMAIL);
    } else {
        send_to(eclient,"Unknown varname %s",name);
        return;
    }

    send_to(eclient,"Done.");
}

void p_who (Eclient *eclient, User *user __unused, char *command, char *tail)
{
    CheckAndLog();
    send_to(eclient,"Userlist :");
    Eclient *ecl;
    LIST_FOREACH_ALL(eclient_list, ecl)
        send_to(eclient,"\t%s\t(%s)",(ecl->authed == 1) ? ecl->nick : "(Unknown)",ecl->host);
    send_to(eclient,"End of list");
}
