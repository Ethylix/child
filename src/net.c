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
#include "core.h"
#include "core_api.h"
#include "logging.h"
#include "mem.h"
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

static jmp_buf timeout_jump;

static void timeout()
{
    longjmp(timeout_jump,1);
}

static void net_init(void)
{
    struct net *net = core_get_net();
    struct net_outdata *outdata = &net->outdata;
    struct net_indata *indata = &net->indata;

    outdata->outbuf[0] = 0;
    outdata->writebytes = 0;

    indata->chunkbuf[0] = 0;
    indata->nextline = indata->chunkbuf;
    indata->currentline = indata->chunkbuf;
    indata->chunkbufentry = indata->chunkbuf;

    net->fd = -1;
}

static int net_connect_fd(void)
{
    int n, sock;
    struct addrinfo hints,hints2;
    struct addrinfo *res = NULL,*res2 = NULL;

    hints.ai_flags = 0;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    char port[6];
    sprintf(port,"%d",core_get_config()->port);

    n = getaddrinfo(core_get_config()->server,port,&hints,&res);
    if (n != 0) {
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(n));
        if (n == EAI_SYSTEM) perror("getaddrinfo");
        return -1;
    }

    sock = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if (sock < 0) { perror("socket"); freeaddrinfo(res); return 0; }

    if (*core_get_config()->bindip) {
        hints2.ai_flags = AI_PASSIVE;
        hints2.ai_family = PF_UNSPEC;
        hints2.ai_socktype = SOCK_STREAM;
        hints2.ai_protocol = IPPROTO_TCP;
        hints2.ai_addrlen = 0;
        hints2.ai_addr = NULL;
        hints2.ai_canonname = NULL;
        hints2.ai_next = NULL;

        n = getaddrinfo(core_get_config()->bindip,NULL,&hints2,&res2);
        if (n != 0) {
            fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(n));
            if (n == EAI_SYSTEM) perror("getaddrinfo");
            return -1;
        }

        if (bind(sock,res2->ai_addr,res2->ai_addrlen) < 0) {
            perror("bind");
            operlog("bind() error: %s",strerror(errno));
            freeaddrinfo(res);
            freeaddrinfo(res2);
            return -1;
        }
    }

    signal(SIGALRM,timeout);
    alarm(CONNECT_TIMEOUT);
    if (setjmp(timeout_jump) == 1) {
        close(sock);
        freeaddrinfo(res);
        if (*core_get_config()->bindip) freeaddrinfo(res2);
        return -1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        alarm(0);
        signal(SIGALRM,SIG_DFL);
        operlog("connect() error: %s",strerror(errno));
        freeaddrinfo(res);
        if (*core_get_config()->bindip) freeaddrinfo(res2);
        close(sock);
        return -1;
    }

    alarm(0);
    signal(SIGALRM,SIG_DFL);
    freeaddrinfo(res);
    if (*core_get_config()->bindip) freeaddrinfo(res2);

    fcntl(sock,F_SETFL,O_NONBLOCK);

    core_get_net()->fd = sock;

    return 0;
}

static void send_raw(const char *msg, ...)
{
    struct net *net = core_get_net();
    struct net_outdata *outdata = &net->outdata;
    char tmp[1024];
    char buf[1024];
    va_list val;
    int len;
    char *outptr;

    ircsprintf(buf,1023,msg,val);

    if (get_core()->vv) printf(">> %s\n",buf);
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

    if (len + outdata->writebytes >= SENDQSIZE) {
        fprintf(stderr, "PANIC: cannot enqueue data (%d bytes): sendq size exceeded (%d > %d)\n", len, len + outdata->writebytes, SENDQSIZE);
        operlog("PANIC: sendq size exceeded (%d > %d, tried to write %d bytes)", len + outdata->writebytes, SENDQSIZE, len);
#ifdef DIE_ON_PANIC
        child_die(1);
#else
        return;
#endif
    }

    outptr = outdata->outbuf + outdata->writebytes;

    strncpy(outptr, tmp, SENDQSIZE - outdata->writebytes - 1);
    outdata->writebytes += len;
}

