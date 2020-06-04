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
#include "commands.h"
#include "modules.h"
#include "string_utils.h"

#include <stdlib.h>
#include <string.h>

extern commandlist command_list;

extern int raws;

#define ACTUAL_CMDLEN 16


void do_help (Nick *, User *, char *);
void help_host (Nick *, User *, char *);
void help_host_set(Nick *);
void help_host_list(Nick *);
void help_nick (Nick *, User *, char *);
void help_nick_link (Nick *);
void help_nick_unlink (Nick *);
void help_nick_identify (Nick *);
void help_nick_register (Nick *);
void help_nick_drop (Nick *);
void help_nick_info (Nick *);
void help_nick_ghost (Nick *);
void help_nick_set (Nick *, User *, char *);
void help_nick_requestpassword (Nick *);
void help_nick_set_password (Nick *);
void help_nick_set_email (Nick *);
void help_nick_set_private (Nick *);
void help_nick_set_protect (Nick *);
void help_nick_set_timeout (Nick *);
void help_nick_set_master (Nick *);
void help_nick_set_noauto (Nick *);
void help_nick_set_hidemail (Nick *);
void help_nick_set_cloak (Nick *);
void help_chan (Nick *, User *, char *);
void help_chan_register (Nick *);
void help_chan_drop (Nick *);
void help_chan_access (Nick *, User *, char *);
void help_chan_op (Nick *);
void help_chan_deop (Nick *);
void help_chan_halfop (Nick *);
void help_chan_dehalfop (Nick *);
void help_chan_voice (Nick *);
void help_chan_devoice (Nick *);
void help_chan_invite (Nick *);
void help_chan_kick (Nick *);
void help_chan_info (Nick *);
void help_chan_unbanall (Nick *);
void help_chan_clearmodes (Nick *);
void help_chan_clearchan (Nick *);
void help_chan_set (Nick *, User *, char *);
void help_chan_entrymsg (Nick *);
void help_chan_assign (Nick *);
void help_chan_unassign (Nick *);
void help_chan_botlist (Nick *);
void help_chan_addbot (Nick *);
void help_chan_delbot (Nick *);
void help_chan_resync (Nick *);
void help_chan_suspend (Nick *);
void help_chan_unsuspend (Nick *);
void help_chan_banlist (Nick *);
void help_chan_topic (Nick *);
void help_chan_clearbans (Nick *);
void help_chan_set_founder (Nick *);
void help_chan_set_avoice (Nick *);
void help_chan_set_aop (Nick *);
void help_chan_set_nojoin (Nick *);
void help_chan_set_noauto (Nick *);
void help_chan_set_private (Nick *);
void help_chan_set_strictop (Nick *);
void help_chan_set_protectops (Nick *);
void help_chan_set_mlock (Nick *);
void help_chan_set_autolimit (Nick *);
void help_chan_set_secure (Nick *);
void help_chan_set_enablemask (Nick *);
void help_chan_set_mass (Nick *);
void help_chan_set_enftopic (Nick *);
void help_chan_set_axxflags (Nick *);
void help_bot (Nick *, User *, char *);
void help_oper (Nick *, User *, char *);
void help_oper_stats (Nick *);
void help_oper_global (Nick *);
void help_oper_fakeuser (Nick *);
void help_oper_fakejoin (Nick *);
void help_oper_fakesay (Nick *);
void help_oper_fakenick (Nick *);
void help_oper_fakekill (Nick *);
void help_oper_userlist (Nick *);
void help_oper_nicklist (Nick *);
void help_oper_chanlist (Nick *);
void help_oper_jupe (Nick *);
void help_oper_killall (Nick *);
void help_oper_glineall (Nick *);
void help_oper_changelev (Nick *);
void help_oper_noexpire (Nick *);
void help_oper_suspend (Nick *);
void help_oper_forceauth (Nick *);
void help_oper_raw (Nick *);
void help_oper_savedb (Nick *);
void help_oper_restart (Nick *);
void help_oper_die (Nick *);
void help_oper_operlist (Nick *);
void help_oper_trustadd (Nick *);
void help_oper_trustdel (Nick *);
void help_oper_trustlist (Nick *);
void help_oper_modload (Nick *);
void help_oper_modunload (Nick *);
void help_oper_modlist (Nick *);
void help_oper_rehash (Nick *);
void help_oper_setraws (Nick *);
void help_oper_cmdlev (Nick *);
#ifdef USE_FILTER
void help_oper_ruleslist (Nick *);
void help_oper_reloadrules (Nick *);
void help_oper_setfilter (Nick *);
#endif
void help_oper_superadmin (Nick *);
void help_oper_sglobal (Nick *);
void help_oper_fakelist (Nick *);
void help_oper_glinechan (Nick *);
void help_oper_regexpcheck (Nick *);

