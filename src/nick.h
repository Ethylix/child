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


#ifndef _NICK_H
#define _NICK_H

#include "llist.h"

#define NICKLEN 35
#define HOSTLEN 60
#define UIDLEN 9
#define SVIDLEN 35

#define BOTSERV_UMODES "oSq"
#define MY_UMODES "oSq"

#define UMODE_o 0x0001
#define UMODE_a 0x0002
#define UMODE_A 0x0004
#define UMODE_r 0x0008
#define UMODE_B 0x0010
#define UMODE_N 0x0020
#define UMODE_S 0x0040
#define UMODE_q 0x0080
#define UMODE_z 0x0100
#define UMODE_SUPERADMIN 0x0200 /* fake umode */

#define UMODE_OPER UMODE_o
#define UMODE_SADMIN UMODE_a
#define UMODE_ADMIN UMODE_A
#define UMODE_REGISTERED UMODE_r
#define UMODE_BOT UMODE_B
#define UMODE_NADMIN UMODE_N
#define UMODE_SERVICE UMODE_S
#define UMODE_NOKICK UMODE_q
#define UMODE_SSL UMODE_z

#define SetUmode(x, y) ((x)->umodes |= (y))
#define ClearUmode(x, y) ((x)->umodes &= ~(y))
#define HasUmode(x, y) ((x)->umodes & (y))

#define IsOper(x) HasUmode(x, UMODE_OPER)
#define IsAdmin(x) HasUmode(x, UMODE_ADMIN)
#define IsSAdmin(x) HasUmode(x, UMODE_SADMIN)
#define IsNAdmin(x) HasUmode(x, UMODE_NADMIN)
#define IsBot(x) HasUmode(x, UMODE_BOT)
#define IsService(x) HasUmode(x, UMODE_SERVICE)
#define IsNokick(x) HasUmode(x, UMODE_NOKICK)
#define IsSSL(x) HasUmode(x, UMODE_SSL)
#define IsRegistered(x) HasUmode(x, UMODE_REGISTERED)

#define SetOper(x) SetUmode(x, UMODE_OPER)
#define SetAdmin(x) SetUmode(x, UMODE_ADMIN)
#define SetSAdmin(x) SetUmode(x, UMODE_SADMIN)
#define SetNAdmin(x) SetUmode(x, UMODE_NADMIN)
#define SetBot(x) SetUmode(x, UMODE_BOT)
#define SetService(x) SetUmode(x, UMODE_SERVICE)
#define SetNokick(x) SetUmode(x, UMODE_NOKICK)
#define SetSSL(x) SetUmode(x, UMODE_SSL)
#define SetRegistered(x) SetUmode(x, UMODE_REGISTERED)

#define ClearOper(x) ClearUmode(x, UMODE_OPER)
#define ClearAdmin(x) ClearUmode(x, UMODE_ADMIN)
#define ClearSAdmin(x) ClearUmode(x, UMODE_SADMIN)
#define ClearNAdmin(x) ClearUmode(x, UMODE_NADMIN)
#define ClearBot(x) ClearUmode(x, UMODE_BOT)
#define ClearService(x) ClearUmode(x, UMODE_SERVICE)
#define ClearNokick(x) ClearUmode(x, UMODE_NOKICK)
#define ClearSSL(x) ClearUmode(x, UMODE_SSL)
#define ClearRegistered(x) ClearUmode(x, UMODE_REGISTERED)

typedef struct nick {
    char nick[NICKLEN + 1]; /* hash key */
    char ident[NICKLEN + 1];
    char host[HOSTLEN + 1];
    char uid[UIDLEN + 1];
    char svid[SVIDLEN + 1];
    char hiddenhost[HOSTLEN + 1];
    char reshost[HOSTLEN + 1];
    long int umodes;
    int msgtime,msgnb;
    int ignored,ignoretime;
    int loginattempts,lasttry;
    struct llist_head wchans;
    struct llist_head server_head;
} Nick;

#endif
