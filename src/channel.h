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


#ifndef _CHANNEL_H
#define _CHANNEL_H

#include "mem.h"
#include "user.h"

#define OPERCHAN "#opers"

#define CHANLEN 35
#define MASKLEN 255
#define TOPICLEN 310

#define CHLEV_OWNER 10000
#define CHLEV_COOWNER 9999

#define CHFL_OP      0x0001
#define CHFL_HALFOP  0x0002
#define CHFL_VOICE   0x0004
#define CHFL_PROTECT 0x0008
#define CHFL_OWNER   0x0010
#define CHFL_ALL (CHFL_OP|CHFL_HALFOP|CHFL_VOICE|CHFL_PROTECT|CHFL_OWNER)

#define SetOption(x, y) ((x)->options |= (y))
#define ClearOption(x, y) ((x)->options &= ~(y))
#define HasOption(x, y) ((x)->options & (y))

#define SetChanFlag(x, y) ((x)->flags |= (y))
#define ClearChanFlag(x, y) ((x)->flags &= ~(y))
#define HasChanFlag(x, y) ((x)->flags & (y))

#define SetOp(x) SetChanFlag(x, CHFL_OP)
#define SetHalfop(x) SetChanFlag(x, CHFL_HALFOP)
#define SetVoice(x) SetChanFlag(x, CHFL_VOICE)
#define SetProtect(x) SetChanFlag(x, CHFL_PROTECT)
#define SetOwner(x) SetChanFlag(x, CHFL_OWNER)

#define ClearOp(x) ClearChanFlag(x, CHFL_OP)
#define ClearHalfop(x) ClearChanFlag(x, CHFL_HALFOP)
#define ClearVoice(x) ClearChanFlag(x, CHFL_VOICE)
#define ClearProtect(x) ClearChanFlag(x, CHFL_PROTECT)
#define ClearOwner(x) ClearChanFlag(x, CHFL_OWNER)

#define COPT_UNUSED      0x0001
#define COPT_NOJOIN      0x0002
#define COPT_NOAUTO      0x0004
#define COPT_AVOICE      0x0008
#define COPT_PRIVATE     0x0010
#define COPT_STRICTOP    0x0020
#define COPT_AOP         0x0040
#define COPT_SECURE      0x0080
#define COPT_SUSPENDED   0x0100
#define COPT_NOEXPIRE    0x0200
#define COPT_PROTECTOPS  0x0800
#define COPT_MASS        0x1000
#define COPT_ENFTOPIC    0x2000
#define COPT_AXXFLAGS    0x4000

#define CFLAG_AUTO_OFF      0
#define CFLAG_AUTO_ON       1
#define CFLAG_AUTO_VOICE    2
#define CFLAG_AUTO_OP       3

#define PartChannel(x) SendRaw(":%s PART %s",channel_botname(x),x)
#define InviteUser(x,y) SendRaw(":%s INVITE %s :%s",core_get_config()->nick,x,y)

#define IsChanSuspended(x) HasOption(x, COPT_SUSPENDED)
#define IsChanNoexpire(x) HasOption(x, COPT_NOEXPIRE)

#define IsCoOwner(x,y) (GetFlag(x,y) == CHLEV_COOWNER)

#define HasOwner(a) HasChanFlag(a,CHFL_OWNER)
#define HasProtect(a) HasChanFlag(a,CHFL_PROTECT)
#define HasOp(a) HasChanFlag(a,CHFL_OP)
#define HasHalfop(a) HasChanFlag(a,CHFL_HALFOP)
#define HasVoice(a) HasChanFlag(a,CHFL_VOICE)

#define MsgToChan(a,b,...) FakeMsg(core_get_config()->nick,a,b,##__VA_ARGS__)

/* That's currently not used, but maybe in the future :) */