void child_init(Module *module)
{
    module->nodreload = 1;

    addBaseCommand("help",do_help,0);

    addCommand("host",CMD_HELP,help_host,me.level_oper);
    addCommand("oper",CMD_HELP,help_oper,me.level_oper);
    addCommand("nick",CMD_HELP,help_nick,0);
    addCommand("chan",CMD_HELP,help_chan,0);
    addCommand("bot",CMD_HELP,help_bot,0);

    addHelpHostCommand("list",help_host_list,"List all users having a vhost",me.level_oper);
    addHelpHostCommand("set",help_host_set,"Set or remove a vhost",me.level_oper);

    addHelpNickCommand("unlink",help_nick_unlink,"Unlink your nick",0);
    addHelpNickCommand("set",help_nick_set,"Set some options",0);
    addHelpNickCommand("requestpassword", help_nick_requestpassword, "Request a new password",0);
    addHelpNickCommand("register",help_nick_register,"Register your nick",0);
    addHelpNickCommand("link",help_nick_link,"Link to another nick",0);
    addHelpNickCommand("info",help_nick_info,"Get informations about someone",0);
    addHelpNickCommand("identify",help_nick_identify,"Identify yourself to services",0);
    addHelpNickCommand("ghost",help_nick_ghost,"Kill a ghosted nick",0);
    addHelpNickCommand("drop",help_nick_drop,"Drop your account",0);
    addHelpNickSetCommand("timeout",help_nick_set_timeout,"Set the maximum time to identify to services",0);
    addHelpNickSetCommand("protect",help_nick_set_protect,"Protect your nick from impersonation",0);
    addHelpNickSetCommand("private",help_nick_set_private,"Hide your informations",0);
    addHelpNickSetCommand("password",help_nick_set_password,"Change your password",0);
    addHelpNickSetCommand("noauto",help_nick_set_noauto,"Do not set channel access on identify",0);
    addHelpNickSetCommand("master",help_nick_set_master,"Set the master for linked nicks",0);
    addHelpNickSetCommand("hidemail",help_nick_set_hidemail,"Hide your email from other users",0);
    addHelpNickSetCommand("email",help_nick_set_email,"Set your email",0);
    addHelpNickSetCommand("cloak",help_nick_set_cloak,"Enable or disable host cloaking",0);

    addHelpChanCommand("voice",help_chan_voice,"Give voice",0);
    addHelpChanCommand("unsuspend",help_chan_unsuspend,"Unsuspend access",0);
    addHelpChanCommand("unbanall",help_chan_unbanall,"Remove all bans",0);
    addHelpChanCommand("unassign",help_chan_unassign,"Unassign a bot from a channel",0);
    addHelpChanCommand("topic",help_chan_topic,"Set the topic",0);
    addHelpChanCommand("suspend",help_chan_suspend,"Suspend access",0);
    addHelpChanCommand("set",help_chan_set,"Set some options",0);
    addHelpChanCommand("resync",help_chan_resync,"Resync users with access list",0);
    addHelpChanCommand("register",help_chan_register,"Register a channel",0);
    addHelpChanCommand("op",help_chan_op,"Give op",0);
    addHelpChanCommand("kick",help_chan_kick,"Kick someone or all users",0);
    addHelpChanCommand("invite",help_chan_invite,"Invite yourself in a channel",0);
    addHelpChanCommand("info",help_chan_info,"Get informations about a channel",0);
    addHelpChanCommand("halfop",help_chan_halfop,"Give halfop",0);
    addHelpChanCommand("entrymsg",help_chan_entrymsg,"Set the entrymsg for a channel",0);
    addHelpChanCommand("drop",help_chan_drop,"Drop a channel",0);
    addHelpChanCommand("devoice",help_chan_devoice,"Take voice",0);
    addHelpChanCommand("deop",help_chan_deop,"Take op",0);
    addHelpChanCommand("delbot",help_chan_delbot,"Remove a bot",me.level_oper);
    addHelpChanCommand("dehalfop",help_chan_dehalfop,"Take halfop",0);
    addHelpChanCommand("clearmodes",help_chan_clearmodes,"Remove all channel modes",0);
    addHelpChanCommand("clearchan",help_chan_clearchan,"Clear channel",0);
    addHelpChanCommand("clearbans",help_chan_clearbans,"Clear services channel bans",0);
    addHelpChanCommand("botlist",help_chan_botlist,"List available bots",0);
    addHelpChanCommand("banlist",help_chan_banlist,"List services channel bans",0);
    addHelpChanCommand("assign",help_chan_assign,"Assign a bot to a channel",0);
    addHelpChanCommand("addbot",help_chan_addbot,"Add a bot",me.level_oper);
    addHelpChanCommand("access",help_chan_access,"Manage access list",0);
    addHelpChanSetCommand("strictop",help_chan_set_strictop,"Enable/disable strict opping",0);
    addHelpChanSetCommand("secure",help_chan_set_secure,"Aop/avoice only identified users",0);
    addHelpChanSetCommand("protectops",help_chan_set_protectops,"Prevent ops from being deopped",0);
    addHelpChanSetCommand("private",help_chan_set_private,"Hide the channel informations",0);
    addHelpChanSetCommand("nojoin",help_chan_set_nojoin,"Set if the bot must join or not the channel",0);
    addHelpChanSetCommand("noauto",help_chan_set_noauto,"Enable or disable auto-modes on join",0);
    addHelpChanSetCommand("mlock",help_chan_set_mlock,"Enforce modes",0);
    addHelpChanSetCommand("mass",help_chan_set_mass,"Enable/disable mass-effect channel commands",0);
    addHelpChanSetCommand("founder",help_chan_set_founder,"Set the founder",0);
    addHelpChanSetCommand("enftopic",help_chan_set_enftopic,"Enable or disable topic enforcing",0);
    addHelpChanSetCommand("enablemask",help_chan_set_enablemask,"Allow masks in access list",0);
    addHelpChanSetCommand("axxflags",help_chan_set_axxflags,"Switch from/to chanflags system",0);
    addHelpChanSetCommand("avoice",help_chan_set_avoice,"Enable/disable autovoice for all users",0);
    addHelpChanSetCommand("autolimit",help_chan_set_autolimit,"Forcefully set a limit",0);
    addHelpChanSetCommand("aop",help_chan_set_aop,"Enable/disable autoop for all users",0);

    addHelpBotCommand("!voiceall","Voice everyone");
    addHelpBotCommand("!voice","Give voice");
    addHelpBotCommand("!unban","Remove a ban");
    addHelpBotCommand("!topic","Change topic");
    addHelpBotCommand("!tblist", "List timebans");
    addHelpBotCommand("!tb", "Set a time ban");
    addHelpBotCommand("!seen","Display how long time a user has not been seen");
    addHelpBotCommand("!protect","Give protect");
    addHelpBotCommand("!owner","Give ownership");
    addHelpBotCommand("!opall","Op everyone");
    addHelpBotCommand("!op","Give op");
    addHelpBotCommand("!kick","Kick someone or every user (*)");
    addHelpBotCommand("!rkick","Kick users matching a C-regexp. WARNING: it is *NOT* mask-style regexp. If you want to kick b*, then type \2!rkick b.*\2. Otherwise it will kick * (b* means \"0 or more b\")");
    addHelpBotCommand("!kb","Kickban someone or every user (*)");
    addHelpBotCommand("!rkb","Kickban users matching a C-regexp. Same warning as !rkick");
    addHelpBotCommand("!halfopall","Halfop everyone");
    addHelpBotCommand("!halfop","Give halfop");
    addHelpBotCommand("!devoiceall","Devoice everyone");
    addHelpBotCommand("!devoice","Take voice");
    addHelpBotCommand("!deprotect","Take protect");
    addHelpBotCommand("!deowner","Take ownership");
    addHelpBotCommand("!deopall","Deop everyone");
    addHelpBotCommand("!deop","Take op");
    addHelpBotCommand("!dehalfopall","Dehalfop everyone");
    addHelpBotCommand("!dehalfop","Take halfop");
    addHelpBotCommand("!ban","Ban someone");
    addHelpBotCommand("!admin","Display online irc operators");

    addHelpOperCommand("userlist",help_oper_userlist,"Search a registered user",me.level_oper);
    addHelpOperCommand("trustlist",help_oper_trustlist,"List trusts",me.level_admin);
    addHelpOperCommand("trustdel",help_oper_trustdel,"Remove a trust",me.level_admin);
    addHelpOperCommand("trustadd",help_oper_trustadd,"Add a trust",me.level_admin);
    addHelpOperCommand("suspend",help_oper_suspend,"Suspend/unsuspend a user/channel",me.level_admin);
    addHelpOperCommand("superadmin",help_oper_superadmin,"Set/unset superadmin status",me.level_root);
    addHelpOperCommand("stats",help_oper_stats,"Display network stats",me.level_oper);
    addHelpOperCommand("sglobal",help_oper_sglobal,"Send a server-wide notice",me.level_oper);
    addHelpOperCommand("setraws",help_oper_setraws,"Enable/disable raws",me.level_owner);
#ifdef USE_FILTER
    addHelpOperCommand("setfilter",help_oper_setfilter,"Enable/disable filter",me.level_root);
#endif
    addHelpOperCommand("savedb",help_oper_savedb,"Save database",me.level_root);
#ifdef USE_FILTER
    addHelpOperCommand("ruleslist",help_oper_ruleslist,"List filter rules",me.level_root);
#endif
    addHelpOperCommand("restart",help_oper_restart,"Restart services",me.level_root);
    addHelpOperCommand("regexpcheck",help_oper_regexpcheck,"Check users affected by a regexp",me.level_oper);
#ifdef USE_FILTER
    addHelpOperCommand("reloadrules",help_oper_reloadrules,"Reload filter rules",me.level_root);
#endif
    addHelpOperCommand("rehash",help_oper_rehash,"Rehash configuration",me.level_root);
    addHelpOperCommand("raw",help_oper_raw,"Send a raw",me.level_root);
    addHelpOperCommand("operlist",help_oper_operlist,"Display online irc operators",me.level_oper);
    addHelpOperCommand("noexpire",help_oper_noexpire,"Set/unset the user/channel noexpire flag",me.level_admin);
    addHelpOperCommand("nicklist",help_oper_nicklist,"Search an online nick",me.level_oper);
    addHelpOperCommand("modunload",help_oper_modunload,"Unload a module",me.level_root);
    addHelpOperCommand("modload",help_oper_modload,"Load a module",me.level_root);
    addHelpOperCommand("modlist",help_oper_modlist,"List loaded modules",me.level_root);
    addHelpOperCommand("killall",help_oper_killall,"Kill all users matching a mask",me.level_admin);
    addHelpOperCommand("jupe",help_oper_jupe,"Jupe a server",me.level_admin);
    addHelpOperCommand("global",help_oper_global,"Send a global notice",me.level_oper);
    addHelpOperCommand("glinechan",help_oper_glinechan,"Gline a whole channel",me.level_admin);
    addHelpOperCommand("glineall",help_oper_glineall,"Gline all users matching a mask",me.level_admin);
    addHelpOperCommand("forceauth",help_oper_forceauth,"Force a nick to auth",me.level_root);
    addHelpOperCommand("fakeuser",help_oper_fakeuser,"Create a fake user",me.level_oper);
    addHelpOperCommand("fakesay",help_oper_fakesay,"Make a fake user say something",me.level_oper);
    addHelpOperCommand("fakenick",help_oper_fakenick,"Change the nick of a fake user",me.level_oper);
    addHelpOperCommand("fakelist",help_oper_fakelist,"List fakeusers",me.level_oper);
    addHelpOperCommand("fakekill",help_oper_fakekill,"Make a fake user quit",me.level_oper);
    addHelpOperCommand("fakejoin",help_oper_fakejoin,"Make a fake user join a channel",me.level_oper);
    addHelpOperCommand("die",help_oper_die,"Die services",me.level_root);
    addHelpOperCommand("cmdlev",help_oper_cmdlev,"Change the level of a command",me.level_owner);
    addHelpOperCommand("chanlist",help_oper_chanlist,"Search a registered channel",me.level_oper);
    addHelpOperCommand("changelev",help_oper_changelev,"Change a user's level",me.level_admin);
}

