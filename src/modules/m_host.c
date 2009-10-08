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


#include <child.h>
#include <globals.h>

void do_host (Nick *, User *, char *);
void do_help (Nick *, User *, char *);
void host_set (Nick *, User *, char *);
void host_list (Nick *);

void child_init()
{
    addBaseCommand("host",do_host,me.level_oper);

    addHostCommand("set",host_set,me.level_oper);
    addHostCommand("list",host_list,me.level_oper);
}

void child_cleanup()
{
    deleteCommand("host",CMD_BASE,0);

    delHostCommand("set");
    delHostCommand("list");
}

void do_host(Nick *nptr, User *uptr, char *all)
{
    char *arg1,*arg2;
    char blah[1024];
    strncpy(blah,all,1024);

    arg1 = blah;
    arg2 = SeperateWord(arg1);
    SeperateWord(arg2);
    
    if (!arg2 || *arg2 == '\0') {
        NoticeToUser(nptr,"Type \2/msg %s help host\2 for more informations", me.nick);
        return;
    }
    
    if (!Strcmp(arg2,"help")) {
        do_help(nptr,uptr,all);
        return;
    }
    
    all = SeperateWord(all);
    all = SeperateWord(all);
    
    Command *cmd;
    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HOST)) {
        if (!Strcmp(cmd->name,arg2) && cmd->type == CMD_HOST) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }
    
    NoticeToUser(nptr,"Unknown command");
}

void host_set (Nick *nptr, User *uptr __unused, char *all)
{
    User *uptr2;
    char *arg3,*arg4;
    arg3 = all;
    arg4 = SeperateWord(arg3);
    SeperateWord(arg4);

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Syntax: \2set \037nick\037 [\037vhost\037]\2");
        return;
    }

    uptr2 = find_user(arg3);
    if (!uptr2) {
        NoticeToUser(nptr,"This user does not exist");
        return;
    }
        
    if (!arg4 || *arg4 == '\0') {
        memset(uptr2->vhost,'\0',HOSTLEN);
        NoticeToUser(nptr,"Vhost for \2%s\2 deleted",arg3);
        operlog("%s removed vhost from %s",nptr->nick,arg3);
        if (find_nick(arg3))
            SendRaw("SVSMODE %s -x+x",arg3);
    } else {
        strncpy(uptr2->vhost,arg4,HOSTLEN);
        NoticeToUser(nptr,"Vhost for \2%s\2 set to \2%s\2",arg3,arg4);
        operlog("%s set vhost for %s to %s",nptr->nick,arg3,arg4);
        if (find_nick(arg3))
            SendRaw("CHGHOST %s %s",arg3,arg4);
    }
}

void host_list (Nick *nptr)
{
    User *uptr2;

    LIST_FOREACH_ALL(user_list, uptr2) {
        if (uptr2->vhost && uptr2->vhost[0] != '\0')
            NoticeToUser(nptr,"%s             %s",uptr2->nick,uptr2->vhost);
    }
    NoticeToUser(nptr,"End of list.");
}
