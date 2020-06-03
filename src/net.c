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


#include "net.h"

#include "child.h"
#include "filter.h"
#include "mem.h"
#include "partyline.h"
#include "string_utils.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

extern eclientlist eclient_list;

extern struct pollfd ufds[ECL_MAXSOCK];
extern int vv;
extern int sock, esock;
extern int eos;

static jmp_buf timeout_jump;

void timeout()
{
    longjmp(timeout_jump,1);
}

int build_poll()
{
    int i,j=0;
    Eclient *ecl;
    for (i=0;i<ECL_MAXSOCK;i++) {
        ufds[i].fd = 0;
        ufds[i].events = POLLIN | POLLPRI;
        ufds[i].revents = 0;
    }

    ufds[j++].fd = sock;
    if (me.listen_port) {
        ufds[j++].fd = esock;
        LIST_FOREACH_ALL(eclient_list, ecl)
            ufds[j++].fd = ecl->fd;
    }

    return j;
}

Eclient *find_eclient (int fd)
{
    Eclient *tmp;
    LIST_FOREACH(eclient_list, tmp, HASH_INT(fd)) {
        if (tmp->fd == fd)
            return tmp;
    }

    return NULL;
}

Eclient *find_eclient_name (char *nick)
{
    Eclient *tmp;
    LIST_FOREACH_ALL(eclient_list, tmp) {
        if (!Strcmp(tmp->nick,nick))
            return tmp;
    }

    return NULL;
}

Eclient *AddEclient (int fd, struct sockaddr_storage sa, socklen_t salen)
{
    Eclient *new_eclient;
    int n;
    new_eclient = (Eclient *)malloc(sizeof(Eclient));

    if ((n = getnameinfo((struct sockaddr *)&sa,salen,new_eclient->addr,NI_MAXHOST,new_eclient->port,NI_MAXSERV,NI_NUMERICHOST+NI_NUMERICSERV))) {
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(n));
        if (n == EAI_SYSTEM) perror("warning: getnameinfo");
        close(fd);
        return NULL;
    }

    if ((n = getnameinfo((struct sockaddr *)&sa,salen,new_eclient->host,NI_MAXHOST,NULL,0,0))) {
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(n));
        if (n == EAI_SYSTEM) perror("warning: getnameinfo");
        close(fd);
        return NULL;
    }

    bzero(new_eclient->nick,NICKLEN);
    new_eclient->authed = 0;
    new_eclient->fd = fd;
    new_eclient->pchunkbufentry = new_eclient->pchunkbuf;
    new_eclient->pnextline = new_eclient->pchunkbuf;

    LIST_INSERT_HEAD(eclient_list, new_eclient, HASH_INT(fd));
    return new_eclient;
}

void DeleteEclient (Eclient *eclient)
{
    LIST_REMOVE(eclient_list, eclient, HASH_INT(eclient->fd));
    free(eclient);
}

int Bind()
{
    int n;
    struct addrinfo hints;
    struct addrinfo *res = NULL;

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    char port[6];
    sprintf(port,"%d",me.listen_port);

    n = getaddrinfo(*me.bindip ? me.bindip : NULL,port,&hints,&res);
    if (n != 0) {
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(n));
        if (n == EAI_SYSTEM) perror("getaddrinfo");
        return 0;
    }

    esock = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if (esock < 0) { perror("socket"); return 0; }

    int yes = 1;
    setsockopt(esock,SOL_SOCKET,SO_REUSEADDR,(char *)&yes,sizeof(yes));

    if (bind(esock,res->ai_addr,res->ai_addrlen) == -1) {
        perror("bind");
        return 0;
    }

    freeaddrinfo(res);

    if (listen(esock,5) == -1)
        return 0;

    fcntl(esock,F_SETFL,O_NONBLOCK);
    return 1;
}

#ifdef USE_GNUTLS
static void close_secure_connection()
{
    gnutls_deinit(session);
    gnutls_certificate_free_credentials(xcred);
    gnutls_global_deinit();
}
#endif

int ConnectToServer()
{
    int n;
    struct addrinfo hints,hints2;
    struct addrinfo *res = NULL,*res2 = NULL;

#ifdef USE_GNUTLS
    if (me.ssl) {
        const int cert_type_priority[3] = { GNUTLS_CRT_X509, GNUTLS_CRT_OPENPGP, 0 };

        gnutls_global_init();
        gnutls_certificate_allocate_credentials(&xcred);
        gnutls_init(&session,GNUTLS_CLIENT);
        gnutls_set_default_priority(session);
        gnutls_certificate_type_set_priority(session, cert_type_priority);
        gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE,xcred);
    }