void child_cleanup()
{
    deleteCommand("help",CMD_BASE,0);

    deleteCommand("host",CMD_HELP,0);
    deleteCommand("oper",CMD_HELP,0);
    deleteCommand("nick",CMD_HELP,0);
    deleteCommand("chan",CMD_HELP,0);
    deleteCommand("bot",CMD_HELP,0);

    delHelpHostCommand("set");
    delHelpHostCommand("list");

    delHelpNickCommand("identify");
    delHelpNickCommand("register");
    delHelpNickCommand("drop");
    delHelpNickCommand("ghost");
    delHelpNickCommand("info");
    delHelpNickCommand("set");
    delHelpNickCommand("link");
    delHelpNickCommand("unlink");
    delHelpNickCommand("requestpassword");
    delHelpNickSetCommand("password");
    delHelpNickSetCommand("email");
    delHelpNickSetCommand("private");
    delHelpNickSetCommand("protect");
    delHelpNickSetCommand("timeout");
    delHelpNickSetCommand("master");
    delHelpNickSetCommand("noauto");
    delHelpNickSetCommand("hidemail");
    delHelpNickSetCommand("cloak");

    delHelpChanCommand("register");
    delHelpChanCommand("drop");
    delHelpChanCommand("access");
    delHelpChanCommand("set");
    delHelpChanCommand("op");
    delHelpChanCommand("deop");
    delHelpChanCommand("halfop");
    delHelpChanCommand("dehalfop");
    delHelpChanCommand("voice");
    delHelpChanCommand("devoice");
    delHelpChanCommand("invite");
    delHelpChanCommand("kick");
    delHelpChanCommand("info");
    delHelpChanCommand("entrymsg");
    delHelpChanCommand("unbanall");
    delHelpChanCommand("clearmodes");
    delHelpChanCommand("clearchan");
    delHelpChanCommand("assign");
    delHelpChanCommand("unassign");
    delHelpChanCommand("botlist");
    delHelpChanCommand("addbot");
    delHelpChanCommand("delbot");
    delHelpChanCommand("resync");
    delHelpChanCommand("suspend");
    delHelpChanCommand("unsuspend");
    delHelpChanCommand("banlist");
    delHelpChanCommand("clearbans");
    delHelpChanCommand("topic");
    delHelpChanSetCommand("founder");
    delHelpChanSetCommand("avoice");
    delHelpChanSetCommand("aop");
    delHelpChanSetCommand("nojoin");
    delHelpChanSetCommand("noauto");
    delHelpChanSetCommand("private");
    delHelpChanSetCommand("strictop");
    delHelpChanSetCommand("protectops");
    delHelpChanSetCommand("mlock");
    delHelpChanSetCommand("autolimit");
    delHelpChanSetCommand("secure");
    delHelpChanSetCommand("enablemask");
    delHelpChanSetCommand("mass");
    delHelpChanSetCommand("enftopic");
    delHelpChanSetCommand("axxflags");

    delHelpBotCommand("!owner");
    delHelpBotCommand("!deowner");
    delHelpBotCommand("!protect");
    delHelpBotCommand("!deprotect");
    delHelpBotCommand("!op");
    delHelpBotCommand("!deop");
    delHelpBotCommand("!halfop");
    delHelpBotCommand("!dehalfop");
    delHelpBotCommand("!voice");
    delHelpBotCommand("!devoice");
    delHelpBotCommand("!opall");
    delHelpBotCommand("!deopall");
    delHelpBotCommand("!halfopall");
    delHelpBotCommand("!dehalfopall");
    delHelpBotCommand("!voiceall");
    delHelpBotCommand("!devoiceall");
    delHelpBotCommand("!kick");
    delHelpBotCommand("!rkick");
    delHelpBotCommand("!kb");
    delHelpBotCommand("!rkb");
    delHelpBotCommand("!ban");
    delHelpBotCommand("!unban");
    delHelpBotCommand("!topic");
    delHelpBotCommand("!seen");
    delHelpBotCommand("!admin");

    delHelpOperCommand("stats");
    delHelpOperCommand("global");
    delHelpOperCommand("fakeuser");
    delHelpOperCommand("fakejoin");
    delHelpOperCommand("fakesay");
    delHelpOperCommand("fakenick");
    delHelpOperCommand("fakekill");
    delHelpOperCommand("operlist");
    delHelpOperCommand("userlist");
    delHelpOperCommand("nicklist");
    delHelpOperCommand("chanlist");
    delHelpOperCommand("jupe");
    delHelpOperCommand("killall");
    delHelpOperCommand("glineall");
    delHelpOperCommand("noexpire");
    delHelpOperCommand("suspend");
    delHelpOperCommand("changelev");
    delHelpOperCommand("trustadd");
    delHelpOperCommand("trustdel");
    delHelpOperCommand("trustlist");
    delHelpOperCommand("forceauth");
    delHelpOperCommand("modload");
    delHelpOperCommand("modunload");
    delHelpOperCommand("modlist");
    delHelpOperCommand("raw");
    delHelpOperCommand("savedb");
    delHelpOperCommand("rehash");
    delHelpOperCommand("restart");
    delHelpOperCommand("die");
    delHelpOperCommand("setraws");
    delHelpOperCommand("cmdlev");
#ifdef USE_FILTER
    delHelpOperCommand("reloadrules");
    delHelpOperCommand("ruleslist");
    delHelpOperCommand("setfilter");
#endif
    delHelpOperCommand("superadmin");
    delHelpOperCommand("sglobal");
    delHelpOperCommand("glinechan");
    delHelpOperCommand("regexpcheck");
}

