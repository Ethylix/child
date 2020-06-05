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
#include "db.h"
#include "filter.h"
#include "modules.h"
#include "string_utils.h"
#include "trust.h"
#include "user.h"

#include <errno.h>
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

int sock;
int startuptime;
int verbose, vv, raws, eos;
MYSQL mysql;

memberlist member_list;
limitlist limit_list;
chanbotlist chanbot_list;
commandlist command_list;
#ifdef USE_FILTER
rulelist rule_list;
#endif
tblist tb_list;

#ifdef USE_GNUTLS
gnutls_session_t session;
gnutls_certificate_credentials_t xcred;
#endif

static void sighandler (int signal)
{
    int status;

    switch (signal) {
        case SIGHUP:
            loadconf(1);
            break;
#ifdef USE_FILTER
        case SIGUSR1:
            loadrulefile();
            break;
#endif
        case SIGUSR2:
            operlog("Got SIGUSR2, reconnecting to server");
            eos = me.retry_attempts = me.connected = 0;
            me.nextretry = time(NULL)+1;
            cleanup_reconnect();
            close(sock);
            break;
        case SIGINT:
            operlog("SIGINT signal received, exiting");
            fakekill(me.nick,"Got SIGINT, exiting");
            child_die(1);
            break;
        case SIGCHLD:
            waitpid(-1,&status,WNOHANG);
            break;
        case SIGPIPE:
            operlog("Got SIGPIPE, ignored");
            break;
    }
}

static void usage (char *progname)
{
    fprintf(stderr,"Usage: %s [-dv]\n\t-d\tDo not daemonize\n\t-v\tBe verbose (use twice for more verbose)\n",progname);
    exit(-1);
}

void mylog (char *file, char *msg, ...)
{
    char tmp[1024];
    char buf[1024];
    va_list val;
    FILE *index;
    time_t tm;
    ircsprintf(buf,1023,msg,val);
    
    tm = time(NULL);
    
    snprintf(tmp,1023,"[ %s] %s\n",ctime(&tm),buf);
    
    /* The ctime() function returns the result with a trailing '\n' */
    
    *(strstr(tmp,"\n")) = ' ';
    index = fopen(file,"a+");
    if (!index) return;
    fputs(tmp,index);
    fclose(index);
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
    FreeAllMem();
    free_core();
    exit(0);
}

