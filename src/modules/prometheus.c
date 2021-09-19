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

#include "prom.h"
#include "promhttp.h"
#include "microhttpd.h"


char *modname = "prometheus";
void child_init();
void child_cleanup();
void initialize_metrics();
int accounts_total_inc(Nick *, User *, Chan *, char **);
int accounts_total_dec(Nick *, User *, Chan *, char **);
int channels_total_inc(Nick *, User *, Chan *, char **);
int channels_total_dec(Nick *, User *, Chan *, char **);
int hook_runs_total_inc(Nick *, User *, Chan *, char **);
int users_total_inc(Nick *, User *, Chan *, char **);
int users_total_dec(Nick *, User *, Chan *, char **);

prom_counter_t *child_build_info;
prom_gauge_t *child_accounts_total;
prom_gauge_t *child_channels_total;
prom_gauge_t *child_users_total;
prom_counter_t *child_hook_runs_total;

void child_init()
{
    prom_collector_registry_default_init();
    promhttp_set_active_collector_registry(NULL);

    initialize_metrics();

    struct MHD_Daemon *daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr,"[prometheus] - Can't start microhttpd\n");
    }
}


void child_cleanup()
{
    prom_collector_registry_destroy(PROM_COLLECTOR_REGISTRY_DEFAULT);
}


void initialize_metrics () {
    child_build_info = prom_collector_registry_must_register_metric(
        prom_counter_new(
            "child_build_info",
            "Child version",
            1,
            (const char *[]) {"version"})
        );
    prom_counter_inc(child_build_info, (const char *[]) {"2.0-dev"});

    child_accounts_total = prom_collector_registry_must_register_metric(
        prom_gauge_new(
            "child_accounts_total",
            "Accounts registered",
            0,
            NULL)
        );
    prom_gauge_set(child_accounts_total, 0, NULL);
    AddHook(HOOK_CREATE_ACCOUNT, &accounts_total_inc, "accounts_total_inc", "prometheus");
    AddHook(HOOK_DELETE_ACCOUNT, &accounts_total_dec, "accounts_total_dec", "prometheus");

    child_channels_total = prom_collector_registry_must_register_metric(
        prom_gauge_new(
            "child_channels_total",
            "Channels registered",
            0,
            NULL)
        );
    prom_gauge_set(child_channels_total, 0, NULL);
    AddHook(HOOK_CREATE_CHANNEL, &channels_total_inc, "channels_total_inc", "prometheus");
    AddHook(HOOK_DELETE_CHANNEL, &channels_total_dec, "channels_total_dec", "prometheus");

    child_users_total = prom_collector_registry_must_register_metric(
        prom_gauge_new(
            "child_users_total",
            "Users logged in",
            0,
            NULL)
        );
    AddHook(HOOK_POST_LOGIN, &users_total_inc, "users_total_inc", "prometheus");
    AddHook(HOOK_QUIT, &users_total_dec, "users_total_dec", "prometheus");

    child_hook_runs_total = prom_collector_registry_must_register_metric(
        prom_counter_new(
            "child_hook_runs_total",
            "Count of hooks fired",
            2,
            (const char *[]) {"hook","module"})
        );
    AddHook(HOOK_RUNHOOK, &hook_runs_total_inc, "hook_runs_total_inc", "prometheus");
}

int accounts_total_inc(__unused Nick *nptr, __unused User *uptr, __unused Chan *cptr, __unused char *parv[]) {
    prom_gauge_inc(child_accounts_total, NULL);
    return MOD_CONTINUE;
}

int accounts_total_dec(__unused Nick *nptr, __unused User *uptr, __unused Chan *cptr, __unused char *parv[]) {
    prom_gauge_dec(child_accounts_total, NULL);
    return MOD_CONTINUE;
}

int channels_total_inc(__unused Nick *nptr, __unused User *uptr, __unused Chan *cptr, __unused char *parv[]) {
    prom_gauge_inc(child_channels_total, NULL);
    return MOD_CONTINUE;
}

int channels_total_dec(__unused Nick *nptr, __unused User *uptr, __unused Chan *cptr, __unused char *parv[]) {
    prom_gauge_dec(child_channels_total, NULL);
    return MOD_CONTINUE;
}

int hook_runs_total_inc(__unused Nick *nptr, __unused User *uptr, __unused Chan *cptr, char *parv[]) {
    prom_counter_inc(child_hook_runs_total, (const char *[]){parv[0],parv[1]});
    return MOD_CONTINUE;
}

int users_total_inc(__unused Nick *nptr, __unused User *uptr, __unused Chan *cptr, __unused char *parv[]) {
    prom_gauge_inc(child_users_total, NULL);
    return MOD_CONTINUE;
}

int users_total_dec(__unused Nick *nptr, __unused User *uptr, __unused Chan *cptr, __unused char *parv[]) {
    prom_gauge_dec(child_users_total, NULL);
    return MOD_CONTINUE;
}