#endif

    hints.ai_flags = 0;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    char port[6];
    sprintf(port,"%d",me.port);

    n = getaddrinfo(me.server,port,&hints,&res);
    if (n != 0) {
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(n));
        if (n == EAI_SYSTEM) perror("getaddrinfo");
        return 0;
    }

    sock = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if (sock < 0) { perror("socket"); freeaddrinfo(res); return 0; }

    if (*me.bindip) {
        hints2.ai_flags = AI_PASSIVE;
        hints2.ai_family = PF_UNSPEC;
        hints2.ai_socktype = SOCK_STREAM;
        hints2.ai_protocol = IPPROTO_TCP;
        hints2.ai_addrlen = 0;
        hints2.ai_addr = NULL;
        hints2.ai_canonname = NULL;
        hints2.ai_next = NULL;

        n = getaddrinfo(me.bindip,NULL,&hints2,&res2);
        if (n != 0) {
            fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(n));
            if (n == EAI_SYSTEM) perror("getaddrinfo");
            return 0;
        }

        if (bind(sock,res2->ai_addr,res2->ai_addrlen) < 0) {
            perror("bind");
            operlog("bind() error: %s",strerror(errno));
#ifdef USE_GNUTLS
            if (me.ssl) close_secure_connection();
#endif
            freeaddrinfo(res);
            freeaddrinfo(res2);
            return 0;
        }
    }

    signal(SIGALRM,timeout);
    alarm(CONNECT_TIMEOUT);
    if (setjmp(timeout_jump) == 1) {
        close(sock);
        freeaddrinfo(res);
        if (*me.bindip) freeaddrinfo(res2);
        return -1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        alarm(0);
        signal(SIGALRM,SIG_DFL);
        operlog("connect() error: %s",strerror(errno));
#ifdef USE_GNUTLS
        if (me.ssl) close_secure_connection();
#endif
        freeaddrinfo(res);
        if (*me.bindip) freeaddrinfo(res2);
        close(sock);
        return 0;
    }

    alarm(0);
    signal(SIGALRM,SIG_DFL);
    freeaddrinfo(res);
    if (*me.bindip) freeaddrinfo(res2);

#ifdef USE_GNUTLS
    if (me.ssl) {
        gnutls_transport_set_ptr(session, (gnutls_transport_ptr_t)sock);
        int ret = gnutls_handshake(session);
        if (ret < 0) {
            fprintf(stderr,"Handshake failed\n");
            gnutls_perror(ret);
            operlog("Handshake failed for secure connection");
            close_secure_connection();
            return 0;
        }
    }
#endif
    fcntl(sock,F_SETFL,O_NONBLOCK);

    return 1;
}

void SendInitToServer()
{
    SendRaw("PROTOCTL NICKv2 VHP NICKIP ESVID");
    SendRaw("PASS :%s",me.linkpass);
    SendRaw("PROTOCTL EAUTH=%s SID=%s",me.name,me.sid);
    SendRaw("SERVER %s 1 :Child IRC Services",me.name);
    SendRaw("SQLINE %s :Reserved for services",me.nick);
    fakeuser(me.nick,me.ident,me.host,MY_UMODES);
}

void DisconnectFromServer ()
{
    SendRaw("SQUIT");
    operlog("Disconnect from server (SQUIT)");
    eos = 0;
}

void flush_sendq()
{
    size_t bytes = 0, len;

    if (outdata.writebytes == 0)
        return;

    len = outdata.writebytes;

#ifdef USE_GNUTLS
    if (me.ssl)
        bytes = gnutls_record_send(session, outdata.outbuf, len);
    else
#endif
    bytes = send(sock, outdata.outbuf, len, 0);

    memmove(outdata.outbuf, outdata.outbuf + bytes, len - bytes);
    outdata.outbuf[len-bytes] = 0;
    outdata.writebytes = len - bytes;
}

