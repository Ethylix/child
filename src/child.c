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

int sock,esock;
int startuptime;
int verbose, vv, raws, eos;
int emerg, emerg_req;
MYSQL mysql, mysql2;

userlist user_list;
nicklist nick_list;
cloneslist clones_list;
modulelist module_list;
hooklist hook_list;
trustlist trust_list;
linklist link_list;
eclientlist eclient_list;
guestlist guest_list;
chanlist chan_list;
wchanlist wchan_list;
cflaglist cflag_list;
memberlist member_list;
limitlist limit_list;
botlist bot_list;
chanbotlist chanbot_list;
commandlist command_list;
#ifdef USE_FILTER
rulelist rule_list;
#endif
tblist tb_list;
fakelist fake_list;

struct pollfd ufds[ECL_MAXSOCK];

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
    fprintf(stderr,"Usage: %s [-acdv]\n\t-a\tConvert anope db to child db\n\t-c\tCreate databases\n\t-d\tDo not daemonize\n\t-v\tBe verbose (use twice for more verbose)\n",progname);
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
    getcwd(buf,1024);
    snprintf(dir,1024,"%s/child",buf);
    system(dir);
    child_clean();
}

void init_srandom(void)
{
#ifdef HAVE_SRANDOMDEV
    srandomdev();
#else
    /* this piece of code comes from srandomdev() source */
    struct timeval tv;
    unsigned long junk; /* XXX left uninitialized on purpose */

    gettimeofday(&tv, NULL);
    srandom(getpid() ^ tv.tv_sec ^ tv.tv_usec ^ junk);
#endif
}

