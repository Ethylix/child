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
            get_core()->eos = false;
            get_core()->retry_attempts = get_core()->connected = 0;
            get_core()->nextretry = time(NULL)+1;
            cleanup_reconnect();
            close(get_core()->sock);
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
    DisconnectFromServer();
    flush_sendq();
    CloseAllSock();
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
    DisconnectFromServer();
    flush_sendq();
    CloseAllSock();
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

int main(int argc, char **argv)
{
    InitMem();
    init_core();

    get_core()->startuptime = time(NULL);

    int retval,mysql_lastconn,lastcheck,lastcheck2,timenow;
    struct pollfd pfd;

    indata.nextline = indata.chunkbufentry = indata.chunkbuf;

    int daemonize = 1;
    char op = 0;
    get_core()->retry_attempts = get_core()->nextretry = get_core()->connected = 0;

    struct sigaction sig, old, sigresethand;
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

    struct rlimit rlim;
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

    bzero(get_core()->remote_server, SERVERNAMELEN+1);
    bzero(get_core()->remote_sid, SIDLEN+1);

    /* -- */

    while ((op = getopt(argc,argv,"dhv")) != EOF) {
        switch(op) {
            case 'd':
                daemonize = 0;
                break;
            case 'v':
                if (get_core()->verbose)
                    get_core()->vv = true;
                else
                    get_core()->verbose = true;
                break;
            case 'h':
            default:
                usage(argv[0]);
        }
    }

    loadconf(0);

    retval = ConnectToServer();
    switch(retval) {
        case -1:
            fprintf(stderr,"Failed to connect to %s:%d (connection timed out)\n",core_get_config()->server,core_get_config()->port);
            operlog("Failed to connect to %s:%d (connection timed out)",core_get_config()->server,core_get_config()->port);
            child_clean();
        case 0:
            fprintf(stderr,"Failed to connect to %s:%d\n",core_get_config()->server,core_get_config()->port);
            operlog("Failed to connect to %s:%d",core_get_config()->server,core_get_config()->port);
            child_clean();
    }

    if (get_core()->verbose) printf("Connected to server\n");

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
    if (get_core()->verbose) printf("Logging in to server\n");
    SendInitToServer();
    get_core()->connected = 1;
    if (get_core()->verbose) printf("Logged in to server\n");

    SendRaw("EOS");

    lastcheck = lastcheck2 = mysql_lastconn = time(NULL);
    if (daemonize) {
        if (daemon(1,0) < 0) {
            fprintf(stderr, "Failed to daemonize: %s\n", strerror(errno));
            abort();
        }
    }
    write_pid();

    for (;;) {
        if (!get_core()->connected)
            goto try_reconnect;

        pfd.fd = get_core()->sock;
        pfd.events = POLLIN | POLLPRI;
        if (outdata.writebytes > 0)
            pfd.events |= POLLOUT;
        pfd.revents = 0;

        retval = poll(&pfd, 1, 10);
        if (retval > 0) {
            if (pfd.revents & POLLOUT)
                flush_sendq();

            if (pfd.revents & (POLLIN | POLLPRI)) {
                if (!ReadChunk()) {
                    if (!get_core()->connected || !get_core()->eos)
                        continue;
                    operlog("Connection reset by peer");
                    savealldb();
                    get_core()->eos = false;
                    get_core()->retry_attempts = get_core()->connected = 0;
                    get_core()->nextretry = time(NULL)+1;
                    cleanup_reconnect();
                    close(get_core()->sock);
                    continue;
                }
                while (GetLineFromChunk())
                    ParseLine();
            }
        }

try_reconnect:
        timenow = time(NULL);

        if (!get_core()->connected && timenow - get_core()->nextretry >= 0) {
            retval = ConnectToServer();
            if (retval == -1 || retval == 0) {
                get_core()->retry_attempts++;
                operlog("Reconnecting attempt #%d failed (%s)",get_core()->retry_attempts,retval ? "timeout" : "error");
                if (get_core()->retry_attempts >= MAX_RECO_ATTEMPTS) {
                    operlog("Maximum reconnection attempts reached, exiting");
                    savealldb();
                    unloadallmod();
                    CloseAllSock();
                    child_clean();
                }
                get_core()->nextretry = timenow + RECONNECT_DELAY;
            } else {
                SendInitToServer();
                get_core()->connected = 1;
                get_core()->nextretry = 0;
                SendRaw("EOS");
                operlog("Reconnected");
            }
            if (get_core()->connected) continue;
        }

        if (timenow - mysql_lastconn >= 60*core_get_config()->savedb_interval) {
            savealldb();
            mysql_lastconn = timenow;
        }

        if (timenow - lastcheck >= 1) {
            CheckGuests();
            CheckLimits();
            CheckTimebans();
            lastcheck = timenow;
        }

        if (timenow - lastcheck2 >= 60) {
            checkexpired();
            lastcheck2 = timenow;
        }

        // Avoid burning the CPU while trying to reconnect.
        if (!get_core()->connected)
            usleep(10000);
    }

    operlog("Program shouldn't reach this piece of code, WTF ? Saving DBs and exiting.");
    savealldb();
    unloadallmod();
    CloseAllSock();
    child_clean();

    return 0;
}