#define CM_c    0x0000001
#define CM_f    0x0000002
#define CM_i    0x0000004
#define CM_j    0x0000008
#define CM_k    0x0000010
#define CM_l    0x0000020
#define CM_m    0x0000040
#define CM_n    0x0000080
#define CM_p    0x0000100
#define CM_r    0x0000200
#define CM_s    0x0000400
#define CM_t    0x0000800
#define CM_z    0x0001000
#define CM_A    0x0002000
#define CM_C    0x0004000
#define CM_G    0x0008000
#define CM_M    0x0010000
#define CM_K    0x0020000
#define CM_L    0x0040000
#define CM_N    0x0080000
#define CM_O    0x0100000
#define CM_Q    0x0200000
#define CM_R    0x0400000
#define CM_S    0x0800000
#define CM_T    0x1000000
#define CM_V    0x2000000
#define CM_u    0x4000000
#define OPERSONLY_CM (CM_A|CM_O)

#define UFLAG_OP            0x00001 // o
#define UFLAG_VOICE         0x00002 // v
#define UFLAG_AUTOOP        0x00004 // O
#define UFLAG_AUTOVOICE     0x00008 // V
#define UFLAG_ACL           0x00010 // x
#define UFLAG_INVITE        0x00020 // i
#define UFLAG_TOPIC         0x00040 // t
#define UFLAG_OWNER         0x00080 // F
#define UFLAG_COOWNER       0x00100 // f
#define UFLAG_HALFOP        0x00200 // h
#define UFLAG_SET           0x00400 // s
#define UFLAG_AUTOHALFOP    0x00800 // H
#define UFLAG_PROTECT       0x01000 // p
#define UFLAG_AUTOPROTECT   0x02000 // P
#define UFLAG_NOOP          0x04000 // N
#define UFLAG_AUTOKICK      0x08000 // k
#define UFLAG_AUTOKICKBAN   0x10000 // b
#define UFLAG_AUTOOWNER     0x20000 // w
#define UFLAG_CHANOWNER (UFLAG_OP|UFLAG_VOICE|UFLAG_ACL|UFLAG_INVITE|UFLAG_TOPIC|UFLAG_OWNER|UFLAG_HALFOP|UFLAG_SET|UFLAG_PROTECT|UFLAG_AUTOOWNER|UFLAG_AUTOOP)
#define UFLAG_CHANCOOWNER (UFLAG_OP|UFLAG_VOICE|UFLAG_ACL|UFLAG_INVITE|UFLAG_TOPIC|UFLAG_COOWNER|UFLAG_HALFOP|UFLAG_SET|UFLAG_PROTECT|UFLAG_AUTOOWNER|UFLAG_AUTOOP)
#define UFLAG_CHANSADMIN (UFLAG_OP|UFLAG_VOICE|UFLAG_ACL|UFLAG_INVITE|UFLAG_TOPIC|UFLAG_HALFOP|UFLAG_SET|UFLAG_PROTECT|UFLAG_AUTOPROTECT|UFLAG_AUTOOP)
#define UFLAG_CHANADMIN (UFLAG_OP|UFLAG_VOICE|UFLAG_ACL|UFLAG_INVITE|UFLAG_TOPIC|UFLAG_HALFOP|UFLAG_PROTECT|UFLAG_AUTOPROTECT|UFLAG_AUTOOP)
#define UFLAG_CHANOP (UFLAG_OP|UFLAG_VOICE|UFLAG_AUTOOP|UFLAG_INVITE|UFLAG_TOPIC|UFLAG_HALFOP)
#define UFLAG_CHANHALFOP (UFLAG_VOICE|UFLAG_INVITE|UFLAG_TOPIC|UFLAG_HALFOP|UFLAG_AUTOHALFOP)
#define UFLAG_CHANVOICE (UFLAG_VOICE|UFLAG_AUTOVOICE|UFLAG_INVITE)

struct bot;
struct limit_;

