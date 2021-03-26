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


#include "child.h"
#include "config.h"
#include "commands.h"
#include "core.h"
#include "db.h"
#include "filter.h"
#include "hashmap.h"
#include "modules.h"
#include "net.h"
#include "string_utils.h"
#include "trust.h"
#include "user.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef USE_FILTER
extern rulelist rule_list;
#endif

void do_oper (Nick *, User *, char *);
void do_help (Nick *, User *, char *);
void oper_nicklist (Nick *, User *, char *);
void oper_userlist (Nick *, User *, char *);
void oper_chanlist (Nick *, User *, char *);
void oper_killall (Nick *, User *, char *);
void oper_glineall (Nick *, User *, char *);
void oper_fakeuser (Nick *, User *, char *);
void oper_fakenick (Nick *, User *, char *);
void oper_forceauth (Nick *, User *, char *);
void oper_trustadd (Nick *, User *, char *);
void oper_trustlist (Nick *);
void oper_trustdel (Nick *, User *, char *);
void oper_massfakeuser (Nick *, User *, char *);
void oper_massfakejoin (Nick *, User *, char *);
void oper_massfakekill (Nick *, User *, char *);
void oper_modload (Nick *, User *, char *);
void oper_modunload (Nick *, User *, char *);
void oper_modlist (Nick *);
void oper_fakejoin (Nick *, User *, char *);
void oper_jupe (Nick *, User *, char *);
void oper_fakekill (Nick *, User *, char *);
void oper_fakesay (Nick *, User *, char *);
void oper_operlist (Nick *);
void oper_stats (Nick *);
void oper_changelev (Nick *, User *, char *);
void oper_global (Nick *, User *, char *);
void oper_sglobal (Nick *, User *, char *);
void oper_raw (Nick *, User *, char *);
void oper_savedb (Nick *);
void oper_setraws (Nick *, User *, char *);
void oper_rehash (Nick *);
void oper_restart (Nick *, User *, char *);
void oper_die (Nick *, User *, char *);
void oper_noexpire (Nick *, User *, char *);
void oper_suspend (Nick *, User *, char *);
void oper_cmdlev (Nick *, User *, char *);
#ifdef USE_FILTER
void oper_reloadrules (Nick *);
void oper_ruleslist (Nick *);
void oper_setfilter (Nick *, User *, char *);
#endif
void oper_superadmin (Nick *, User *, char *);
void oper_fakelist (Nick *);
void oper_glinechan (Nick *, User *, char *);
void oper_regexpcheck (Nick *, User *, char *);

void child_init(Module *module)
{
    module->nodreload = 1;
    addBaseCommand("oper",do_oper,me.level_oper);

    addOperCommand("userlist",oper_userlist,me.level_oper);
    addOperCommand("trustlist",oper_trustlist,me.level_admin);
    addOperCommand("trustdel",oper_trustdel,me.level_admin);
    addOperCommand("trustadd",oper_trustadd,me.level_admin);
    addOperCommand("suspend",oper_suspend,me.level_admin);
    addOperCommand("superadmin",oper_superadmin,me.level_root);
    addOperCommand("stats",oper_stats,me.level_oper);
    addOperCommand("sglobal",oper_sglobal,me.level_oper);
    addOperCommand("setraws",oper_setraws,me.level_owner);
#ifdef USE_FILTER
    addOperCommand("setfilter",oper_setfilter,me.level_root);
#endif
    addOperCommand("savedb",oper_savedb,me.level_root);
#ifdef USE_FILTER
    addOperCommand("ruleslist",oper_ruleslist,me.level_root);
#endif
    addOperCommand("restart",oper_restart,me.level_root);
    addOperCommand("regexpcheck",oper_regexpcheck,me.level_oper);
#ifdef USE_FILTER
    addOperCommand("reloadrules",oper_reloadrules,me.level_root);
#endif
    addOperCommand("rehash",oper_rehash,me.level_root);
    addOperCommand("raw",oper_raw,me.level_root);
    addOperCommand("operlist",oper_operlist,me.level_oper);
    addOperCommand("noexpire",oper_noexpire,me.level_admin);
    addOperCommand("nicklist",oper_nicklist,me.level_oper);
    addOperCommand("modunload",oper_modunload,me.level_root);
    addOperCommand("modload",oper_modload,me.level_root);
    addOperCommand("modlist",oper_modlist,me.level_root);
    addOperCommand("massfakeuser",oper_massfakeuser,me.level_admin);
    addOperCommand("massfakekill",oper_massfakekill,me.level_admin);
    addOperCommand("massfakejoin",oper_massfakejoin,me.level_admin);
    addOperCommand("killall",oper_killall,me.level_admin);
    addOperCommand("glinechan",oper_glinechan,me.level_admin);
    addOperCommand("glineall",oper_glineall,me.level_admin);
    addOperCommand("jupe",oper_jupe,me.level_admin);
    addOperCommand("global",oper_global,me.level_oper);
    addOperCommand("forceauth",oper_forceauth,me.level_root);
    addOperCommand("fakeuser",oper_fakeuser,me.level_oper);
    addOperCommand("fakesay",oper_fakesay,me.level_oper);
    addOperCommand("fakenick",oper_fakenick,me.level_oper);
    addOperCommand("fakelist",oper_fakelist,me.level_oper);
    addOperCommand("fakekill",oper_fakekill,me.level_oper);
    addOperCommand("fakejoin",oper_fakejoin,me.level_oper);
    addOperCommand("die",oper_die,me.level_root);
    addOperCommand("cmdlev",oper_cmdlev,me.level_owner);
    addOperCommand("chanlist",oper_chanlist,me.level_oper);
    addOperCommand("changelev",oper_changelev,me.level_admin);
}