void do_help (Nick *nptr, User *uptr, char *all)
{
    char blah[1024];
    strncpy(blah,all,1024);
    char *arg1,*arg2,*arg3;
    arg1 = blah;
    arg2 = SeperateWord(arg1);
    arg3 = SeperateWord(arg2);
    SeperateWord(arg3);

    if (!arg2 || *arg2 == '\0') {
        NoticeToUser(nptr,"To get help, type one of the following commands :");
        NoticeToUser(nptr," ");
        NoticeToUser(nptr,"\2/msg %s help nick\2",me.nick);
        NoticeToUser(nptr,"\2/msg %s help chan\2",me.nick);
        NoticeToUser(nptr,"\2/msg %s help bot\2",me.nick);
        if (uptr && (uptr->level >= 100) && (IsOper(nptr)) && IsAuthed(uptr)) {
            NoticeToUser(nptr," ");
            NoticeToUser(nptr,"Opers only commands :");
            NoticeToUser(nptr,"\2/msg %s help host\2",me.nick);
            NoticeToUser(nptr,"\2/msg %s help oper\2",me.nick);
        }
        return;
    }

    if (!Strcmp(arg2,"help"))
        arg2 = arg1;

    all = SeperateWord(all);
    all = SeperateWord(all);

    Command *cmd;
    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP)) {
        if (!Strcmp(cmd->name,arg2) && cmd->type == CMD_HELP) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr, "No help available for %s %s",arg2,arg3 ? arg3 : "");
}

void help_host (Nick *nptr, User *uptr, char *all)
{
    char *arg3 = all;
    all = SeperateWord(arg3);
    Command *cmd;
    unsigned int i,spacecount;
    char padding[ACTUAL_CMDLEN];

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Commands available for vhosts management :");
        NoticeToUser(nptr," ");

        LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_HOST)) {
            if (cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_HOST) {
                if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level)) {
                    bzero(padding,ACTUAL_CMDLEN);
                    spacecount = ACTUAL_CMDLEN - strlen(cmd->name);
                    for (i=0;i<spacecount;i++)
                        strcat(padding," ");
                    NoticeToUser(nptr,"\2%s\2%s%s",cmd->name,padding,cmd->desc);
                }
            }
        }
        return;
    }

    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_HOST)) {
        if (!Strcmp(cmd->name,arg3) && cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_HOST) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr, "No help available for host %s",arg3 ? arg3 : "");
}

void help_host_set (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2set \037nick\037 [\037vhost\037]\2");
    NoticeToUser(nptr,"Set someone's vhost");
}

void help_host_list (Nick *nptr)
{
    NoticeToUser(nptr,"List all users having a vhost");
}

void help_nick (Nick *nptr, User *uptr, char *all)
{
    char *arg3 = all;
    all = SeperateWord(arg3);
    Command *cmd;
    unsigned int i,spacecount;
    char padding[ACTUAL_CMDLEN];

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Commands available for nicks management :");
        NoticeToUser(nptr," ");

        LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_NICK)) {
            if (cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_NICK) {
                if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level)) {
                    bzero(padding,ACTUAL_CMDLEN);
                    spacecount = ACTUAL_CMDLEN - strlen(cmd->name);
                    for (i=0;i<spacecount;i++)
                        strcat(padding," ");
                    NoticeToUser(nptr,"\2%s\2%s%s",cmd->name,padding,cmd->desc);
                }
            }
        }
        return;
    }

    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_NICK)) {
        if (!Strcmp(cmd->name,arg3) && cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_NICK) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr, "No help available for nick %s",arg3 ? arg3 : "");
}

void help_nick_link (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK LINK \037nick\037 \037password\037\2");
    NoticeToUser(nptr,"Link your nick to another, you can use the access list of the remote nick.");
    NoticeToUser(nptr,"If you want to share the access list of another nick, use the command \2nick set master\2");
}

void help_nick_unlink (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK UNLINK \037nick\037\2");
    NoticeToUser(nptr,"Unlink your nick");
}

void help_nick_identify (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK IDENTIFY \037password\037\2");
    NoticeToUser(nptr,"Identify yourself to services");
}

