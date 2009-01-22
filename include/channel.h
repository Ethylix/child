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


#ifndef _CHANNEL_H
#define _CHANNEL_H

#include <mem.h>
#include <user.h>

#define OPERCHAN "#opers"

#define CHANLEN 35
#define MASKLEN 93
#define TOPICLEN 310

#define CHLEV_OWNER 10000
#define CHLEV_COOWNER 9999

#define CHFL_OP      0x0001
#define CHFL_HALFOP  0x0002
#define CHFL_VOICE   0x0004
#define CHFL_PROTECT 0x0008
#define CHFL_OWNER   0x0010
#define CHFL_ALL (CHFL_OP|CHFL_HALFOP|CHFL_VOICE|CHFL_PROTECT|CHFL_OWNER)

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
#define COPT_ENABLEMASK  0x0400
#define COPT_PROTECTOPS  0x0800
#define COPT_MASS        0x1000
#define COPT_ENFTOPIC    0x2000

#define CFLAG_AUTO_OFF      0
#define CFLAG_AUTO_ON       1
#define CFLAG_AUTO_VOICE    2
#define CFLAG_AUTO_OP       3

#define PartChannel(x) SendRaw(":%s PART %s",whatbot(x),x)
#define InviteUser(x,y) SendRaw(":%s INVITE %s :%s",me.nick,x,y)

#define IsChanSuspended(x) HasOption(x, COPT_SUSPENDED)
#define IsChanNoexpire(x) HasOption(x, COPT_NOEXPIRE)

#define IsFounder(x,y) ((GetFlag(x,y) == CHLEV_OWNER) || (IsCoOwner(x,y)))
#define IsTrueOwner(x,y) (GetFlag(x,y) == CHLEV_OWNER)
#define IsCoOwner(x,y) (GetFlag(x,y) == CHLEV_COOWNER)

#define IsOwner(a,b) IsChanFlag(a,b,CHFL_OWNER)
#define IsProtect(a,b) IsChanFlag(a,b,CHFL_PROTECT)
#define IsOp(a,b) IsChanFlag(a,b,CHFL_OP)
#define IsHalfop(a,b) IsChanFlag(a,b,CHFL_HALFOP)
#define IsVoice(a,b) IsChanFlag(a,b,CHFL_VOICE)

#define MsgToChan(a,b,...) FakeMsg(me.nick,a,b,##__VA_ARGS__)

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

typedef struct chan {
    char channelname[CHANLEN + 1]; /* hash key */
    char owner[NICKLEN + 1];
    long int options;
    char entrymsg[250+1];
    char mlock[50+1];
    char topic[TOPICLEN + 1];
    int autolimit, lastseen, regtime;
    struct chan *next,*prev;
    struct chan *lnext,*lprev;
} Chan;

typedef struct wchan {
    char chname[CHANLEN + 1]; /* hash key */
    char topic[TOPICLEN + 1];
    struct wchan *next,*prev;
    struct wchan *lnext,*lprev;
} Wchan;

typedef struct member_ {
    char nick[NICKLEN + 1];
    char channel[CHANLEN + 1]; /* hash key */
    long int flags;
    struct member_ *next,*prev;
    struct member_ *lnext,*lprev;
} Member;

typedef struct cflag {
    char channel[CHANLEN + 1]; /* hash key */
    char nick[NICKLEN + MASKLEN + 1];
    int flags;
    int automode;
    int suspended;
    struct cflag *next,*prev;
    struct cflag *lnext,*lprev;
} Cflag;

typedef struct limit_ {
    char channel[CHANLEN + 1]; /* hash key */
    int time;
    struct limit_ *next,*prev;
    struct limit_ *lnext,*lprev;
} Limit;

typedef struct timeban {
    char channel[CHANLEN + 1];
    char mask[MASKLEN + 1];
    char reason[256+1];
    int duration, setat;
    struct timeban *next, *prev;
    struct timeban *lnext, *lprev;
} TB;

typedef struct {
    int size;
    TABLE(Chan);
    Chan *lhead;
} chanlist;

typedef struct {
    int size;
    TABLE(Wchan);
    Wchan *lhead;
} wchanlist;

typedef struct {
    int size;
    TABLE(Member);
    Member *lhead;
} memberlist;

typedef struct {
    int size;
    TABLE(Cflag);
    Cflag *lhead;
} cflaglist;

typedef struct {
    int size;
    TABLE(Limit);
    Limit *lhead;
} limitlist;

typedef struct {
    int size;
    TABLE(TB);
    TB *lhead;
} tblist;

Chan *find_channel (char *);
Wchan *find_wchan (char *);
Cflag *find_cflag (char *, char *);
int GetFlag (User *, Chan *);
Chan *CreateChannel (char *, char *, int);
Wchan *CreateWchan(char *);
void DeleteChannel(Chan *);
void DeleteWchan(Wchan *);
Cflag *AddUserToChannel (User *, Chan *, int);
void DeleteUserFromChannel (User *, Chan *);
Cflag *AddMaskToChannel (char *, Chan *, int);
void DeleteMaskFromChannel (char *, Chan *);
Member *AddUserToWchan(Nick *, Wchan *);
void DeleteUserFromWchan (Nick *, Wchan *);
void KickUser(char *, char *, char *, char *);
void JoinChannel(char *, char *);
Member *find_member (char *, char *);
void SetStatus (Nick *, char *, long int, int, char *);
void DeleteUserFromChannels (User *);
void DeleteUsersFromChannel (Chan *);
void DeleteUserFromWchans(Nick *);
int member_exists (Wchan *);
int members_num (Wchan *);
void checkexpired (void);
int IsChanFlag (char *, Wchan *, int);
void CheckLimits (void);
Limit *AddLimit (char *);
int IsMember(char *, char *);
int chansreg (char *);
char *whatbot (char *);
void joinallchans (void);
void DeleteCflag (Cflag *);
void DeleteMember (Member *);
void DeleteLimit (Limit *);
inline void chandrop (Chan *);
TB *AddTB (Chan *, char *, int, char *);
void DeleteTB (TB *);
void CheckTB (void);
TB *find_tb (Chan *, char *);
void acl_resync (Chan *);
void sync_cflag (Cflag *, Nick *);
int IsAclOnChan (Chan *);

#endif