void child_cleanup()
{
    deleteCommand("oper",CMD_BASE,0);

    delOperCommand("nicklist");
    delOperCommand("userlist");
    delOperCommand("chanlist");
    delOperCommand("killall");
    delOperCommand("glineall");
    delOperCommand("fakeuser");
    delOperCommand("fakenick");
    delOperCommand("forceauth");
    delOperCommand("trustadd");
    delOperCommand("trustlist");
    delOperCommand("trustdel");
    delOperCommand("massfakeuser");
    delOperCommand("massfakejoin");
    delOperCommand("massfakekill");
    delOperCommand("modload");
    delOperCommand("modunload");
    delOperCommand("modlist");
    delOperCommand("fakejoin");
    delOperCommand("jupe");
    delOperCommand("fakekill");
    delOperCommand("fakesay");
    delOperCommand("operlist");
    delOperCommand("stats");
    delOperCommand("changelev");
    delOperCommand("global");
    delOperCommand("raw");
    delOperCommand("savedb");
    delOperCommand("setraws");
    delOperCommand("rehash");
    delOperCommand("restart");
    delOperCommand("die");
    delOperCommand("noexpire");
    delOperCommand("suspend");
    delOperCommand("cmdlev");
#ifdef USE_FILTER
    delOperCommand("reloadrules");
    delOperCommand("ruleslist");
    delOperCommand("setfilter");
#endif
    delOperCommand("superadmin");
    delOperCommand("sglobal");
    delOperCommand("fakelist");
    delOperCommand("glinechan");
    delOperCommand("regexpcheck");
}

void do_oper (Nick *nptr, User *uptr, char *all)
{
    char *arg1,*arg2;
    char blah[1024];
    strncpy(blah,all,1024);
    arg1 = blah;
    arg2 = SeperateWord(arg1);
    SeperateWord(arg2);
    
    if (!IsOper(nptr)) {
        NoticeToUser(nptr,"Access denied");
        globops("Access denied to OPER commands from \2%s\2 (non-oper)",nptr->nick);
        return;
    }   
    
    if (!arg2 || *arg2 == '\0') {
        NoticeToUser(nptr,"Type \2/msg %s help oper\2 for more informations",me.nick);
        return;
    }   
    
    if (!Strcmp(arg2,"help")) {
        do_help(nptr,uptr,all);
        return;
    }   
    
    all = SeperateWord(all);
    all = SeperateWord(all);
    
    Command *cmd;
    LLIST_FOREACH_ENTRY(core_get_commands(), cmd, list_head) {
        if (!Strcmp(cmd->name,arg2) && cmd->type == CMD_OPER) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }   
    }   
    
    NoticeToUser(nptr,"Unknown command");
}

void oper_nicklist (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3;
    Nick *nptr2;
    struct hashmap_entry *entry;

    arg3 = all;
    SeperateWord(arg3);

    int count=0;
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_nicks(), entry, nptr2) {
        if (arg3 && *arg3 != '\0') {
            if (Strstr(nptr2->nick,arg3) || Strstr(nptr2->ident,arg3) || Strstr(nptr2->host,arg3)) {
                NoticeToUser(nptr,"\2%s\2!%s@%s",nptr2->nick,nptr2->ident,nptr2->host);
                count++;
            }
        } else {
            NoticeToUser(nptr,"\2%s\2!%s@%s",nptr2->nick,nptr2->ident,nptr2->host);
            count++;
        }
    }
    NoticeToUser(nptr,"End of list (%d entries).",count);
}

void oper_userlist (Nick *nptr, User *uptr __unused, char *all)
{
    User *uptr2;
    Nick *nptr2;
    char *arg3;
    struct hashmap_entry *entry;

    arg3 = all;
    SeperateWord(arg3);

    char tmp[1024];
    bzero(tmp,1024);

    int count = 0;
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_users(), entry, uptr2) {
        if (arg3 && *arg3 != '\0') {
            if (Strstr(uptr2->nick,arg3) || Strstr(uptr2->vhost,arg3)) {
                nptr2 = find_nick(uptr2->nick);
                if (nptr2) {
                    if (uptr2->authed == 1)
                        sprintf(tmp,"%s     %d     Authed      %s!%s@%s",uptr2->nick,uptr2->level,nptr2->nick,nptr2->ident,nptr2->host);
                    else
                        sprintf(tmp,"%s     %d     Not authed  %s!%s@%s",uptr2->nick,uptr2->level,nptr2->nick,nptr2->ident,nptr2->host);
                } else
                    sprintf(tmp,"%s     %d      Not authed, not online",uptr2->nick,uptr2->level);
                NoticeToUser(nptr,tmp);
                count++;
            }
        } else {
            nptr2 = find_nick(uptr2->nick);
            if (nptr2) {
                if (uptr2->authed == 1)
                    sprintf(tmp,"%s     %d      Authed      %s!%s@%s",uptr2->nick,uptr2->level,nptr2->nick,nptr2->ident,nptr2->host);
                else
                    sprintf(tmp,"%s     %d      Not authed  %s!%s@%s",uptr2->nick,uptr2->level,nptr2->nick,nptr2->ident,nptr2->host);
            } else
                sprintf(tmp,"%s     %d      Not authed, not online",uptr2->nick,uptr2->level);
            NoticeToUser(nptr,tmp);
            count++;
        }
    }
    NoticeToUser(nptr,"End of list (%d entries).",count);
}

