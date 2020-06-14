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
#include "modules.h"
#include "stdio.h"

#include <prom.h>
#include <promhttp.h>
#include <microhttpd.h>


char *modname = "prometheus";
void child_init();
void child_cleanup();
int(metrics)(Nick *, User *, Chan *, char *[]);

prom_counter_t *child_version;

void child_init()
{
    prom_collector_registry_default_init();
    AddHook(HOOK_PRE_PRIVMSG,&metrics,"prometheus",modname);
    promhttp_set_active_collector_registry(NULL);

    child_version = prom_collector_registry_must_register_metric(
        prom_counter_new(
            "child_version",
            "Child version",
            1,
            "version")
        );
    prom_counter_inc(child_version, "2.0");

    struct MHD_Daemon *daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        operlog("[Prometheus] Failed to start the exporter");
        fprintf(stderr,"prometheus - Can't start microhttpd");
    }
    printf("[Prometheus] - exporter started on http://localhost:8000/metrics");
}

void child_cleanup()
{
    MsgToChan("#stats","[Prometheus] Help! I'm being unloaded !");
    prom_collector_registry_destroy(PROM_COLLECTOR_REGISTRY_DEFAULT);
}

int metrics (Nick *nptr, User *uptr, Chan *chptr, char *parv[])
{
    // char cmd[1024];
    // bzero(cmd,1024);

    // char *badword_list[] = {"rezal"};
    // unsigned int i;

    // strncpy(cmd,parv[1],1023);

    // if (!Strcmp(nptr->nick,"target0")) {
    //     return MOD_CONTINUE;
    // }

    // for (i=0;i<strlen(cmd);i++) {
    //     cmd[i] = tolower(cmd[i]);
    //     if (cmd[i] == '�' || cmd[i] == '�')
    //         cmd[i] = 'e';
    //     if (cmd[i] == '�')
    //         cmd[i] = 'a';
    // }

    // for (i=0;i<(sizeof(badword_list)/sizeof(char *));i++) {
    //     if (strstr(cmd,badword_list[i])) {
    //         killuser(nptr->nick,"Et paf, la souris0",me.nick);
    //         operlog("Badwords: %s said %s: KILLED",nptr->nick,badword_list[i]);
    //         return MOD_STOP;
    //     }
    // }

    return MOD_CONTINUE;
}