void help_nick_register (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK REGISTER \037password\037 \037email\037\2");
    NoticeToUser(nptr,"Register your nick with the specified password and email");
    NoticeToUser(nptr,"Nicks expire after %d days without being used",me.nick_expire);
}

void help_nick_drop (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK DROP [\037nick\037]\2");
    NoticeToUser(nptr,"Drop your account");
}

void help_nick_info (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK INFO \037nick\037\2");
    NoticeToUser(nptr,"Retrieve informations about the specified nick");
}

void help_nick_ghost (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK GHOST \037nick\037 \037password\037\2");
    NoticeToUser(nptr,"Kill a ghosted nick.");
}

void help_nick_set (Nick *nptr, User *uptr, char *all)
{
    unsigned int i,spacecount;
    char padding[ACTUAL_CMDLEN];

    char *arg4 = all;
    SeperateWord(arg4);

    Command *cmd;

    if (!arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2NICK SET \037option\037 \037value\037");
        NoticeToUser(nptr," ");
        LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_NICK_SET)) {
            if (cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_NICK_SET) {
                if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level)) {
                    bzero(padding,ACTUAL_CMDLEN);
                    spacecount = ACTUAL_CMDLEN - strlen(cmd->name);
                    for (i=0;i<spacecount;i++)
                        strcat(padding," ");
                    NoticeToUser(nptr,"\2%s\2%s%s",cmd->name,padding,cmd->desc);
                }
            }
        }
        return;
    }

    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_NICK_SET)) {
        if (!Strcmp(cmd->name,arg4) && cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_NICK_SET) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr,"No help available for \2set %s\2",arg4);
}

void help_nick_set_password (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK SET PASSWORD [\037username\037] \037newpass\037\2");
    NoticeToUser(nptr,"Change your password, or the password of the specified nick");
}

void help_nick_set_email (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK SET EMAIL \037email@domain.xx\037\2");
    NoticeToUser(nptr,"Set your email address. This is required for requestpassword command");
}

void help_nick_set_private (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK SET PRIVATE [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"Hide your informations (level, status, lastseen, etc).");
}

void help_nick_set_protect (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK SET PROTECT [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"Enable or disable the nick protection. When the protection is enabled,");
    NoticeToUser(nptr,"if someone take your nick and does not identify, his nick will be changed");
    NoticeToUser(nptr,"after a specified time. See \2help set timeout\2 for the time limit.");
}

void help_nick_set_timeout (Nick *nptr) {
    NoticeToUser(nptr,"Syntax: \2NICK SET TIMEOUT \037time\037\2");
    NoticeToUser(nptr,"Set the time limit in seconds for the nick protection.");
    NoticeToUser(nptr,"The default value is %d. Setting the time to 0 will set the timeout to the default value.",TIMEOUT_DFLT);
}

void help_nick_set_master (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK SET MASTER \037nick\037\2");
    NoticeToUser(nptr,"Set the nick whose access list is shared when several nicks are linked.");
}

void help_nick_set_noauto (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK SET NOAUTO [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"This option prevents from setting modes on ident");
}

void help_nick_set_hidemail (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK SET HIDEMAIL [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"This option allows you to hide your email from other users");
}

void help_nick_set_cloak (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK SET CLOAK [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"Enable or disable host cloaking. Your cloak will be \2%s%s\2. Note that the cloak never overrides your vhost.",nptr->nick,me.usercloak);
}

void help_nick_requestpassword (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2NICK REQUESTPASSWORD \037username\037\2");
    NoticeToUser(nptr,"Generate a new password and send it to the user's email address.");
}

void help_chan (Nick *nptr, User *uptr, char *all)
{
    char *arg3 = all;
    all = SeperateWord(arg3);
    Command *cmd;
    unsigned int i,spacecount;
    char padding[ACTUAL_CMDLEN];
    
    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Commands available for channels management :");
        NoticeToUser(nptr," ");
        
        LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_CHAN)) {
            if (cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_CHAN) {
                if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level)) {
                    bzero(padding,ACTUAL_CMDLEN);
                    spacecount = ACTUAL_CMDLEN - strlen(cmd->name);
                    for (i=0;i<spacecount;i++)
                        strcat(padding," ");
                    NoticeToUser(nptr,"\2%s\2%s%s",cmd->name,padding,cmd->desc);
                }
            }
        }
        return;
    }

    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_CHAN)) {
        if (!Strcmp(cmd->name,arg3) && cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_CHAN) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr,uptr,all);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr, "No help available for chan %s",arg3 ? arg3 : "");
}

void help_chan_register (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN REGISTER \037#channel\037\2");
    NoticeToUser(nptr,"Register specified channel");
    NoticeToUser(nptr,"Chans expire after %d days without being used.",me.chan_expire);
}

void help_chan_drop (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN DROP \037#channel\037\2");
    NoticeToUser(nptr,"Drop specified channel");
}