void oper_chanlist (Nick *nptr, User *uptr __unused, char *all)
{
    struct hashmap_entry *entry;
    Chan *chptr;
    char *arg3 = all;
    SeperateWord(arg3);

    int count = 0;

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_chans(), entry, chptr) {
        if (arg3 && *arg3 != '\0') {
            if (Strstr(chptr->channelname,arg3) || Strstr(chptr->owner,arg3)) {
                NoticeToUser(nptr,"%s     %s",chptr->channelname,chptr->owner);
                count++;
            }
        } else {
            NoticeToUser(nptr,"%s     %s",chptr->channelname,chptr->owner);
            count++;
        }
    }
    NoticeToUser(nptr,"End of list (%d entries).",count);
}

void oper_killall (Nick *nptr, User *uptr __unused, char *all)
{
    struct hashmap_entry *entry, *tmp_entry;
    Nick *nptr2;
    char *arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2killall \037mask\037\2");
        return;
    }   

    if (!IsMask(arg3)) {
        NoticeToUser(nptr,"Error: malformed mask");
        return;
    }   

    char mask[256];
    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_nicks(), entry, tmp_entry, nptr2) {
        snprintf(mask, 256, "%s!%s@%s",nptr2->nick,nptr2->ident,nptr2->host);
        if (match_mask(arg3,mask)) {
            killuser(nptr2->nick,"Clearing users",me.nick);
        }
    }

    NoticeToUser(nptr,"Done.");
}

void oper_regexpcheck (Nick *nptr, User *uptr __unused, char *all)
{
    Nick *nptr2;
    unsigned int i = 0;
    struct hashmap_entry *entry;

    SeperateWord(all);

    if (!all || *all == '\0') {
        NoticeToUser(nptr, "Syntax: \2regexpcheck \037mask\037\2");
        return;
    }

    if (!IsMask(all)) {
        NoticeToUser(nptr, "Error: malformed mask");
        return;
    }

    char mask[256];
    NoticeToUser(nptr, "Affected users :");
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_nicks(), entry, nptr2) {
        snprintf(mask, 256, "%s!%s@%s", nptr2->nick, nptr2->ident, nptr2->host);
        if (match_mask(all, mask)) {
            NoticeToUser(nptr, "\2%s\2      (%s@%s)", nptr2->nick, nptr2->ident, nptr2->host);
            i++;
        }
    }
    NoticeToUser(nptr, "Total affected users: \2%d\2.", i);
}

void oper_glineall (Nick *nptr, User *uptr __unused, char *all)
{
    struct hashmap_entry *entry, *tmp_entry;
    Nick *nptr2;
    char *arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2glineall \037mask\037\2");
        return;
    }

    if (!IsMask(arg3)) {
        NoticeToUser(nptr,"Error: malformed mask");
        return;
    }

    char mask[256];
    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_nicks(), entry, tmp_entry, nptr2) {
        snprintf(mask, 256, "%s!%s@%s",nptr2->nick,nptr2->ident,nptr2->host);
        if (match_mask(arg3,mask)) {
            glineuser("*", nptr2->host, 86400, "Clearing users");
        }
    }

    NoticeToUser(nptr,"Done.");
}

void oper_glinechan (Nick *nptr, User *uptr __unused, char *all)
{
    Wchan *wchan;
    Member *member;
    char *chname, *duration, *reason;
    chname = all;
    duration = SeperateWord(chname);
    reason = SeperateWord(duration);

    if (!chname || *chname == '\0' || !duration || *duration == '\0' || !reason || *reason == '\0') {
        NoticeToUser(nptr, "Syntax: \2glinechan \037#channel\037 \037duration (in seconds, 0 for perm)\037 \037reason\037");
        return;
    }

    if ((wchan = find_wchan(chname)) == NULL) {
        NoticeToUser(nptr, "This channel does not exist.");
        return;
    }

    LLIST_FOREACH_ENTRY(&wchan->members, member, wchan_head) {
        if (!IsOper(member->nick))
            glineuser("*", member->nick->host, atoi(duration), reason);
    }

    NoticeToUser(nptr, "Done.");
}

void oper_fakeuser (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3,*arg4,*arg5;
    Fake *fake;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    arg5 = SeperateWord(arg4);
    SeperateWord(arg5);

    if (!arg3 || !arg4 || !arg5 || *arg5 == '\0' || *arg4 == '\0' || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2fakeuser \037nick\037 \037ident\037 \037host\037\2");
        return;
    }   
        
    if (!Strcmp(arg3,me.nick)) {
        NoticeToUser(nptr,"That's my nick you sucker.");
        return;
    }

    if ((fake = find_fake(arg3)) != NULL) {
        NoticeToUser(nptr, "This fakeuser already exists.");
        return;
    }

    fakeuser(arg3, arg4, arg5, NULL, "i");
}

