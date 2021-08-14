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

#include "botserv.h"
#include "channel.h"
#include "commands.h"
#include "core.h"
#include "core_api.h"
#include "db.h"
#include "logging.h"
#include "modules.h"
#include "string_utils.h"
#include "trust.h"
#include "user.h"

#include <errno.h>
#include <execinfo.h>
#include <mysql/mysql.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void sighandler (int signal)
{
    int status;

    switch (signal) {
        case SIGHUP:
            loadconf(1);
            break;
        case SIGUSR2:
            operlog("Got SIGUSR2, reconnecting to server");
            get_core_api()->net_disconnect(/*graceful=*/true);
            gettimeofday(&get_core()->next_connect_time, NULL);
            break;
        case SIGINT:
            operlog("SIGINT signal received, exiting");
            fakekill(core_get_config()->nick,"Got SIGINT, exiting");
            child_die(1);
            break;
        case SIGCHLD:
            waitpid(-1,&status,WNOHANG);
            break;
        case SIGPIPE:
            operlog("Got SIGPIPE, ignored");
            break;
        case SIGSEGV:
            operlog("Segmentation fault");
            // from https://stackoverflow.com/questions/77005/how-to-automatically-generate-a-stacktrace-when-my-program-crashes
            void *array[10];
            size_t size;

            // get void*'s for all entries on the stack
            size = backtrace(array, 10);

            // print out all the frames to stderr
            fprintf(stderr, "Segmentation fault:\n");
            backtrace_symbols_fd(array, size, STDERR_FILENO);
            raise(SIGSEGV);
        }
}

static void usage (char *progname)
{
    fprintf(stderr,"Usage: %s [-dv]\n\t-d\tDo not daemonize\n\t-v\tBe verbose (use twice for more verbose)\n",progname);
    exit(-1);
}

static void write_pid()
{
    FILE *fp;
    char pid[16];
    sprintf(pid,"%d\n",getpid());
    fp = fopen("child.pid","w");
    if (!fp) return;
    fputs(pid,fp);
    fclose(fp);
}

void child_clean()
{
    free_core();
    exit(0);
}

void child_die(int save)
{
    if (save) savealldb();
    mysql_close(&get_core()->mysql_handle);
    killallfakes();
    unloadallmod();
    get_core_api()->net_disconnect(/*graceful=*/true);
    unlink("child.pid");
    child_clean();
}

// TODO(target0): improve this.
void child_restart(int save)
{
    if (save) savealldb();
    mysql_close(&get_core()->mysql_handle);
    killallfakes();
    unloadallmod();
    get_core_api()->net_disconnect(/*graceful=*/true);

    char buf[1024];
    char dir[1024];
    if (getcwd(buf,1024) == NULL) {
        fprintf(stderr, "Failed to getcwd(%s) for restart: %s\n", buf, strerror(errno));
        abort();
    }
    snprintf(dir,1024,"%s/child",buf);
    // Note that this only checks for child process creation and not execution errors.
    // See system(3).
    if (system(dir) < 0) {
        fprintf(stderr, "Failed to system(%s) for restart: %s\n", dir, strerror(errno));
        abort();
    }
    child_clean();
}

static void child_run_loop_once(void)
{
    struct core_api *api = get_core_api();
    struct core *core = get_core();
    struct timeval now;
    int err;

    gettimeofday(&now, NULL);

    // Connection attempts.
    if (timerisset(&core->next_connect_time) && timercmp(&now, &core->next_connect_time, >)) {
        err = api->net_connect();
        if (!err) {
            core->connect_retries = 0;
            timerclear(&core->next_connect_time);
        } else {
            core->connect_retries++;
            if (core->connect_retries > MAX_RECO_ATTEMPTS) {
                operlog("Maximum reconnection attempts reached, exiting");
                savealldb();
                unloadallmod();
                child_clean();
            }
            gettimeofday(&core->next_connect_time, NULL);
            core->next_connect_time.tv_sec += RECONNECT_DELAY;
            return;
        }
    }

    // Network I/O.
    err = api->net_poll();
    if (!err) {
        char *line;

        while ((line = api->read_line()) != NULL)
            parse_line(line);
    } else {
        api->net_disconnect(/*graceful=*/false);
        return;
    }

    // Periodic checks.
    CheckGuests();
    CheckLimits();
    CheckTimebans();

    gettimeofday(&now, NULL);

    if (timerisset(&core->next_expired_check_time) && timercmp(&now, &core->next_expired_check_time, >)) {
        checkexpired();
        gettimeofday(&core->next_expired_check_time, NULL);
        core->next_expired_check_time.tv_sec += 60;
    }

    if (timerisset(&core->next_savedb_time) && timercmp(&now, &core->next_savedb_time, >)) {
        savealldb();
        gettimeofday(&core->next_savedb_time, NULL);
        core->next_savedb_time.tv_sec += 60*core_get_config()->savedb_interval;
    }
}

