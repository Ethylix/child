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
#include "nick.h"

#include <time.h>

#define EMAILLEN 100
#define MD5_LEN  32

#define TIMEOUT_DFLT 60

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
#define _killuser(x,y,z) SendRaw(":%s KILL %s :%s!%s (%s)",z,x,core_get_config()->name,z,y)
#define killuser(x,y,z) { \
                            _killuser(x,y,z); \
                            userquit(x); \
                        }
#define glineuser(a,b,c,d) SendRaw("TKL + G %s %s %s %ld %ld :%s",a,b,core_get_config()->name,(c) ? time(NULL) + c : 0,time(NULL),d)
#define unglineuser(a,b) SendRaw("TKL - G %s %s %s",a,b,core_get_config()->name)

#define NoticeToUser(a,b,...) FakeNotice(core_get_config()->nick,a,b,##__VA_ARGS__)

#define Global(a, ...) send_global("*",a,##__VA_ARGS__)

typedef struct user_ {
    char nick[NICKLEN + 1]; /* hash key */
    int authed,level;
    char md5_pass[MD5_LEN + 1];
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

User *find_account(const Nick *);
User *find_user(const char *);
Guest *find_guest(const char *);
Link *find_link(const char *);
Link *find_link2 (char *, char *);
User *AddUser (const char *, int);
Guest *AddGuest (char *, int, int);
Link *AddLink (const char *, const char *);
void DeleteAccount (User *);
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