void oper_fakenick (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3,*arg4;
    Fake *fake;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2fakenick \037oldnick\037 \037newnick\037\2");
        return;
    }

    if ((fake = find_fake(arg3)) == NULL) {
        NoticeToUser(nptr, "This fakeuser does not exist.");
        return;
    }

    if (find_fake(arg4) != NULL) {
        NoticeToUser(nptr, "New nick is already held by another fakeuser.");
        return;
    }

    HASHMAP_ERASE(core_get_fakeusers(), fake->nick);
    strncpy(fake->nick, arg4, NICKLEN);
    HASHMAP_INSERT(core_get_fakeusers(), fake->nick, fake, NULL);
    SendRaw(":%s NICK %s %ld",arg3,arg4,time(NULL));
}

void oper_fakelist (Nick *nptr)
{
    struct hashmap_entry *entry;
    Fake *fake;

    NoticeToUser(nptr, "Fakeusers list :");
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_fakeusers(), entry, fake) {
        NoticeToUser(nptr, "\2%s\2 (%s@%s)", fake->nick, fake->ident, fake->host);
    }
    NoticeToUser(nptr, "End of list.");
}

void oper_forceauth (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3;
    User *uptr2;
    arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2forceauth \037nick\037\2");
        return;
    }   
        
    if (!find_nick(arg3)) {
        NoticeToUser(nptr,"This nick does not exist");
        return;
    }   
        
    uptr2 = find_user(arg3);
    if (!uptr2) {
        NoticeToUser(nptr,"This nick is not registered");
        return;
    }   
        
    uptr2->authed = 1;
    uptr2->lastseen = time(NULL);
    SendRaw("SVSMODE %s +r",arg3);
    NoticeToUser(nptr,"\2%s\2 has been authed.",arg3);
    operlog("%s force-authed %s",nptr->nick,arg3);
    if (HasOption(uptr2, UOPT_PROTECT)) DeleteGuest(arg3);
}

void oper_trustadd (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3,*arg4;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2trustadd \037hostname\037 \037limit\037\2");
        return;
    }   
        
    Trust *trust;
    trust = find_trust_strict(arg3);
    if (trust) {
        trust->limit = strtol(arg4,NULL,10);
        NoticeToUser(nptr,"The limit of the trust on %s is now %s",arg3,arg4);
        operlog("%s set limit for trust %s to %s",nptr->nick,arg3,arg4);
    } else {
        AddTrust(arg3,strtol(arg4,NULL,10));
        NoticeToUser(nptr,"Trust for %s added (max %s clones)",arg3,arg4);
        operlog("%s added trust for %s (%s clones)",nptr->nick,arg3,arg4);
    }
}

void oper_trustlist (Nick *nptr)
{
    NoticeToUser(nptr,"Trustlist :");
    Trust *trust;
    struct hashmap_entry *entry;

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_trusts(), entry, trust) {
        NoticeToUser(nptr,"%s         %d",trust->host,trust->limit);
    }
    NoticeToUser(nptr,"End of list.");
}

void oper_trustdel (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

        
    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2trustdel \037hostname\037\2");
        return;
    }   
        
    Trust *trust = find_trust_strict(arg3);
    if (!trust)
        NoticeToUser(nptr,"There is no trust on %s",arg3);
    else {
        DeleteTrust(trust);
        NoticeToUser(nptr,"Trust for %s removed",arg3);
        operlog("%s removed trust for %s",nptr->nick,arg3);
    }   
}

void oper_massfakeuser (Nick *nptr, User *uptr __unused, char *all)
{
    int i;
    char *arg3,*arg4,*arg5;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    arg5 = SeperateWord(arg4);
    SeperateWord(arg5);

        
    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0' || !arg5 || *arg5 == '\0') {
        NoticeToUser(nptr,"Syntax: \2massfakeuser \037prefix\037 \037number-start\037 \037number-end\037\2");
        return;
    }   
        
    int start = strtol(arg4,NULL,10);
    int end = strtol(arg5,NULL,10);
    int howmanyfakes = end - start;
    char name[1024]; 
        
    if (howmanyfakes > 1000 && !IsSAdmin(nptr)) {
        NoticeToUser(nptr,"Too many clones. 1000 max. Or be a service admin.");
        return;
    }   
        
    for(i=start;i<=end;i++) {
        sprintf(name,"%s%d",arg3,i);
        fakeuser(name, name, me.host, NULL, "i");
    }   
        
    operlog("%s executed MASSFAKEUSER :%d clones (%s)",nptr->nick,howmanyfakes,arg3);
    NoticeToUser(nptr,"Done.");
}

