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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
USA.
*/


#include <child.h>
#include <globals.h>

Command *__addCommand (char *name, int type, void (*func)(), char *desc, int subtype, int level)
{
    Command *newcmd;

    newcmd = (Command *)malloc(sizeof(Command));
    strncpy(newcmd->name,name,CMDLEN);
    newcmd->type = type;
    newcmd->subtype = subtype;
    newcmd->level = level;
    newcmd->func = func;

    if (!desc)
        bzero(newcmd->desc,CMDLEN);
    else
        strncpy(newcmd->desc,desc,CMDLEN);

    LIST_INSERT_HEAD(command_list, newcmd, HASH_INT(type+subtype));

    return newcmd;
}

inline void delCommand (Command *cmd)
{
    LIST_REMOVE(command_list, cmd, HASH_INT(cmd->type+cmd->subtype));
    free(cmd);
}

int deleteCommand (char *name, int type, int subtype)
{
    Command *cmd;
    cmd = find_command(name,type,subtype);
    if (!cmd) return 0;
    delCommand(cmd);
    return 1;
}

Command *find_command (char *name, int type, int subtype)
{
    Command *tmp;
    LIST_FOREACH(command_list, tmp, HASH_INT(type+subtype)) {
        if (!Strcmp(tmp->name,name) && tmp->type == type && tmp->subtype == subtype)
            return tmp;
    }

    return NULL;
}

int setcmdlev (char *base, char *type, char *subtype, char *end, int level)
{
    Command *cmd;
    int ibase,itype=0,isubtype=0;

    if (level > me.level_owner) return 0;

    if (!Strcmp(base,"nick"))
        ibase = CMD_NICK;
    else if (!Strcmp(base,"chan"))
        ibase = CMD_CHAN;
    else if (!Strcmp(base,"oper"))
        ibase = CMD_OPER;
    else if (!Strcmp(base,"host"))
        ibase = CMD_HOST;
    else if (!Strcmp(base,"bot"))
        ibase = CMD_BOT;
    else if (!Strcmp(base,"help"))
        ibase = CMD_HELP;
    else
        return 0;

    switch(ibase) {
        case CMD_NICK:
            if (!Strcmp(type,"set"))
                itype = CMD_NICK_SET;
            break;
        case CMD_CHAN:
            if (!Strcmp(type,"set"))
                itype = CMD_CHAN_SET;
            break;
        case CMD_HELP:
            if (!Strcmp(type,"nick"))
                itype = CMD_HELP_NICK;
            else if (!Strcmp(type,"chan"))
                itype = CMD_HELP_CHAN;
            else if (!Strcmp(type,"oper"))
                itype = CMD_HELP_OPER;
            else if (!Strcmp(type,"host"))
                itype = CMD_HELP_HOST;
            else if (!Strcmp(type,"bot"))
                itype = CMD_HELP_BOT;
            else
                return 0;
            break;
        default:
            itype = 0;
    }

    switch(itype) {
        case CMD_HELP_NICK:
            if (!Strcmp(subtype,"set"))
                isubtype = itype = CMD_HELP_NICK_SET;
            break;
        case CMD_HELP_CHAN:
            if (!Strcmp(subtype,"set"))
                isubtype = itype = CMD_HELP_CHAN_SET;
            break;
        default:
            isubtype = 0;
    }

    if (isubtype) {
        if (end && *end != '\0')
            cmd = find_command(end,ibase,itype);
        else
            cmd = find_command(subtype,ibase,itype == CMD_HELP_NICK_SET ? CMD_HELP_NICK : CMD_HELP_CHAN);

        if (!cmd) return 0;
        cmd->level = level;
        return 1;
    }

    if (itype) {
        if (subtype && *subtype != '\0')
            cmd = find_command(subtype,ibase,itype);
        else
            cmd = find_command(type,ibase,0);

        if (!cmd) return 0;
        cmd->level = level;
        return 1;
    }

    if (type && *type != '\0')
        cmd = find_command(type,ibase,0);
    else
        cmd = find_command(base,CMD_BASE,0);

    if (!cmd) return 0;
    cmd->level = level;
    return 1;
}

int setcmdlev2 (char *data)
{
    int ret;
    char *arg1,*arg2,*arg3,*arg4,*arg5;

    arg1 = data;
    arg2 = SeperateWord(arg1);
    arg3 = SeperateWord(arg2);
    arg4 = SeperateWord(arg3);
    arg5 = SeperateWord(arg4);
    SeperateWord(arg5);

    if (!arg1 || *arg1 == '\0' || !arg2 || *arg2 == '\0')
        return 0;

    if (!arg3 || *arg3 == '\0')
        ret = setcmdlev(arg1,NULL,NULL,NULL,atoi(arg2));
    else if (!arg4 || *arg4 == '\0')
        ret = setcmdlev(arg1,arg2,NULL,NULL,atoi(arg3));
    else if (!arg5 || *arg5 == '\0')
        ret = setcmdlev(arg1,arg2,arg3,NULL,atoi(arg4));
    else
        ret = setcmdlev(arg1,arg2,arg3,arg4,atoi(arg5));

    return ret;
}