typedef struct chan {
    char channelname[CHANLEN + 1]; /* hash key */
    char owner[NICKLEN + 1];
    long int options;
    char entrymsg[250+1];
    char mlock[50+1];
    char topic[TOPICLEN + 1];
    int autolimit, lastseen, regtime;
    struct limit_ *active_autolimit;
    struct llist_head cflags;
    struct bot *chanbot;
    struct llist_head timebans;
} Chan;

typedef struct wchan {
    char chname[CHANLEN + 1]; /* hash key */
    char topic[TOPICLEN + 1];
    struct llist_head members;
} Wchan;

typedef struct member_ {
    Wchan *wchan;
    Nick *nick;
    long int flags;
    struct llist_head wchan_head;
    struct llist_head nick_head;
} Member;

typedef struct cflag {
    Chan *chan;
    User *user;
    int flags; /* level */
    int automode;
    int suspended;
    int uflags; /* flags (really) */
    struct llist_head chan_head;
    struct llist_head user_head;
} Cflag;

typedef struct limit_ {
    Chan *chan;
    int time;
    struct llist_head list_head;
} Limit;

typedef struct timeban {
    Chan *chan;
    char mask[MASKLEN + 1];
    char reason[256+1];
    int duration, setat;
    struct llist_head core_head;
    struct llist_head chan_head;
} Timeban;

Chan *find_channel(const char *);
Wchan *find_wchan(const char *);
Cflag *find_cflag(const Chan *, const User *);
Cflag *find_cflag_recursive(const Chan *, const User *);
int GetFlag (User *, Chan *);
Chan *CreateChannel (char *, char *, int);
Wchan *CreateWchan(char *);
void DeleteChannel(Chan *);
void DeleteWchan(Wchan *);
void clear_wchans(void);
Cflag *AddUserToChannel (User *, Chan *, int, int);
void DeleteUserFromChannel (User *, Chan *);
Member *AddUserToWchan(Nick *, Wchan *);
void DeleteUserFromWchan (Nick *, Wchan *);
void KickUser(const char *, const char *, const char *, const char *, ...);
void JoinChannel(const char *, const char *);
Member *find_member(const Wchan *, const Nick *);
void SetStatus(Nick *, const char *, long int, int, const char *);
void DeleteUserFromChannels (User *);
void DeleteUsersFromChannel (Chan *);
void DeleteUserFromWchans(Nick *);
bool member_exists (Wchan *);
int members_num (Wchan *);
void checkexpired (void);
void CheckLimits (void);
Limit *AddLimit(Chan *);
int chansreg (char *);
const char *channel_botname(const Chan *);
void joinallchans (void);
void DeleteCflag (Cflag *);
void DeleteMember (Member *);
void DeleteLimit (Limit *);
void clear_limits(void);
void chandrop (Chan *);
Timeban *AddTimeban (Chan *, const char *, int, const char *);
void DeleteTimeban (Timeban *);
void CheckTimebans(void);
Timeban *find_timeban (const Chan *, const char *);
void acl_resync (Chan *);
void sync_cflag(const Cflag *);
int IsAclOnChan (Chan *);
int parse_uflags (char *);
int GetUFlagsFromLevel (int);
int ChannelCanACL (User *uptr, Chan *chptr);
int IsFounder (User *, Chan *);
int IsTrueOwner (User *, Chan *);
char *get_uflags_string (int);
int ChannelCanReadACL (User *, Chan *);
int ChannelCanWriteACL (User *, User *, Chan *);
int ChannelCanProtect (User *, Chan *);
int ChannelCanOp (User *, Chan *);
int ChannelCanHalfop (User *, Chan *);
int ChannelCanVoice (User *, Chan *);
int ChannelCanSet (User *, Chan *);
int ChannelCanInvite (User *, Chan *);
int ChannelCanTopic (User *, Chan *);
int ChannelCanOverride (User *, User *, Chan *);
int can_modify_uflag (User *, Chan *, int);
User *get_coowner (Chan *);
Cflag *find_cflag_r (char *, char *);

#endif