void oper_massfakejoin (Nick *nptr, User *uptr __unused, char *all)
{
    int i;
    char *arg3,*arg4,*arg5,*arg6;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    arg5 = SeperateWord(arg4);
    arg6 = SeperateWord(arg5);
    SeperateWord(arg6);

        
    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0' || !arg5 || *arg5 == '\0' || !arg6 || *arg6 == '\0' || *arg6 != '#') {
        NoticeToUser(nptr,"Syntax: \2massfakejoin \037prefix\037 \037number-start\037 \037number-end\037 \037#channel\037\2");
        return;
    }   
        
    int start = strtol(arg4,NULL,10);
    int end = strtol(arg5,NULL,10);
       
    int howmanyfakes = end - start;
    char name[1024]; 
       
    if (howmanyfakes > 1000 && !IsSAdmin(nptr)) {
        NoticeToUser(nptr,"Too many clones. 1000 max. Or be a service admin.");
        return;
    }   
        
    for (i=start;i<=end;i++) {
        sprintf(name,"%s%d",arg3,i);
        fakejoin(name,arg6);
    }   
        
    NoticeToUser(nptr,"Done.");
    operlog("%s executed MASSFAKEJOIN :%d clones (%s,%s)",nptr->nick,howmanyfakes,arg3,arg6);
}

void oper_massfakekill (Nick *nptr, User *uptr __unused, char *all)
{
    int i;
    char *arg3,*arg4,*arg5;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    arg5 = SeperateWord(arg4);
    all = SeperateWord(arg5);


    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0' || !arg5 || *arg5 == '\0') {
        NoticeToUser(nptr,"Syntax: \2massfakekill \037prefix\037 \037number-start\037 \037number-end\037 [\037reason\037]\2");
        return;
    }   
        
    int start = strtol(arg4,NULL,10);
    int end = strtol(arg5,NULL,10);
        
    int howmanyfakes = end - start;
    char name[1024]; 
        
    if (howmanyfakes > 1000 && !IsSAdmin(nptr)) {
        NoticeToUser(nptr,"Too many clones. 1000 max. Or be a service admin.");
        return;
    }   
        
    for(i=start;i<=end;i++) {
        sprintf(name,"%s%d",arg3,i);
        fakekill(name,all);
    }   
        
    NoticeToUser(nptr,"Done.");
    operlog("%s executed MASSFAKEKILL :%d clones (%s,%s)",nptr->nick,howmanyfakes,arg3,all);
}

void oper_modload (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2modload \037modname\037\2");
        return;
    }   
        
    if (find_module(arg3)) {
        NoticeToUser(nptr,"Module %s already loaded",arg3);
        return;
    }   
        
    if (!loadmodule(arg3)) {
        NoticeToUser(nptr,"Cannot load module %s",arg3);
        return;
    } else {
        NoticeToUser(nptr,"Module %s loaded",arg3);
        globops("%s loaded module \2%s\2",nptr->nick,arg3);
    }
    operlog("Module %s loaded by %s",arg3,nptr->nick);
}

void oper_modunload (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2modunload \037modname\037\2");
        return;
    }   

    Module *mod;
    mod = find_module(arg3);
    if (!mod) {
        NoticeToUser(nptr,"Module %s is not loaded",arg3);
        return;
    }

    if (mod->nodreload == 1) {
        NoticeToUser(nptr,"Cannot unload %s through modunload command. Please rehash instead.",arg3);
        return;
    }

    if (!unloadmodule(arg3)) {
        NoticeToUser(nptr,"Cannot unload %s",arg3);
        return;
    }

    NoticeToUser(nptr,"Module %s unloaded",arg3);
    globops("%s unloaded module \2%s\2",nptr->nick,arg3);
    operlog("Module %s unloaded by %s",arg3,nptr->nick);
}

void oper_modlist (Nick *nptr)
{
    Module *mod;
    struct hashmap_entry *entry;

    NoticeToUser(nptr,"Modules list :");
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_modules(), entry, mod) {
        NoticeToUser(nptr,"   \2%s\2",mod->modname);
    }
    NoticeToUser(nptr,"End of list. (%d entries)", HASHMAP_SIZE(core_get_modules()));
}

void oper_fakejoin (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3,*arg4;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2fakejoin \037nick\037 \037#channel\037\2");
        return;
    }

    if (find_fake(arg3) == NULL) {
        NoticeToUser(nptr, "This fakeuser does not exist.");
        return;
    }

    fakejoin(arg3,arg4);
}

void oper_jupe (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3 = all;
    all = SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0' || !all || *all == '\0') {
        NoticeToUser(nptr,"Syntax: \2jupe \037servername\037 \037reason\037\2");
        return;
    }   
     
    SendRaw("SQUIT %s :Server juped by %s (%s)",arg3,nptr->nick,all);
    SendRaw("SERVER %s 1 :Server juped (%s)",arg3,all);
    NoticeToUser(nptr,"Done.");
    globops("Server %s \2JUPED\2 by %s (%s)",arg3,nptr->nick,all);
}

void oper_fakekill (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3 = all;
    all = SeperateWord(arg3);

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2fakekill \037nick\037 [\037reason\037]\2");
        return;
    }   
        
    if (!Strcmp(arg3,me.nick)) {
        NoticeToUser(nptr,"That's my nick you sucker");
        return;
    }

    if (find_fake(arg3) == NULL) {
        NoticeToUser(nptr, "This fakeuser does not exist.");
        return;
    }

    fakekill(arg3,all);
}