void help_chan_access (Nick *nptr, User *uptr __unused, char *all)
{
    char *arg1 = all;
    SeperateWord(arg1);
    if (!arg1 || *arg1 == '\0') {
        NoticeToUser(nptr, "Syntax: \2HELP CHAN ACCESS FLAGS\2 for help about chanflags");
        NoticeToUser(nptr, "Syntax: \2HELP CHAN ACCESS LEVELS\2 for help about old levels system");
        NoticeToUser(nptr, "The CHAN ACCESS command automatically adapts himself regarding to the AXXFLAGS channel option.");
        return;
    }

    if (!Strcmp(arg1,"levels")) {
        NoticeToUser(nptr,"Syntax: \2CHAN ACCESS \037#channel\037 {add|auto|del|list} [\037nick\037 [\037level\037]]\2");
        NoticeToUser(nptr," ");
        NoticeToUser(nptr,"Following access levels are available :");
        NoticeToUser(nptr,"      %d  Co-Owner. Get +qo modes and inherits channel ownership if the original owner expires.", CHLEV_COOWNER);
        NoticeToUser(nptr,"      %d   Full control over the channel without DROP nor SET FOUNDER",me.chlev_sadmin);
        NoticeToUser(nptr,"      %d   can use access add/del command and set some options",me.chlev_admin);
        NoticeToUser(nptr,"       %d   auto-op",me.chlev_op);
        NoticeToUser(nptr,"       %d   auto-halfop",me.chlev_halfop);
        NoticeToUser(nptr,"       %d   auto-voice",me.chlev_voice);
        NoticeToUser(nptr,"       %d   can use invite command",me.chlev_invite);
        NoticeToUser(nptr,"      %d   cannot get opped",me.chlev_nostatus);
        NoticeToUser(nptr,"      %d   auto-kicked",me.chlev_akick);
        NoticeToUser(nptr,"      %d   auto-banned",me.chlev_akb);
        NoticeToUser(nptr,"The maximum level is %d.",CHLEV_OWNER-1);
        NoticeToUser(nptr, " ");
        NoticeToUser(nptr,"Usage of AUTO param :");
        NoticeToUser(nptr,"CHAN ACCESS \037#channel\037 AUTO \037nick\037 {op|voice|default|off}");
        NoticeToUser(nptr,"OP: the user is auto-opped if his/her level >= %d", me.chlev_op);
        NoticeToUser(nptr,"VOICE: the user is auto-voiced if his/her level >= %d", me.chlev_voice);
        NoticeToUser(nptr,"DEFAULT: the user is set the mode corresponding to his level");
        NoticeToUser(nptr,"OFF: no auto-mode is set");
    } else if (!Strcmp(arg1, "flags")) {
        NoticeToUser(nptr, "Syntax: \2CHAN ACCESS \037#channel\037 [\037username\037 {+|-}\037flags\037]\2");
        NoticeToUser(nptr, " ");
        NoticeToUser(nptr, "Following channel flags are available :");
        NoticeToUser(nptr, "    \2o\2   Can get/give/take op");
        NoticeToUser(nptr, "    \2O\2   Get auto-op on join");
        NoticeToUser(nptr, "    \2v\2   Can get voice");
        NoticeToUser(nptr, "    \2V\2   Get auto-voice on join");
        NoticeToUser(nptr, "    \2x\2   Can modify access list");
        NoticeToUser(nptr, "    \2i\2   Can get invited");
        NoticeToUser(nptr, "    \2t\2   Can change topic");
        NoticeToUser(nptr, "    \2F\2   Channel owner, can only be modified with CHAN SET FOUNDER command");
        NoticeToUser(nptr, "    \2f\2   Channel co-owner");
        NoticeToUser(nptr, "    \2h\2   Can get halfop");
        NoticeToUser(nptr, "    \2H\2   Get auto-halfop on join");
        NoticeToUser(nptr, "    \2s\2   Can use CHAN SET commands");
        NoticeToUser(nptr, "    \2p\2   Can get protect status");
        NoticeToUser(nptr, "    \2P\2   Get auto-protect on join (+a mode)");
        NoticeToUser(nptr, "    \2N\2   Can't get any status (op/voice/etc)");
        NoticeToUser(nptr, "    \2k\2   Auto-kick on join");
        NoticeToUser(nptr, "    \2b\2   Auto-kickban on join");
        NoticeToUser(nptr, "    \2w\2   Get auto-owner on join (+q mode)");
        NoticeToUser(nptr, " ");
        NoticeToUser(nptr, "Channel flags are cumulative. Example: +oOV flags will give auto-op and auto-voice on join.");
    } else
        NoticeToUser(nptr, "Unknown argument, see HELP CHAN ACCESS");
}

void help_chan_op (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN OP #channel [\037nick\037]\2");
    NoticeToUser(nptr,"Give channel operator status");
}

void help_chan_deop (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN DEOP #channel [\037nick\037]\2");
    NoticeToUser(nptr,"Take channel operator status");
}

void help_chan_halfop (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN HALFOP #channel [\037nick\037]\2");
    NoticeToUser(nptr,"Give channel half-operator status");
}          

void help_chan_dehalfop (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN DEHALFOP #channel [\037nick\037]\2");
    NoticeToUser(nptr,"Take channel half-operator status");
}           

void help_chan_voice (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN VOICE #channel [\037nick\037]\2");
    NoticeToUser(nptr,"Give voice status");
}           

void help_chan_devoice (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN DEVOICE #channel [\037nick\037]\2");
    NoticeToUser(nptr,"Take voice status");
}           

void help_chan_invite (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN INVITE \037#channel\037\2");
    NoticeToUser(nptr,"Invite yourself on a channel");
}

void help_chan_kick (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN KICK \037#channel\037 \037nick\037 [\037reason\037]\2");
    NoticeToUser(nptr,"Kick someone or all users");
}

void help_chan_info (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN INFO #channel\2");
    NoticeToUser(nptr,"Give some informations for the specified channel");
}

void help_chan_unbanall (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN UNBANALL #channel\2");
    NoticeToUser(nptr,"Remove all bans");
}

void help_chan_clearmodes (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN CLEARMODES #channel\2");
    NoticeToUser(nptr,"Remove all channel modes");
}

void help_chan_clearchan (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN CLEARCHAN #channel\2");
    NoticeToUser(nptr,"Remove all channel modes, bans, excepts, invex, user status, ...");
}

void help_chan_topic (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN TOPIC \037#chan\037 [\037topic\037]\2");
    NoticeToUser(nptr,"Set the topic");
}

void help_chan_set (Nick *nptr, User *uptr, char *all)
{
    unsigned int i,spacecount;
    char padding[ACTUAL_CMDLEN];

    char *arg4 = all;
    SeperateWord(arg4);

    Command *cmd;

    if (!arg4 || *arg4 == '\0') {
        NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 \037option\037 \037value\037\2");
        NoticeToUser(nptr," ");
        LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_CHAN_SET)) {
            if (cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_CHAN_SET) {
                if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level)) {
                    bzero(padding,ACTUAL_CMDLEN);
                    spacecount = ACTUAL_CMDLEN - strlen(cmd->name);
                    for (i=0;i<spacecount;i++)
                        strcat(padding," ");
                    NoticeToUser(nptr,"\2%s\2%s%s",cmd->name,padding,cmd->desc);
                }
            }
        }
        return;
    }

    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_CHAN_SET)) {
        if (!Strcmp(cmd->name,arg4) && cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_CHAN_SET) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr,"No help available for \2set %s\2",arg4);
}

void help_chan_set_founder (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 founder \037user\037\2");
    NoticeToUser(nptr,"Change the channel founder.");
}

void help_chan_set_avoice (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 avoice [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"When the option is enabled, users are automatically voiced.");
    NoticeToUser(nptr,"A minimum level of 10 is required.");
}

void help_chan_set_aop (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 aop [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"WHen the option is enabled, users are automatically opped.");
    NoticeToUser(nptr,"A minimum level of 20 is required.");
}

void help_chan_set_nojoin (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 nojoin [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"When this option is enabled, %s will leave your channel, operating from outside.",me.nick);
    NoticeToUser(nptr,"A minimum level of 20 is required.");
}

void help_chan_set_noauto (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 noauto [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"Defaultly, when a user join a channel where he has an access, he automatically gets the modes");
    NoticeToUser(nptr,"correponding to his access. When this option is enabled, %s will not automatically gives modes.",me.nick);
    NoticeToUser(nptr,"A minimum level of 20 is required");
}

