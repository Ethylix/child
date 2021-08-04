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


#ifndef _USER_H
#define _USER_H

#include "llist.h"
#include "mem.h"
#include "net.h"

#include <time.h>

#define NICKLEN 35
#define HOSTLEN 60
#define EMAILLEN 100

#define UIDLEN 9

#define SVIDLEN 35

#define TIMEOUT_DFLT 60

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

#define UOPT_PROTECT    0x0001
#define UOPT_PRIVATE    0x0002
#define UOPT_SUSPENDED  0x0004
#define UOPT_NOEXPIRE   0x0008
#define UOPT_CANPL      0x0010
#define UOPT_NOAUTO     0x0020
#define UOPT_HIDEMAIL   0x0040
#define UOPT_CLOAKED    0x0080

#define SetOption(x, y) ((x)->options |= (y))
#define ClearOption(x, y) ((x)->options &= ~(y))
#define HasOption(x, y) ((x)->options & (y))

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

#define IsUserSuspended(x) HasOption(x, UOPT_SUSPENDED)
#define IsUserNoexpire(x) HasOption(x, UOPT_NOEXPIRE)
#define CanPl(x) HasOption(x, UOPT_CANPL)

#define IsAuthed(x) ((x) && (x)->authed == 1)

#define fakeuser(nick_,ident_,host_, uid_, umodes_) { \
                                if ((AddFake(nick_, ident_, host_, uid_)) != NULL) { \
                                    SendRaw("UID %s 1 %ld %s %s %s 0 +%s * * * :%s", nick_, time(NULL), ident_, host_, uid_, umodes_, ident_); \
                                } \
                          }
#define fakejoin(x,y) SendRaw(":%s JOIN %s",x,y)
#define fakekill(x,y) { \
                            if (find_fake(x) != NULL) \
                                DeleteFake(find_fake(x)); \
                            SendRaw(":%s QUIT :%s",x,y); \
                      }
#define fakesay(x,y,z) SendRaw(":%s PRIVMSG %s :%s",x,y,z)
#define _killuser(x,y,z) SendRaw(":%s KILL %s :%s!%s (%s)",z,x,me.name,z,y)
#define killuser(x,y,z) { \
                            _killuser(x,y,z); \
                            userquit(x); \
                        }
#define glineuser(a,b,c,d) SendRaw("TKL + G %s %s %s %ld %ld :%s",a,b,me.name,(c) ? time(NULL) + c : 0,time(NULL),d)
#define unglineuser(a,b) SendRaw("TKL - G %s %s %s",a,b,me.name)

#define NoticeToUser(a,b,...) FakeNotice(me.nick,a,b,##__VA_ARGS__)

#define Global(a, ...) send_global("*",a,##__VA_ARGS__)

typedef struct user_ {
    char nick[NICKLEN + 1]; /* hash key */
    int authed,level;
    char md5_pass[35];
    int lastseen,timeout;
    long int options;
    char vhost[HOSTLEN + 1];
    char email[EMAILLEN + 1];
    int regtime;
    struct llist_head cflags;
} User;

typedef struct clone {
    char host[HOSTLEN + 1];
    int count;
} Clone;

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

typedef struct guest {
    char nick[NICKLEN+1]; /* hash key */
    int nickconn;
    int timeout;
} Guest;

typedef struct link_ {
    char master[NICKLEN + 1];
    char slave[NICKLEN + 1]; /* hash key */
} Link;

typedef struct fakeuser {
    char nick[NICKLEN + 1]; /* hash key */
    char ident[NICKLEN + 1];
    char host[HOSTLEN + 1];
    char uid[UIDLEN + 1];
} Fake;

User *find_user(const char *);
Nick *find_nick(const char *);
Guest *find_guest(const char *);
Link *find_link(const char *);
Link *find_link2 (char *, char *);
User *AddUser (char *, int);
Nick *AddNick (char *, char *, char *, char *, char *, long int, char *);
Guest *AddGuest (char *, int, int);
Link *AddLink (char *, char *);
void DeleteAccount (User *);
void DeleteWildNick (Nick *);
void clear_nicks(void);
void DeleteGuest (char *);
void clear_guests(void);
void DeleteLink (char *);
void DeleteLinks (char *);
void globops (char *, ...);
void send_global (char *, char *, ...);
int howmanyclones (char *);
void CheckGuests (void);
int match_mask (char *, char *);
int IsMask(char *);
int sendmail (char *, char *);
void FakeMsg(const char *, const char *, const char *, ...);
void FakeNotice(const char *, const Nick *, const char *, ...);
void userquit (char *);
void killallfakes (void);
void loadallfakes (void);
void userdrop (User *);
Clone *find_clone (char *);
void sync_user(const User *);
Fake *AddFake (const char *, const char *, const char *, const char *);
Fake *find_fake (const char *);
void DeleteFake (Fake *);
void clear_fakes(void);
User *get_link_master (User *);
int IsSuperAdmin (User *);
void generate_uid(char *);

#endif