int main(int argc, char **argv)
{
    startuptime = time(NULL);

    InitMem();

    int retval,mysql_lastconn,newfd,i,lastcheck,lastcheck2,nbfd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    Eclient *eclient;

    indata.nextline = indata.chunkbufentry = indata.chunkbuf;

    emerg = emerg_req = 0;

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
    strcpy(me.pl_logfile,"partyline.log");
    strcpy(me.sendmail,"/usr/sbin/sendmail -t");
    strcpy(me.sendfrom,"noreply@geeknode.org");
    strcpy(me.usercloak,".users.geeknode.org");
    bzero(me.mysql_anope_host,32);
    bzero(me.mysql_anope_db,32);
    bzero(me.mysql_anope_login,32);
    bzero(me.mysql_anope_passwd,32);
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
    me.listen_port = 0;
#ifdef USE_GNUTLS
    me.ssl = 0;
#endif
    me.enable_exec = 0;
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
    me.anopemd5 = 0;
#ifdef USE_FILTER
    me.filter = 0;
#endif
    me.emailreg = 0;

    /* -- */

    int ladb=0,cdb=0;

    while ((op = getopt(argc,argv,"acdhv")) != EOF) {
        switch(op) {
            case 'a':
                ladb = 1;
                break;
            case 'c':
                cdb = 1;
                break;
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

    if (ladb) {
        if (!connect_to_db()) {
            printf("Cannot connect to db\n");
            child_clean();
        }

        loadalldb();
        mysql_close(&mysql);

        if (!connect_to_anope_db()) {
            printf("Cannot connect to anope db\n");
            child_clean();
        }

        printf("Loading anope database... ");
        fflush(stdout);

        loadanopedb();
        mysql_close(&mysql2);
        printf("done.\n");
        printf("Saving databases... ");
        fflush(stdout);
        savealldb();
        printf("done.\n");
        printf("Anope database converted\n");
        child_clean();
    }

    if (cdb) {
        if (!connect_to_db()) {
            printf("Cannot connect to db\n");
            child_clean();
        }

        printf("Creating databases ... ");
        fflush(stdout);
        char tmp[512];
        sprintf(tmp,"CREATE TABLE IF NOT EXISTS `child_chans` (chname varchar(50) not null, founder varchar(50) not null, entrymsg blob not null, options int not null, mlock varchar(32) not null, autolimit int not null, lastseen int not null, regtime int not null, topic blob not null)");
        mysql_query(&mysql,tmp);

        sprintf(tmp,"CREATE TABLE IF NOT EXISTS `child_chan_access` (chname varchar(50) not null, username varchar(255) not null, level int not null, automode int not null default 1, suspended int not null, uflags int not null)");
        mysql_query(&mysql,tmp);

        sprintf(tmp,"CREATE TABLE IF NOT EXISTS `child_trusts` (hostname varchar(255) not null, clones int not null)");
        mysql_query(&mysql,tmp);

        sprintf(tmp,"CREATE TABLE IF NOT EXISTS `child_users` (username varchar(50) not null, authlevel int not null, seen int not null, vhost varchar(200) not null, md5_pass varchar(32) not null, options int not null, timeout int not null, email varchar(100) not null, regtime int not null)");
        mysql_query(&mysql,tmp);

        sprintf(tmp,"CREATE TABLE IF NOT EXISTS `child_links` (master varchar(50) not null, slave varchar(50) not null)");
        mysql_query(&mysql,tmp);

        sprintf(tmp,"CREATE TABLE IF NOT EXISTS `child_botserv_bots` (name varchar(50) not null, ident varchar(50) not null, host varchar(50) not null)");
        mysql_query(&mysql,tmp);

        sprintf(tmp,"CREATE TABLE IF NOT EXISTS `child_botserv_chans` (chan varchar(50) not null, bot varchar(50) not null)");
        mysql_query(&mysql,tmp);

        RunHooks(HOOK_CREATEDB,NULL,NULL,NULL,NULL);

        printf(" done.\n");
        mysql_close(&mysql);
        child_clean();
    }

    if (me.listen_port) {
        if (!Bind()) {
            fprintf(stderr,"Error while binding\n");
            child_clean();
        }
        pllog("Partyline created");
    }

    retval = ConnectToServer();
    switch(retval) {
        case -1:
            fprintf(stderr,"Failed to connect to %s:%d (connection timed out)\n",me.server,me.port);
            operlog("Failed to connect to %s:%d (connection timed out)",me.server,me.port);
            child_clean();
        case 0:
            fprintf(stderr,"Failed to connect to %s:%d",me.server,me.port);
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
    if (daemonize) daemon(1,0);
    write_pid();

    nbfd = build_poll();
    struct pollfd pfdout;
    pfdout.fd = sock;
    pfdout.events = POLLOUT;
    pfdout.revents = 0;

    while(1) {
        if (outdata.writebytes > 0) {
            retval = poll(&pfdout,1,1000);
            if (retval > 0 && (pfdout.revents & POLLOUT))
                flush_sendq();
        }
        retval = poll(ufds,nbfd,1000);
        if (retval > 0) {
            for (i=0;i<nbfd;i++) {
                if (ufds[i].revents & (POLLIN | POLLPRI)) {
                    switch(i) {
                        case 0:
                            if (!ReadChunk()) {
                                if (!me.connected || !eos) break;
                                operlog("Connection reset by peer");
                                savealldb();
                                eos = me.retry_attempts = me.connected = 0;
                                me.nextretry = time(NULL)+1;
                                cleanup_reconnect();
                                close(sock);
                                break;
                            }
                            while (GetLineFromChunk())
                                ParseLine();
                            break;
                        case 1:
                            if (!me.listen_port) break;
                            newfd = accept(esock,(struct sockaddr *)&sa,&salen);
                            if (eclient_list.size+1 >= ECL_MAXSOCK) {
                                close(newfd);
                                break;
                            }
                            fcntl(newfd,F_SETFL,O_NONBLOCK);
                            eclient = AddEclient(newfd,sa,salen);
                            sendto_all_butone(eclient,"*** Connection from %s (%s)",eclient->host,eclient->addr);
                            nbfd = build_poll(); i++;
                            break;
                        default:
                            eclient = find_eclient(ufds[i].fd);
                            if (!eclient) break;
                            int oldnbfd = nbfd;
                            int old_eclient_size = eclient_list.size;
                            if (!ReadPChunk(eclient)) {
                                if (eclient->authed == 1)
                                    sendto_all_butone(eclient,"*** User %s (%s) logged out (Connection reset by peer)",eclient->nick,eclient->host);
                                else
                                    sendto_all_butone(eclient,"*** Lost connection from %s",eclient->host);
                                close(eclient->fd);
                                DeleteEclient(eclient);
                                nbfd = build_poll();
                                i--;
                                break;
                            }
                            int tmpfd = eclient->fd;
                            while (GetLineFromPChunk(tmpfd))
                                ParseEclient(eclient);
                            if (old_eclient_size > eclient_list.size)
                                nbfd = build_poll();
                            if (nbfd < oldnbfd) i -= (oldnbfd - nbfd);
                            break;
                    }
                }
            }
        }

        int timenow = time(NULL);

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
    }

    operlog("Program shouldn't reach this piece of code, WTF ? Saving DBs and exiting.");
    savealldb();
    unloadallmod();
    CloseAllSock();
    child_clean();

    return 0;
}
