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

#include <llist.h>
#include <stdbool.h>
#include <time.h>

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

#define SetUmode(_nptr, _umode) \
    { \
        long int _umodes = nick_umodes(_nptr); \
        nick_set_umodes(_nptr, _umodes | _umode); \
    }

#define ClearUmode(_nptr, _umode) \
    { \
        long int _umodes = nick_umodes(_nptr); \
        nick_set_umodes(_nptr, _umodes & ~_umode); \
    }

#define HasUmode(_nptr, _umode) (nick_umodes(_nptr) & _umode)

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

typedef struct nick Nick;
typedef struct user User;

struct nick_llist_wrapper {
    Nick *nick;
    struct llist_head wchans;
    struct llist_head server_head;
};

const char *nick_name(const Nick *nptr);
void nick_set_name(Nick *nptr, const char *name);
const char *nick_ident(const Nick *nptr);
void nick_set_ident(Nick *nptr, const char *ident);
const char *nick_host(const Nick *nptr);
void nick_set_host(Nick *nptr, const char *host);
const char *nick_uid(const Nick *nptr);
const char *nick_svid(const Nick *nptr);
void nick_set_svid(Nick *nptr, const char *svid);
const char *nick_hiddenhost(const Nick *nptr);
void nick_set_hiddenhost(Nick *nptr, const char *hiddenhost);
const char *nick_reshost(const Nick *nptr);
void nick_set_reshost(Nick *nptr, const char *reshost);
long int nick_umodes(const Nick *nptr);
void nick_set_umodes(Nick *nptr, long int umodes);
time_t nick_msgtime(const Nick *nptr);
void nick_set_msgtime(Nick *nptr, time_t time);
int nick_msgnb(const Nick *nptr);
void nick_set_msgnb(Nick *nptr, int msgnb);
bool nick_ignored(const Nick *nptr);
void nick_set_ignored(Nick *nptr, bool ignored);
time_t nick_ignoretime(const Nick *nptr);
void nick_set_ignoretime(Nick *nptr, time_t ignoretime);
int nick_loginattempts(const Nick *nptr);
void nick_set_loginattempts(Nick *nptr, int loginattempts);
time_t nick_lasttry(const Nick *nptr);
void nick_set_lasttry(Nick *nptr, time_t lasttry);
struct user *nick_account(const Nick *nptr);
void nick_set_account(Nick *nptr, User *account);

struct nick_llist_wrapper *nick_llist_wrapper(Nick *nptr);

#endif
