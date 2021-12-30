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


#include <child.h>
#include <globals.h>


char *modname = "badwords";
void child_init();
int (badwords)(Nick *, User *, Chan *, char *[]);

void child_init()
{
    AddHook(HOOK_PRE_PRIVMSG,&badwords,"badwords",modname);
}

void child_cleanup()
{
    MsgToChan("#stats","[Badwords] Help! I'm being unloaded !");
}

int badwords (Nick *nptr, User *uptr, Chan *chptr, char *parv[])
{
    char cmd[1024];
    bzero(cmd,1024);

    char *badword_list[] = {"rezal"};
    unsigned int i;

    strncpy(cmd,parv[1],1023);

    if (!Strcmp(nick_name(nptr),"target0")) {
        return MOD_CONTINUE;
    }

    for (i=0;i<strlen(cmd);i++) {
        cmd[i] = tolower(cmd[i]);
        if (cmd[i] == 'é' || cmd[i] == 'è')
            cmd[i] = 'e';
        if (cmd[i] == 'à')
            cmd[i] = 'a';
    }

    for (i=0;i<(sizeof(badword_list)/sizeof(char *));i++) {
        if (strstr(cmd,badword_list[i])) {
            killuser(nick_name(nptr),"Et paf, la souris0",core_get_config()->nick);
            operlog("Badwords: %s said %s: KILLED",nick_name(nptr),badword_list[i]);
            return MOD_STOP;
        }
    }

    return MOD_CONTINUE;
}
