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


#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <mem.h>

#define CMDLEN 200

#define CMD_BASE    0
/*
void func (Nick *nptr, User *uptr, char *all);
    nptr : sender's nick structure (should never be NULL)
    uptr : sender's user account structure (can be NULL)
    all : sender's message
*/

#define CMD_NICK    1
/*
void func (Nick *nptr, User *uptr, char *all);
    nptr : sender's nick structure (should never be NULL)
    uptr : sender's user account structure (can be NULL)
    all : sender's message, without the base command. E.g. if someone sent /msg C nick register blah, then "all" will contain "register blah"
*/

#define CMD_CHAN    2
/*
void func (Nick *nptr, User *uptr, Chan *chptr, char *all);
    nptr : sender's nick structure (should never be NULL)
    uptr : sender's user account structure (can be NULL)
    chptr : if the sender provided a registered channel as parameter (e.g. /msg C chan info #opers), then chptr will point to its structure. Otherwise, it's a NULL-pointer
    all : same as CMD_NICK
*/

#define CMD_OPER    3
/*
void func (Nick *nptr, User *uptr, char *all);
    nptr : sender's nick structure (should never be NULL)
    uptr : sender's user account structure (can be NULL)
    all : same as CMD_NICK
*/

#define CMD_HOST    4
/*
void func (Nick *nptr, User *uotr, char *all);
    nptr : sender's nick structure (should never be NULL)
    uptr : sender's user account structure (can be NULL)
    all : same as CMD_NICK
*/

#define CMD_BOT     5
/*
void func (Nick *nptr, User *uptr, Chan *chptr, Wchan *wchan, char *all);
    nptr : sender's nick structure (should never be NULL)
    uptr : sender's user account structure (can be NULL)
    chptr : registered channel structure (should never be NULL)
    wchan : channel structre (should never be NULL)
    all : sender's parameters (e.g. if someone typed !kick foo no reason, then all will contain "foo no reason"
*/

#define CMD_HELP    6
/*
void func (Nick *nptr, User *uptr, char *all);
    nptr : sender's nick structure (should never be NULL)
    uptr : sender's user account structure (can be NULL)
    all : same as CMD_NICK
*/

/* subtypes */

#define CMD_HELP_NICK   1
#define CMD_HELP_CHAN   2
#define CMD_HELP_OPER   3
#define CMD_HELP_HOST   4
#define CMD_HELP_BOT    5
#define CMD_HELP_NICK_SET 6
#define CMD_HELP_CHAN_SET 7

#define CMD_CHAN_SET 1
#define CMD_NICK_SET 1

/* end of subtypes */

#define addCommand(a,b,c,d) __addCommand(a,b,c,NULL,0,d)

#define addBaseCommand(a,b,c) addCommand(a,CMD_BASE,b,c)
#define addBotCommand(a,b,c) addCommand(a,CMD_BOT,b,c)
#define addChanCommand(a,b,c) addCommand(a,CMD_CHAN,b,c)
#define addChanSetCommand(a,b,c) __addCommand(a,CMD_CHAN,b,NULL,CMD_CHAN_SET,c)
#define addHostCommand(a,b,c) addCommand(a,CMD_HOST,b,c)
#define addNickCommand(a,b,c) addCommand(a,CMD_NICK,b,c)
#define addNickSetCommand(a,b,c) __addCommand(a,CMD_NICK,b,NULL,CMD_NICK_SET,c)
#define addOperCommand(a,b,c) addCommand(a,CMD_OPER,b,c)
#define addHelpCommand(a,b,c,d,e) __addCommand(a,CMD_HELP,b,c,d,e)
#define addHelpHostCommand(a,b,c,d) addHelpCommand(a,b,c,CMD_HELP_HOST,d)
#define addHelpNickCommand(a,b,c,d) addHelpCommand(a,b,c,CMD_HELP_NICK,d)
#define addHelpNickSetCommand(a,b,c,d) addHelpCommand(a,b,c,CMD_HELP_NICK_SET,d)
#define addHelpChanCommand(a,b,c,d) addHelpCommand(a,b,c,CMD_HELP_CHAN,d)
#define addHelpChanSetCommand(a,b,c,d) addHelpCommand(a,b,c,CMD_HELP_CHAN_SET,d)
#define addHelpBotCommand(a,b) addHelpCommand(a,NULL,b,CMD_HELP_BOT,0)
#define addHelpOperCommand(a,b,c,d) addHelpCommand(a,b,c,CMD_HELP_OPER,d)

#define delBaseCommand(a) deleteCommand(a,CMD_BASE,0)
#define delBotCommand(a) deleteCommand(a,CMD_BOT,0)
#define delChanCommand(a) deleteCommand(a,CMD_CHAN,0)
#define delChanSetCommand(a) deleteCommand(a,CMD_CHAN,CMD_CHAN_SET)
#define delHostCommand(a) deleteCommand(a,CMD_HOST,0)
#define delNickCommand(a) deleteCommand(a,CMD_NICK,0)
#define delNickSetCommand(a) deleteCommand(a,CMD_NICK,CMD_NICK_SET)
#define delOperCommand(a) deleteCommand(a,CMD_OPER,0)
#define delHelpHostCommand(a) deleteCommand(a,CMD_HELP,CMD_HELP_HOST)
#define delHelpNickCommand(a) deleteCommand(a,CMD_HELP,CMD_HELP_NICK)
#define delHelpNickSetCommand(a) deleteCommand(a,CMD_HELP,CMD_HELP_NICK_SET)
#define delHelpChanCommand(a) deleteCommand(a,CMD_HELP,CMD_HELP_CHAN)
#define delHelpChanSetCommand(a) deleteCommand(a,CMD_HELP,CMD_HELP_CHAN_SET)
#define delHelpBotCommand(a) deleteCommand(a,CMD_HELP,CMD_HELP_BOT)
#define delHelpOperCommand(a) deleteCommand(a,CMD_HELP,CMD_HELP_OPER)

/* hash key : type + subtype (HASH_INT) */

typedef struct command {
    char name[CMDLEN + 1];
    char desc[CMDLEN + 1];
    int type;
    int subtype;
    int level;
    void (*func)();
    struct command *next,*prev;
    struct command *lnext,*lprev;
} Command;

typedef struct {
    int size;
    TABLE(Command);
    Command *lhead;
} commandlist;

Command *__addCommand (char *, int, void(*)(), char *, int, int);
void delCommand (Command *);
Command *find_command (char *, int, int);
int deleteCommand (char *, int, int);
int setcmdlev (char *, char *, char *, char *, int);
int setcmdlev2 (char *);

void m_chghost (char *, char *);
void m_chgident (char *, char *);
void m_eos ();
void m_join (char *, char *);
void m_kick (char *, char *);
void m_kill (char *, char *);
void m_umode (char *, char *);
void m_mode (char *, char *);
void m_nick (char *, char *);
void m_part (char *, char *);
void m_ping (char *);
void m_privmsg (char *, char *);
void m_quit (char *);
void m_register_user_v3 (char *, char *);
void m_register_user_v4 (char *, char *);
void m_sethost (char *, char *);
void m_setident (char *, char *);
void m_protoctl ();
void m_topic (char *, char *);
void m_stopic (char *, char *);

#endif
