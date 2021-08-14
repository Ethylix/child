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


#ifndef _NET_H
#define _NET_H

#define CHUNKSIZE   65536
#define SENDQSIZE   65536 /* 64 KB should be really enough */

#define RECONNECT_DELAY 10
#define MAX_RECO_ATTEMPTS 10
#define CONNECT_TIMEOUT 5

struct net_indata {
    char *nextline, *currentline;
    char *chunkbufentry;
    char chunkbuf[CHUNKSIZE];
};

struct net_outdata {
    char outbuf[SENDQSIZE];
    int writebytes;
};

struct net {
    struct net_indata indata;
    struct net_outdata outdata;
    int fd;
};

int match_ipmask (int, int, int);
const char *decode_ip (char *);

#endif