void child_die(int save)
{
    if (save) savealldb();
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

void init_srandom(void)
{
#ifdef HAVE_SRANDOMDEV
    srandomdev();
#else
    /* this piece of code comes from srandomdev() source */
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srandom(getpid() ^ tv.tv_sec ^ tv.tv_usec);
#endif
}

int main(int argc, char **argv)
{
    startuptime = time(NULL);

    InitMem();
    init_core();

    int retval,mysql_lastconn,lastcheck,lastcheck2,timenow;
    struct pollfd pfd;

    indata.nextline = indata.chunkbufentry = indata.chunkbuf;

    eos = raws = verbose = vv = 0;
    int daemonize = 1;
    char op = 0;
    me.retry_attempts = me.nextretry = me.connected = 0;

    struct sigaction sig, old;
    memset(&sig,0,sizeof(sig));
    sig.sa_handler = sighandler;
    sigaction(SIGHUP,&sig,&old);
#ifdef USE_FILTER
    sigaction(SIGUSR1,&sig,&old);
#endif
    sigaction(SIGUSR2,&sig,&old);
    sigaction(SIGINT,&sig,&old);
    sigaction(SIGCHLD,&sig,&old);
    sigaction(SIGPIPE,&sig,&old);

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

    strcpy(me.nick,"C");
    strcpy(me.name,"services.geeknode.org");
    strcpy(me.ident,"cserve");
    strcpy(me.host,"geeknode.org");
    strcpy(me.linkpass,"f00p4ss");
    strcpy(me.mysql_host,"localhost");
    strcpy(me.mysql_db,"child");
    strcpy(me.mysql_login,"child");
    strcpy(me.mysql_passwd,"childp4ss");
    strcpy(me.logfile,"child.log");
    strcpy(me.guest_prefix,"Geek");
    strcpy(me.sendmail,"/usr/sbin/sendmail -t");
    strcpy(me.sendfrom,"noreply@geeknode.org");
    strcpy(me.usercloak,".users.geeknode.org");
    bzero(me.bindip,32);
    me.port = 4400;
    me.maxclones = 5;
    me.nick_expire = 45;
    me.chan_expire = 60;
    me.chanperuser = 10;
    me.level_oper = 100;
    me.level_admin = 500;
    me.level_root = 900;
    me.level_owner = 1000;
    me.limittime = 5;
    me.savedb_interval = 60;
#ifdef USE_GNUTLS
    me.ssl = 0;
#endif
    me.anonymous_global = 0;
    me.maxmsgtime = 2;
    me.maxmsgnb = 5;
    me.ignoretime = 60;
    me.maxloginatt = 3;
    me.chlev_sadmin = 20;
    me.chlev_admin = 10;
    me.chlev_op = 5;
    me.chlev_halfop = 4;
    me.chlev_voice = 3;
    me.chlev_invite = 1;
    me.chlev_nostatus = -1;
    me.chlev_akick = -2;
    me.chlev_akb = -3;
#ifdef USE_FILTER
    me.filter = 0;
#endif
    me.emailreg = 0;

    /* -- */

    while ((op = getopt(argc,argv,"dhv")) != EOF) {
        switch(op) {
            case 'd':
                daemonize = 0;
                break;
            case 'v':
                if (verbose)
                    vv = 1;
                else
                    verbose = 1;
                break;
            case 'h':
            default:
                usage(argv[0]);
        }
    }

    loadconf(0);
#ifdef USE_FILTER
    if (me.filter)
        loadrulefile();
#endif

    retval = ConnectToServer();
    switch(retval) {
        case -1:
            fprintf(stderr,"Failed to connect to %s:%d (connection timed out)\n",me.server,me.port);
            operlog("Failed to connect to %s:%d (connection timed out)",me.server,me.port);
            child_clean();
        case 0:
            fprintf(stderr,"Failed to connect to %s:%d\n",me.server,me.port);
            operlog("Failed to connect to %s:%d",me.server,me.port);
            child_clean();
    }

    if (verbose) printf("Connected to server\n");

    if (!connect_to_db()) {
        fprintf(stderr,"Cannot connect to mysql\n");
        operlog("Cannot connect to mysql db");
        child_clean();
    }

    if (verbose) printf("Connected to mysql DB\n");
    loadalldb();
    mysql_close(&mysql);
    if (verbose) printf("Logging in to server\n");
    SendInitToServer();
    me.connected = 1;
    if (verbose) printf("Logged in to server\n");

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
        if (!me.connected)
            goto try_reconnect;

        pfd.fd = sock;
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
                    if (!me.connected || !eos)
                        continue;
                    operlog("Connection reset by peer");
                    savealldb();
                    eos = me.retry_attempts = me.connected = 0;
                    me.nextretry = time(NULL)+1;
                    cleanup_reconnect();
                    close(sock);
                    continue;
                }
                while (GetLineFromChunk())
                    ParseLine();
            }
        }

try_reconnect:
        timenow = time(NULL);

        if (!me.connected && timenow - me.nextretry >= 0) {
            retval = ConnectToServer();
            if (retval == -1 || retval == 0) {
                me.retry_attempts++;
                operlog("Reconnecting attempt #%d failed (%s)",me.retry_attempts,retval ? "timeout" : "error");
                if (me.retry_attempts >= MAX_RECO_ATTEMPTS) {
                    operlog("Maximum reconnection attempts reached, exiting");
                    savealldb();
                    unloadallmod();
                    CloseAllSock();
                    child_clean();
                }
                me.nextretry = timenow + RECONNECT_DELAY;
            } else {
                SendInitToServer();
                me.connected = 1;
                me.nextretry = 0;
                SendRaw("EOS");
                operlog("Reconnected");
            }
            if (me.connected) continue;
        }

        if (timenow - mysql_lastconn >= 60*me.savedb_interval) {
            savealldb();
            mysql_lastconn = timenow;
        }

        if (timenow - lastcheck >= 1) {
            CheckGuests();
            CheckLimits();
            CheckTB();
            lastcheck = timenow;
        }

        if (timenow - lastcheck2 >= 60) {
            checkexpired();
            lastcheck2 = timenow;
        }

        // Avoid burning the CPU while trying to reconnect.
        if (!me.connected)
            usleep(10000);
    }

    operlog("Program shouldn't reach this piece of code, WTF ? Saving DBs and exiting.");
    savealldb();
    unloadallmod();
    CloseAllSock();
    child_clean();

    return 0;
}