void oper_fakesay (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3,*arg4;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    all = SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0' || !all || *all == '\0') {
        NoticeToUser(nptr,"Syntax: \2fakesay \037nick\037 \037#channel\037 \037message\037\2");
        return;
    }

    if (find_fake(arg3) == NULL) {
        NoticeToUser(nptr, "This fakeuser does not exist.");
        return;
    }

    fakesay(arg3,arg4,all);
}

void oper_operlist (Nick *nptr)
{
    int count=0;
    Nick *nptr2;
    struct hashmap_entry *entry;

    NoticeToUser(nptr,"Online operators :");
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_nicks(), entry, nptr2) {
        if (IsOper(nptr2)) {
            NoticeToUser(nptr,"\2%s\2     %s@%s",nptr2->nick,nptr2->ident,nptr2->host);
            count++;
        }
    }
        
    NoticeToUser(nptr,"End of list (%d entries).",count);
}

void oper_stats (Nick *nptr)
{
    Nick *nptr2;
    struct hashmap_entry *entry;

    int uptime = time(NULL) - get_core()->startuptime;
    int days = uptime / 86400, hours = (uptime / 3600) % 24, mins = (uptime / 60) % 60, secs = uptime % 60;

    NoticeToUser(nptr,"There are %d registered users and %d registered channels",
                 HASHMAP_SIZE(core_get_users()), HASHMAP_SIZE(core_get_chans()));

    int j=0,k=0,l=0,m=0,n=0,o=0;
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_nicks(), entry, nptr2) {
        if (IsOper(nptr2)) j++;
        if (IsOper(nptr2) && !IsBot(nptr2) && !IsService(nptr2)) k++;
        if ((IsOper(nptr2) && IsBot(nptr2)) || IsService(nptr2)) l++;
        if (!IsOper(nptr2) && IsBot(nptr2)) m++;
        if (IsBot(nptr2) || IsService(nptr2)) n++;
    }

    o = HASHMAP_SIZE(core_get_nicks()) - l - m - k;

    NoticeToUser(nptr,"There are \2%d\2 online users (\2%d\2 opers)", HASHMAP_SIZE(core_get_nicks()) + HASHMAP_SIZE(core_get_fakeusers()), j);
    NoticeToUser(nptr,"   %d fakeusers", HASHMAP_SIZE(core_get_fakeusers()));
    NoticeToUser(nptr,"   %d ircops",k);
    NoticeToUser(nptr,"   %d bots",n);
    NoticeToUser(nptr,"       %d opers bots",l);
    NoticeToUser(nptr,"       %d normals bots",m);
    NoticeToUser(nptr,"   \2%d\2 real users",o);

    char hr[10];
    char mn[10];
    char sc[10];

    if (hours < 10) sprintf(hr,"0%d",hours); else sprintf(hr,"%d",hours);
    if (mins < 10) sprintf(mn,"0%d",mins); else sprintf(mn,"%d",mins);
    if (secs < 10) sprintf(sc,"0%d",secs); else sprintf(sc,"%d",secs);

    if (days > 1) {
        NoticeToUser(nptr,"Uptime: \2%d\2 days, \2%s:%s\2",days,hr,mn);
    } else if (days == 1) {
        NoticeToUser(nptr,"Uptime: \2%d\2 day, \2%s:%s\2",days,hr,mn);
    } else {
        if (hours > 1) {
            NoticeToUser(nptr,"Uptime: \2%d\2 hours, \2%d\2 minutes",hours,mins);
        } else if (hours == 1) {
            NoticeToUser(nptr,"Uptime: \2%d\2 hour, \2%d\2 minutes",hours,mins);
        } else {
            if (mins > 1) {
                NoticeToUser(nptr,"Uptime: \2%d\2 minutes, \2%d\2 seconds",mins,secs);
            } else if (mins == 1) {
                NoticeToUser(nptr,"Uptime: \2%d\2 minute, \2%d\2 seconds",mins,secs);
            } else {
                if (secs > 1)
                    NoticeToUser(nptr,"Uptime: \2%d\2 seconds",secs);
                else if (secs == 1)
                    NoticeToUser(nptr,"Uptime: \2%d\2 second",secs);
                else
                    NoticeToUser(nptr,"You discovered a hidden message, congratulations :)");
            }
        }
    }

    NoticeToUser(nptr,"Memory : vsize %lu ko , rss %ld ko",get_mem(MEM_VSIZE),get_mem(MEM_RSS));
}

void oper_changelev (Nick *nptr, User *uptr, char *all)
{
    char *arg3,*arg4;
    User *uptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2changelev \037nick\037 \037level\037\2");
        return;
    }   
        
    uptr2 = find_user(arg3);
    if (!uptr2) {
        NoticeToUser(nptr,"This user does not exist");
        return;
    }   
        
    if (uptr2->level >= uptr->level || strtol(arg4,NULL,10) >= uptr->level) {
        NoticeToUser(nptr,"You cannot change the level of a user outranking you");
        return;
    }   
        
    uptr2->level = strtol(arg4,NULL,10);
    NoticeToUser(nptr,"The level of \2%s\2 is now \2%s\2",arg3,arg4);
        
    operlog("%s changed the level of %s to %s",nptr->nick,arg3,arg4);
}

