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


#ifndef _NET_H
#define _NET_H

#define CHUNKSIZE   65536
#define SENDQSIZE   65536 /* 64 KB should be really enough */

#define RECONNECT_DELAY 60
#define MAX_RECO_ATTEMPTS 10
#define CONNECT_TIMEOUT 5

#define ECL_MAXSOCK 64

struct {
    char *nextline, *currentline;
    char *chunkbufentry;
    char chunkbuf[CHUNKSIZE];
} indata;

struct {
    char outbuf[SENDQSIZE];
    int writebytes;
} outdata;

int ConnectToServer (void);
void SendInitToServer (void);
void DisconnectFromServer (void);
void SendRaw (char *, ...);
int ReadChunk (void);
int GetLineFromChunk (void);
int GetLineFromPChunk (int);
int Bind (void);
void CloseAllSock (void);
int build_poll (void);
int match_ipmask (int, int, int);
const char *decode_ip (char *);
void flush_sendq(void);

#endif