void help_chan_set_private (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 private [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"Define if your channel information are private or public.");
    NoticeToUser(nptr,"A minimum level of 20 is required");
}

void help_chan_set_strictop (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 strictop [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"When this option is enabled, only the users having a sufficient level can be opped");
    NoticeToUser(nptr,"A minimum level of 20 is required.");
}

void help_chan_set_protectops (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 protectops [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"This option prevents ops having an access from being deopped");
    NoticeToUser(nptr,"The protection can be overridden by the !deop command.");
}

void help_chan_set_mlock (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 mlock [\037modes\037]\2");
    NoticeToUser(nptr,"This define the modes who must be forcefully set. If no param is given after the '+' or the '-', mlock is cleared.");
    NoticeToUser(nptr,"A minimum level of 20 is required.");
}

void help_chan_set_autolimit (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 autolimit [\037limit\037]\2");
    NoticeToUser(nptr,"Forcefully set a limit. The limit depends on the numbers of the members present on the channel");
    NoticeToUser(nptr,"E.g.: If there is 5 members and an autolimit set to 3, %s will set the limit to 8",me.nick);
}

void help_chan_set_secure (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 secure [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"If this option is enabled, users who are not identified won't be auto-opped or auto-voiced if the aop or avoice option are set.");
}

void help_chan_set_enablemask (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 enablemask [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"This option allows masks in access list. Example: you can set a level of -2 on *!*@lamerhost.net to autokickban all users matching this mask");
    NoticeToUser(nptr,"Note that the SECURE option has no effect with masks, and that masks are only effective with auto-voice/halfop/op features (you can't add a mask with a level > 5)");
}

void help_chan_set_mass (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 mass [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"This option enable or disable mass-effect channel comands like !rkick, !rkb, !kick * and !kb *");
}

void help_chan_set_enftopic (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN SET \037#channel\037 enftopic [\037on\037|\037off\037]\2");
    NoticeToUser(nptr,"This option defines whether the services enforces the topic defined by the CHAN TOPIC command.");
}

void help_chan_set_axxflags (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2CHAN SET \037#channel\037 axxflags [\037on\037|\037off\037]\2");
    NoticeToUser(nptr, "Enable or disable chanflags system. The CHAN ACCESS command automatically adapts itself regarding to this option. See HELP CHAN ACCESS for more informations about chanflags.");
}

void help_chan_entrymsg (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN ENTRYMSG \037#channel\037 [\037message\037]\2");
    NoticeToUser(nptr,"Set a welcome message displayed on join. If no parameter is specified, then entrymsg is removed");
}

void help_chan_assign (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN ASSIGN \037#channel\037 \037botnick\037\2");
    NoticeToUser(nptr,"Assign a bot to a channel. This will automatically set NOJOIN option and make %s leaving the channel.",me.nick);
}

void help_chan_unassign (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN UNASSIGN \037#channel\037\2");
    NoticeToUser(nptr,"Unassign a bot from a channel.");
}

void help_chan_botlist (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN BOTLIST\2");
    NoticeToUser(nptr,"List all available bots");
}

void help_chan_addbot (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN ADDBOT \037nick\037 \037ident\037 \037host\037\2");
    NoticeToUser(nptr,"Add a bot");
}

void help_chan_delbot (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2CHAN DELBOT \037nick\037\2");
    NoticeToUser(nptr,"Delete a bot");
}

void help_chan_resync (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2CHAN RESYNC \037#channel\037\2");
    NoticeToUser(nptr, "Resync channel access list with channel members");
}

void help_chan_suspend (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2CHAN SUSPEND \037#channel\037 \037username\037\2");
    NoticeToUser(nptr, "Suspend the access level of a user");
}

void help_chan_banlist (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2CHAN BANLIST \037#channel\037\2");
    NoticeToUser(nptr, "List channel bans (added with !tb command)");
}

void help_chan_clearbans (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2CHAN CLEARBANS \037#channel\037\2");
    NoticeToUser(nptr, "Clear internal channel banlist.");
}

void help_chan_unsuspend (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2CHAN UNSUSPEND \037#channel\037 \037username\037\2");
    NoticeToUser(nptr, "Unsuspend the access level of a user");
}

void help_bot (Nick *nptr, User *uptr, char *all)
{
    char *arg3 = all;
    all = SeperateWord(arg3);
    Command *cmd;
    unsigned int i,spacecount;
    char padding[ACTUAL_CMDLEN];

    NoticeToUser(nptr,"Commands available in channels :");
    NoticeToUser(nptr," ");

    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_BOT)) {
        if (cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_BOT) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level)) {
                bzero(padding,ACTUAL_CMDLEN);
                spacecount = ACTUAL_CMDLEN - strlen(cmd->name);
                for (i=0;i<spacecount;i++)
                    strcat(padding," ");
                NoticeToUser(nptr,"\2%s\2%s%s",cmd->name,padding,cmd->desc);
            }
        }
    }
}

void help_oper (Nick *nptr, User *uptr, char *all)
{
    char *arg3 = all;
    all = SeperateWord(arg3);
    Command *cmd;
    unsigned int i,spacecount;
    char padding[ACTUAL_CMDLEN];

    if (!IsOper(nptr) || !IsAuthed(uptr) || uptr->level < 100) {
        NoticeToUser(nptr,"Access denied");
        return;
    }

    if (!arg3 || *arg3 == '\0') {
        NoticeToUser(nptr,"Commands available for administration :");
        NoticeToUser(nptr," ");

        LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_OPER)) {
            if (cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_OPER) {
                if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level)) {
                    bzero(padding,ACTUAL_CMDLEN);
                    spacecount = ACTUAL_CMDLEN - strlen(cmd->name);
                    for (i=0;i<spacecount;i++)
                        strcat(padding," ");
                    NoticeToUser(nptr,"\2%s\2%s%s",cmd->name,padding,cmd->desc);
                }
            }
        }
        return;
    }

    LIST_FOREACH(command_list, cmd, HASH_INT(CMD_HELP+CMD_HELP_OPER)) {
        if (!Strcmp(cmd->name,arg3) && cmd->type == CMD_HELP && cmd->subtype == CMD_HELP_OPER) {
            if ((!IsAuthed(uptr) && cmd->level == 0) || (IsAuthed(uptr) && uptr->level >= cmd->level))
                cmd->func(nptr);
            else
                NoticeToUser(nptr,"Access denied");
            return;
        }
    }

    NoticeToUser(nptr, "No help available for oper %s",arg3 ? arg3 : "");
}

void help_oper_stats (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2stats\2");
    NoticeToUser(nptr,"Display some informations about the network and the services");
}

void help_oper_global (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2global \037message\037\2");
    NoticeToUser(nptr,"Send a notice to each user connected on the network");
}