void oper_global (Nick *nptr, User *uptr __unused, char *all)
{
    if (!all || *all == '\0') {
        NoticeToUser(nptr,"Syntax: \2global \037message\037\2");
        return;
    }

    if (me.anonymous_global)
        Global("(Global) %s", all);
    else
        Global("(Global) [%s] %s", nptr->nick, all);
}

void oper_sglobal (Nick *nptr, User *uptr __unused, char *all)
{
    char *serv, *message;
    serv = all;
    message = SeperateWord(all);

    if (!serv || *serv == '\0' || !message || *message == '\0') {
        NoticeToUser(nptr, "Syntax: \2sglobal \037server\037 \037message\037\2");
        return;
    }

    if (me.anonymous_global)
        send_global(serv, "(sGlobal) %s", message);
    else
        send_global(serv, "(sGlobal) [%s] %s", nptr->nick, message);
}

void oper_raw (Nick *nptr, User *uptr __unused, char *all)
{
    if (!get_core()->raws) {
        NoticeToUser(nptr,"The raws are disabled");
        return;
    }

    if (!all || *all == '\0') {
        NoticeToUser(nptr,"Syntax: \2raw \037message\037\2");
        return;
    }   
        
    SendRaw("%s",all);
    operlog("%s executed RAW: %s",nptr->nick,all);
}

void oper_savedb (Nick *nptr)
{
    savealldb();
    NoticeToUser(nptr,"Databases saved");
}

void oper_setraws (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

    if (!Strcmp(arg3,"1")) {
        get_core()->raws = true;
        NoticeToUser(nptr,"The raws are now enabled");
        operlog("%s enabled RAWS",nptr->nick);
    } else if (!Strcmp(arg3,"0")) {
        get_core()->raws = false;
        NoticeToUser(nptr,"The raws are now disabled");
        operlog("%s disabled RAWS",nptr->nick);
    } else
        NoticeToUser(nptr,"Syntax: \2setraws [1|0]\2");
}

void oper_rehash (Nick *nptr)
{
    loadconf(1);
    NoticeToUser(nptr,"Configuration rehashed");
    operlog("%s rehashed configuration",nptr->nick);
}

void oper_restart (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

    SendRaw(":%s QUIT :Restarting",me.nick);
    operlog("%s executed RESTART",nptr->nick);
    if (!arg3 || *arg3 == '\0')
        child_restart(1); 
    if (!Strcmp(arg3,"0")) child_restart(0);
}

void oper_die (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3 = all;
    SeperateWord(arg3);

    SendRaw(":%s QUIT :I'll be back soon !",me.nick);
    operlog("%s executed DIE",nptr->nick);
    if (!arg3 || *arg3 == '\0')
        child_die(1);
    if (!Strcmp(arg3,"0")) child_die(0);
}

void oper_noexpire (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg3,*arg4;
    Chan *chptr;
    User *uptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2noexpire [\037username\037|\037#channel\037] [on|off]\2");
        return;
    }   
        
    if (*arg3 == '#') {
        chptr = find_channel(arg3);
        if (!chptr) {
            NoticeToUser(nptr,"This channel does not exist");
            return;
        }   
            
        if (!Strcmp(arg4,"on")) {
            SetOption(chptr, COPT_NOEXPIRE);
            NoticeToUser(nptr,"Option \2NOEXPIRE\2 set to \2on\2 for channel \2%s\2",chptr->channelname);
            return;
       } else if (!Strcmp(arg4,"off")) {
            ClearOption(chptr, COPT_NOEXPIRE);
            NoticeToUser(nptr,"Option \2NOEXPIRE\2 set to \2off\2 for channel \2%s\2",chptr->channelname);
            return;
        } else {
            NoticeToUser(nptr,"Syntax: \2noexpire [\037username\037|\037#channel\037] [on|off]\2");
            return;
        }   
    } else {
        uptr2 = find_user(arg3);
        if (!uptr2) {
            NoticeToUser(nptr,"This user does not exist");
            return;
        }   
            
        if (!Strcmp(arg4,"on")) {
            SetOption(uptr2, UOPT_NOEXPIRE);
            NoticeToUser(nptr,"Option \2NOEXPIRE\2 set to \2on\2 for user \2%s\2",uptr2->nick);
            return;
        } else if (!Strcmp(arg4,"off")) {
            ClearOption(uptr2, UOPT_NOEXPIRE);
            NoticeToUser(nptr,"Option \2NOEXPIRE\2 set to \2off\2 for user \2%s\2",uptr2->nick);
            return;
        } else {
            NoticeToUser(nptr,"Syntax: \2noexpire [\037username\037|\037#channel\037] [on|off]\2");
            return;
        }
    }
}