static void send_handshake(void)
{
    send_raw("PASS :%s",core_get_config()->linkpass);
    send_raw("PROTOCTL EAUTH=%s SID=%s", core_get_config()->name, core_get_config()->sid);
    send_raw("PROTOCTL NOQUIT NICKv2 SJOIN SJ3 CLK TKLEXT TKLEXT2 NICKIP ESVID MLOCK EXTSWHOIS");
    send_raw("SERVER %s 1 :Child IRC Services",core_get_config()->name);
    send_raw("SQLINE %s :Reserved for services",core_get_config()->nick);
    generate_uid(get_core()->uid);
    fakeuser(core_get_config()->nick, core_get_config()->ident, core_get_config()->host, get_core()->uid, MY_UMODES);
    send_raw("EOS");
}

static int net_connect(void)
{
    int ret;

    ret = net_connect_fd();
    if (ret < 0)
        return ret;

    send_handshake();
    get_core()->connected = 1;

    return 0;
}

static int flush_sendq(void)
{
    struct net *net = core_get_net();
    struct net_outdata *outdata = &net->outdata;
    ssize_t bytes, len;

    if (outdata->writebytes == 0)
        return 0;

    len = outdata->writebytes;

    bytes = send(net->fd, outdata->outbuf, len, 0);
    if (bytes < 0)
        return -1;

    memmove(outdata->outbuf, outdata->outbuf + bytes, len - bytes);
    outdata->outbuf[len-bytes] = 0;
    outdata->writebytes = len - bytes;

    return 0;
}

static void net_disconnect(bool graceful)
{
    struct net *net = core_get_net();

    operlog("Disconnect from server (SQUIT)");

    if (graceful) {
        send_raw("SQUIT");
        flush_sendq();
    }

    close(net->fd);
    get_core()->eos = false;
    get_core()->connected = 0;

    cleanup_reconnect();
    net_init();
}

static int read_chunk(void)
{
    struct net *net = core_get_net();
    struct net_indata *indata = &net->indata;
    int oldbytes;
    int readbytes;
    char *dest;

    if (*indata->nextline != '\0') {
        oldbytes = strlen(indata->nextline);
        dest = indata->chunkbuf;
        while (*indata->nextline)
            *dest++ = *indata->nextline++;
        indata->chunkbufentry = indata->chunkbuf + oldbytes;
    } else {
        oldbytes = 0;
        indata->chunkbufentry = indata->chunkbuf;
    }

    readbytes = read(net->fd, indata->chunkbufentry, (CHUNKSIZE - oldbytes) - 10);
    if (readbytes <= -1)
        return -1;

    *(indata->chunkbufentry + readbytes) = '\0';
    indata->nextline = indata->chunkbuf;

    return 0;
}

static char *read_line(void)
{
    struct net *net = core_get_net();
    struct net_indata *indata = &net->indata;

    indata->currentline = indata->nextline;
    while (*indata->nextline != '\n' && *indata->nextline != '\r' && *indata->nextline != '\0')
        indata->nextline++;

    if (*indata->nextline == '\0') {
        indata->nextline = indata->currentline;
        return NULL;
    }

    if (*indata->nextline == '\n' || *indata->nextline == '\r') {
        *indata->nextline = '\0';
        indata->nextline++;
    }

    if (*indata->nextline == '\n' || *indata->nextline == '\r') {
        *indata->nextline = '\0';
        indata->nextline++;
    }

    return indata->currentline;
}

static int net_poll(void)
{
    struct net *net = core_get_net();
    struct pollfd pfd;
    int ret;

    if (!get_core()->connected)
        return 0;

    pfd.fd = net->fd;
    pfd.events = POLLIN | POLLPRI;
    if (net->outdata.writebytes > 0)
        pfd.events |= POLLOUT;
    pfd.revents = 0;

    ret = poll(&pfd, 1, 10);
    if (ret <= 0)
        return ret;

    if (pfd.revents & POLLOUT) {
        ret = flush_sendq();
        if (ret < 0)
            return ret;
    }

    if (pfd.revents & (POLLIN | POLLPRI)) {
        ret = read_chunk();
        if (ret < 0)
            return ret;
    }

    return 0;
}

int match_ipmask (int addr, int mask, int bits)
{
    if ((ntohl(addr) & ~((1 << (32 - bits)) - 1)) == ntohl(mask))
        return 1;
    else
        return 0;
}

static struct core_api net_api = {
    .net_init       = net_init,
    .net_connect    = net_connect,
    .net_disconnect = net_disconnect,
    .net_poll       = net_poll,
    .read_line      = read_line,
    .send_raw       = send_raw,
};
REGISTER_CORE_API(&net_api);