void SendRaw (char *msg, ...)
{
    char tmp[1024];
    char buf[1024];
    va_list val;
    int len;
    char *outptr;
    ircsprintf(buf,1023,msg,val);

    if (vv) printf(">> %s\n",buf);
    snprintf(tmp,1024,"%s\r\n",buf);

    len = strlen(tmp);
    if (len > SENDQSIZE) {
        fprintf(stderr, "PANIC: cannot enqueue data: sendq size exceeded (%d > %d)\n", len, SENDQSIZE);
        operlog("PANIC: sendq size exceeded (%d > %d)", len, SENDQSIZE);
#ifdef DIE_ON_PANIC
        child_die(1);
#else
        return;
#endif
    }

    if (len + outdata.writebytes >= SENDQSIZE) {
        fprintf(stderr, "PANIC: cannot enqueue data (%d bytes): sendq size exceeded (%d > %d)\n", len, len + outdata.writebytes, SENDQSIZE);
        operlog("PANIC: sendq size exceeded (%d > %d, tried to write %d bytes)", len + outdata.writebytes, SENDQSIZE, len);
#ifdef DIE_ON_PANIC
        child_die(1);
#else
        return;
#endif
    }

#ifdef USE_FILTER
    if ((filter_check(buf, DIRECT_OUT)) == RULE_DROP)
        return;
#endif

    outptr = outdata.outbuf + outdata.writebytes;

    strncpy(outptr, tmp, SENDQSIZE - outdata.writebytes - 1);
    outdata.writebytes += len;
}

void send_to (Eclient *eclient, char *msg, ...)
{
    char tmp[1024];
    char buf[1024];
    va_list val;
    ircsprintf(buf,1024,msg,val);

    snprintf(tmp,1024,"%s\r\n",buf);

    send(eclient->fd,tmp,strlen(tmp),0);
}

void sendto_all (char *msg, ...)
{
    char buf[1024];
    va_list val;
    ircsprintf(buf,1024,msg,val);

    Eclient *eclient;
    LIST_FOREACH_ALL(eclient_list, eclient) {
        if (eclient->authed == 1)
            send_to(eclient,buf);
    }
}

void sendto_all_butone (Eclient *ecl, char *msg, ...)
{
    char buf[1024];
    va_list val;
    ircsprintf(buf,1024,msg,val);

    Eclient *eclient;
    LIST_FOREACH_ALL(eclient_list, eclient) {
        if (eclient->fd != ecl->fd && eclient->authed == 1)
            send_to(eclient,buf);
    }
    pllog(buf);
}

int ReadChunk(void)
{
    int oldbytes;
    int readbytes;
    char *dest;

    if (*indata.nextline != '\0') {
        oldbytes = strlen(indata.nextline);
        dest = indata.chunkbuf;
        while (*indata.nextline)
            *dest++ = *indata.nextline++;
        indata.chunkbufentry = indata.chunkbuf + oldbytes;
    } else {
        oldbytes = 0;
        indata.chunkbufentry = indata.chunkbuf;
    }

#ifdef USE_GNUTLS
    if (me.ssl)
        readbytes = gnutls_record_recv(session, indata.chunkbufentry, (CHUNKSIZE - oldbytes) - 10);
    else
#endif
    readbytes = read(sock, indata.chunkbufentry, (CHUNKSIZE - oldbytes) - 10);
    if (readbytes == 0 || readbytes == -1)
        return 0;

    *(indata.chunkbufentry + readbytes) = '\0';
    indata.nextline = indata.chunkbuf;

    return 1;
}

int GetLineFromChunk(void)
{
    indata.currentline = indata.nextline;
    while (*indata.nextline != '\n' && *indata.nextline != '\r' && *indata.nextline != '\0')
        indata.nextline++;

    if (*indata.nextline == '\0') {
        indata.nextline = indata.currentline;
        return 0;
    }

    if (*indata.nextline == '\n' || *indata.nextline == '\r') {
        *indata.nextline = '\0';
        indata.nextline++;
    }

    if (*indata.nextline == '\n' || *indata.nextline == '\r') {
        *indata.nextline = '\0';
        indata.nextline++;
    }

    return 1;
}

int ReadLine (char *cur, char *next)
{
    cur = next;
    while (*next != '\n' && *next != '\r' && *next != '\0')
        next++;

    if (*next == '\0') {
        next = cur;
        return 0;
    }

    if (*next == '\n' || *next == '\r') {
        *next = '\0';
        next++;
    }

    if (*next == '\n' || *next == '\r') {
        *next = '\0';
        next++;
    }

    return 1;
}

void CloseAllSock()
{
    close(sock);
#ifdef USE_GNUTLS
    if (me.ssl)
        close_secure_connection();
#endif

    if (me.listen_port) {
        Eclient *eclient;

        while (!LIST_EMPTY(eclient_list)) {
            eclient = LIST_HEAD(eclient_list);
            close(eclient->fd);
            DeleteEclient(eclient);
        }

        close(esock);
        pllog("Partyline closed");
    }
}

int match_ipmask (int addr, int mask, int bits)
{
    if ((ntohl(addr) & ~((1 << (32 - bits)) - 1)) == ntohl(mask))
        return 1;
    else
        return 0;
}