void oper_suspend (Nick *nptr, User *uptr, char *all)
{
    char *arg3,*arg4;
    Chan *chptr;
    User *uptr2;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0' || !arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2suspend [\037username\037|\037#channel\037] [on|off]\2");
        return;
    };  
        
    if (*arg3 == '#') {
        chptr = find_channel(arg3);
        if (!chptr) {
            NoticeToUser(nptr,"This channel does not exist");
            return;
        }   
            
        if (!Strcmp(arg4,"on")) {
            SetOption(chptr, COPT_SUSPENDED);
            SendRaw(":%s PART %s :Channel suspended",channel_botname(chptr),chptr->channelname);
            NoticeToUser(nptr,"Channel %s suspended.",chptr->channelname);
            return;
        } else if (!Strcmp(arg4,"off")) {
            ClearOption(chptr, COPT_SUSPENDED);
            JoinChannel(channel_botname(chptr),chptr->channelname);
            NoticeToUser(nptr,"Channel %s unsuspended.",chptr->channelname);
            return;
        } else {
            NoticeToUser(nptr,"Syntax: \2suspend [\037username\037|\037#channel\037] [on|off]\2");
            return;
        }   
    } else {
        uptr2 = find_user(arg3);
        if (!uptr2) {
            NoticeToUser(nptr,"This user does not exist");
            return;
        }   

        if (uptr2->level >= uptr->level) {
            NoticeToUser(nptr,"Cannot change suspend status of a user outranking you.");
            return;
        }

        if (!Strcmp(arg4,"on")) {
            SetOption(uptr2, UOPT_SUSPENDED);
            NoticeToUser(nptr,"User %s suspended.",uptr2->nick);
            uptr2->authed = 0;
            return;
        } else if (!Strcmp(arg4,"off")) {
            ClearOption(uptr2, UOPT_SUSPENDED);
            NoticeToUser(nptr,"User %s unsuspended.",uptr2->nick);
            return;
        } else {
             NoticeToUser(nptr,"Syntax: \2suspend [\037username\037|\037#channel\037] [on|off]\2");
            return;
        }   
    }   
}

void oper_cmdlev (Nick *nptr, User *uptr __unused, char *all)
{
    int ret;
    char *arg1,*arg2,*arg3,*arg4,*arg5;
    arg1 = all;
    arg2 = SeperateWord(arg1);
    arg3 = SeperateWord(arg2);
    arg4 = SeperateWord(arg3);
    arg5 = SeperateWord(arg4);
    SeperateWord(arg5);

    if (!arg1 || *arg1 == '\0' || !arg2 || *arg2 == '\0') {
        NoticeToUser(nptr,"Syntax: \2cmdlev \037command\037 \037new level\037\2");
        NoticeToUser(nptr,"Example: cmdlev chan register 100");
        return;
    }

    if (!arg3 || *arg3 == '\0')
        ret = setcmdlev(arg1,NULL,NULL,NULL,atoi(arg2));
    else if (!arg4 || *arg4 == '\0')
        ret = setcmdlev(arg1,arg2,NULL,NULL,atoi(arg3));
    else if (!arg5 || *arg5 == '\0')
        ret = setcmdlev(arg1,arg2,arg3,NULL,atoi(arg4));
    else
        ret = setcmdlev(arg1,arg2,arg3,arg4,atoi(arg5));

    if (ret)
        NoticeToUser(nptr,"Done.");
    else
        NoticeToUser(nptr,"Failed.");
}

void oper_superadmin (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg1;
    arg1 = all;
    SeperateWord(arg1);

    if (!arg1 || *arg1 == '\0') {
        NoticeToUser(nptr, "Syntax: \2superadmin [\037on\037|\037off\037]\2");
        return;
    }

    if (!Strcmp(arg1, "on")) {
        SetUmode(nptr, UMODE_SUPERADMIN);
        NoticeToUser(nptr, "SuperAdmin status enabled.");
    } else if (!Strcmp(arg1, "off")) {
        ClearUmode(nptr, UMODE_SUPERADMIN);
        NoticeToUser(nptr, "SuperAdmin status disabled.");
    } else
        NoticeToUser(nptr, "Syntax: \2superadmin [\037on\037|\037off\037]\2");
}

#ifdef USE_FILTER
void oper_ruleslist (Nick *nptr)
{
    struct ruleset *rule;
    NoticeToUser(nptr,"Rules list :");
    for (rule = rule_list.ltail; rule; rule = rule->lprev)
        NoticeToUser(nptr,"%s %s%s from %s to %s action %s data \"%s\"", rule->rule == RULE_DROP ? "drop" : "pass",
                    rule->direction == DIRECT_IN ? "in" : "out", rule->quick ? " quick" : "", rule->from[0] ? rule->from : "(null)",
                    rule->to[0] ? rule->to : "(null)", rule->action[0] ? rule->action : "(null)", rule->data[0] ? rule->data : "(null)");
    NoticeToUser(nptr,"End of list.");
}

void oper_reloadrules (Nick *nptr)
{
    if ((loadrulefile()) == 1)
        NoticeToUser(nptr,"Filter rules reloaded");
    else
        NoticeToUser(nptr,"Cannot reload rules");
}

void oper_setfilter (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg1;
    arg1 = all;
    SeperateWord(arg1);

    if (!arg1 || *arg1 == '\0')
        NoticeToUser(nptr,"Filter status: %s", rule_list.enabled ? "enabled" : "disabled");
    else {
        if (!Strcmp(arg1,"0") || !Strcmp(arg1,"1")) {
            rule_list.enabled = atoi(arg1);
            NoticeToUser(nptr,"Done.");
        } else
            NoticeToUser(nptr,"Unknown value %s",arg1);
    }
}
#endif