static void child_run_loop(void)
{
    const int min_loop_delay_ms = 10;
    struct timeval before, after, res;
    int loop_delay_ms;

    while (!get_core()->stop) {
        gettimeofday(&before, NULL);
        child_run_loop_once();
        gettimeofday(&after, NULL);

        timersub(&before, &after, &res);
        loop_delay_ms = res.tv_sec*1000 + res.tv_usec/1000;

        // Pace run loops. Avoids using 100% CPU when we are trying to reconnect.
        // TODO: report loop delay metric as load indicator.
        if (loop_delay_ms < min_loop_delay_ms)
            usleep((min_loop_delay_ms - loop_delay_ms)*1000);
    }
}
int main(int argc, char **argv)
{
    struct sigaction sig, old, sigresethand;
    struct core_api *api = get_core_api();
    struct rlimit rlim;
    struct core *core;
    int err, timenow;
    int daemonize = 1;
    char op = 0;

    init_core();
    core = get_core();

    core->startuptime = time(NULL);

    memset(&sig,0,sizeof(sig));
    memset(&sigresethand,0,sizeof(sigresethand));
    sig.sa_handler = sighandler;
    sigresethand = sig;
    sigresethand.sa_flags = SA_RESETHAND;

    sigaction(SIGHUP, &sig, &old);
    sigaction(SIGUSR2,&sig,&old);
    sigaction(SIGINT,&sig,&old);
    sigaction(SIGCHLD,&sig,&old);
    sigaction(SIGPIPE,&sig,&old);
    sigaction(SIGSEGV, &sigresethand, &old);

    if ((getrlimit(RLIMIT_CORE, &rlim)) == -1) {
        perror("getrlimit");
        return -1;
    }
    rlim.rlim_cur = RLIM_INFINITY;
    if ((setrlimit(RLIMIT_CORE, &rlim)) == -1) {
        perror("setrlimit");
        return -1;
    }

    /* Setting default values */

    strcpy(core_get_config()->nick,"C");
    strcpy(core_get_config()->name,"services.geeknode.org");
    strcpy(core_get_config()->ident,"cserve");
    strcpy(core_get_config()->host,"geeknode.org");
    strcpy(core_get_config()->linkpass,"f00p4ss");
    strcpy(core_get_config()->mysql_host,"localhost");
    strcpy(core_get_config()->mysql_db,"child");
    strcpy(core_get_config()->mysql_login,"child");
    strcpy(core_get_config()->mysql_passwd,"childp4ss");
    strcpy(core_get_config()->logfile,"child.log");
    strcpy(core_get_config()->guest_prefix,"Geek");
    strcpy(core_get_config()->sendmail,"/usr/sbin/sendmail -t");
    strcpy(core_get_config()->sendfrom,"noreply@geeknode.org");
    strcpy(core_get_config()->usercloak,".users.geeknode.org");
    bzero(core_get_config()->bindip,32);
    core_get_config()->port = 4400;
    core_get_config()->maxclones = 5;
    core_get_config()->nick_expire = 45;
    core_get_config()->chan_expire = 60;
    core_get_config()->chanperuser = 10;
    core_get_config()->level_oper = 100;
    core_get_config()->level_admin = 500;
    core_get_config()->level_root = 900;
    core_get_config()->level_owner = 1000;
    core_get_config()->limittime = 5;
    core_get_config()->savedb_interval = 60;
    core_get_config()->anonymous_global = 0;
    core_get_config()->maxmsgtime = 2;
    core_get_config()->maxmsgnb = 5;
    core_get_config()->ignoretime = 60;
    core_get_config()->maxloginatt = 3;
    core_get_config()->chlev_sadmin = 20;
    core_get_config()->chlev_admin = 10;
    core_get_config()->chlev_op = 5;
    core_get_config()->chlev_halfop = 4;
    core_get_config()->chlev_voice = 3;
    core_get_config()->chlev_invite = 1;
    core_get_config()->chlev_nostatus = -1;
    core_get_config()->chlev_akick = -2;
    core_get_config()->chlev_akb = -3;
    core_get_config()->emailreg = 0;

    bzero(core->remote_server, SERVERNAMELEN+1);
    bzero(core->remote_sid, SIDLEN+1);

    /* -- */

    while ((op = getopt(argc,argv,"dhv")) != EOF) {
        switch(op) {
            case 'd':
                daemonize = 0;
                break;
            case 'v':
                if (core->verbose)
                    core->vv = true;
                else
                    core->verbose = true;
                break;
            case 'h':
            default:
                usage(argv[0]);
        }
    }

    loadconf(0);

    if (!connect_to_db()) {
        fprintf(stderr,"Cannot connect to mysql: %s\n", mysql_error(&get_core()->mysql_handle));
        operlog("Cannot connect to mysql db");
        child_clean();
    }

    if (get_core()->verbose) printf("Connected to mysql DB\n");
    if (get_core_api()->load_all_db() < 0) {
        fprintf(stderr, "Failed to load database, exiting.\n");
        child_clean();
    }

    err = api->net_connect();
    if (err < 0) {
        fprintf(stderr,"Failed to connect to %s:%d\n",core_get_config()->server,core_get_config()->port);
        operlog("Failed to connect to %s:%d",core_get_config()->server,core_get_config()->port);
        child_clean();
    }
    if (get_core()->verbose) printf("Connected to server\n");

    if (daemonize) {
        if (daemon(1,0) < 0) {
            fprintf(stderr, "Failed to daemonize: %s\n", strerror(errno));
            abort();
        }
    }
    write_pid();

    child_run_loop();

    operlog("Returned from main run loop, gracefully exiting.");
    child_die(/*save=*/1);

    return 0;
}