void help_oper_sglobal (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2sglobal \037server\037 \037message\037\2");
    NoticeToUser(nptr,"Send a server-wide notice");
}

void help_oper_fakeuser (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2fakeuser \037nick\037 \037ident\037 \037host\037\2");
    NoticeToUser(nptr,"Create a fakeuser");
}

void help_oper_fakejoin (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2fakejoin \037nick\037 \037#channel\037\2");
    NoticeToUser(nptr,"Make a fakeuser join a channel");
}

void help_oper_fakesay (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2fakesay \037nick\037 \037#channel\037 \037message\037\2");
    NoticeToUser(nptr,"Make a fakeuser say something");
}

void help_oper_fakenick (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2fakenick \037oldnick\037 \037newnick\037\2");
    NoticeToUser(nptr,"Change the nick of a fakeuser");
}

void help_oper_fakelist (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2fakenick\2");
    NoticeToUser(nptr, "List fakeusers created with OPER FAKEUSER");
}

void help_oper_fakekill (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2fakekill \037nick\037 [\037quit message\037]\2");
    NoticeToUser(nptr,"Destroy a fakeuser");
}

void help_oper_userlist (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2userlist [\037pattern\037]\2");
    NoticeToUser(nptr,"Without the pattern, display all users registered. With the pattern, display all users matching it");
}

void help_oper_nicklist (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2nicklist [\037pattern\037]\2");
    NoticeToUser(nptr,"Without the pattern, display all users connected. With the pattern, display all users whose nick, ident or host are matching the pattern");
}

void help_oper_chanlist (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2chanlist [\037pattern\037]\2");
    NoticeToUser(nptr,"Without the pattern, display all registered channels. With the pattern, display all channels of which name or founder's nick are matching the pattern");
}

void help_oper_jupe (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2jupe \037servername\037 \037reason\037\2");
    NoticeToUser(nptr,"Jupe a server");
}

void help_oper_killall (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2killall \037mask\037\2");
    NoticeToUser(nptr,"Kill all users matching the mask");
}

void help_oper_glineall (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2glineall \037mask\037\2");
    NoticeToUser(nptr,"Gline all users matching the mask");
}

void help_oper_glinechan (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2glinechan \037#channel\037 \037duration\037 \037reason\037\2");
    NoticeToUser(nptr, "Gline all non-oper members of a channel. Duration is to be specified in seconds, 0 for permanent gline.");
}

void help_oper_changelev (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2changelev \037username\037 \037level\037\2");
    NoticeToUser(nptr,"Change the level of a user");
    NoticeToUser(nptr,"Here is the differents levels :");
    NoticeToUser(nptr,"  \2 1-%d\2     : normal user",me.level_oper-1);
    NoticeToUser(nptr,"  \2 %d-%d\2  : services operator",me.level_oper,me.level_admin-1);
    NoticeToUser(nptr,"  \2 %d-%d\2  : services administrator",me.level_admin,me.level_root-1);
    NoticeToUser(nptr,"  \2 %d-%d\2 : services root administrator",me.level_root,me.level_owner);
}

void help_oper_noexpire (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2noexpire [\037username\037|\037#channel\037] [on|off]\2");
    NoticeToUser(nptr,"Set/unset noexpire flag for a user or a channel");
}

void help_oper_suspend (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2suspend [\037username\037|\037#channel\037] [on|off]\2");
    NoticeToUser(nptr,"Suspend/unsuspend a user or a channel");
}

void help_oper_forceauth (Nick *nptr)
{ 
    NoticeToUser(nptr,"Syntax: \2forceauth \037nick\037\2");
    NoticeToUser(nptr,"Force someone to auth. Use it carefully...");
}

void help_oper_raw (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2raw \037commands\037\2");
    NoticeToUser(nptr,"Execute a raw (%s)",raws ? "enabled" : "disabled");
    NoticeToUser(nptr,"This command is very DANGEROUS and might be disabled.");
}

void help_oper_savedb (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2savedb\2");
    NoticeToUser(nptr,"Save the database");
}

void help_oper_restart (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2restart\2");
    NoticeToUser(nptr,"Restart services");
}

void help_oper_die (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2die\2");
    NoticeToUser(nptr,"Stop services");
}

void help_oper_operlist (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2operlist\2");
    NoticeToUser(nptr,"Display online irc operators");
}

void help_oper_trustadd (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2trustadd \037hostname/ip\037 \037limit\037\2");
    NoticeToUser(nptr,"Add a trust for the specified hostname or ip. CIDR can be used. The limit is per-ip, not per-range");
}

void help_oper_trustdel (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2trustdel \037trust\037\2");
    NoticeToUser(nptr,"Remove an existing trust");
}

void help_oper_trustlist (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2trustlist\2");
    NoticeToUser(nptr,"List existing trusts");
}

void help_oper_modload (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2modload \037modulename\037\2");
    NoticeToUser(nptr,"Load a module. \037modulename\037 is the file name without the .so extension");
}

void help_oper_modunload (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2modunload \037modulename\037\2");
    NoticeToUser(nptr,"Remove a loaded module");
}

void help_oper_modlist (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2modlist\2");
    NoticeToUser(nptr,"List loaded modules");
}

void help_oper_rehash (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2rehash\2");
    NoticeToUser(nptr,"Reload configuration");
}

void help_oper_setraws (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2setraws \0370|1\037\2");
    NoticeToUser(nptr,"Enable/disable raws");
}

void help_oper_cmdlev (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2cmdlev \037name\037 \037type\037 \037subtype\037 \037new level\037\2");
    NoticeToUser(nptr,"Change the required level to use a command");
    NoticeToUser(nptr,"Example: cmdlev chan register 100");
}

#ifdef USE_FILTER
void help_oper_ruleslist (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2ruleslist\2");
    NoticeToUser(nptr,"List all filter rules");
}

void help_oper_reloadrules (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2reloadrules\2");
    NoticeToUser(nptr,"Reload filter rules");
}

void help_oper_setfilter (Nick *nptr)
{
    NoticeToUser(nptr,"Syntax: \2setfilter [0|1]\2");
    NoticeToUser(nptr,"Display or set the filter status (disabled or enabled, 0 or 1)");
}
#endif

void help_oper_superadmin (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2superadmin [\037on\037|\037off\037]\2");
    NoticeToUser(nptr, "Once superadmin mode is enabled, you are recognized as the founder of every registered channel. Use with caution.");
}

void help_oper_regexpcheck (Nick *nptr)
{
    NoticeToUser(nptr, "Syntax: \2regexpcheck \037nick!ident@host\037\2");
    NoticeToUser(nptr, "Check users affected by a given regular expression. Very useful to avoid spamfilter fails.");
}
