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


#ifndef _GLOBALS_H
#define _GLOBALS_H
#include <child.h>

extern int sock;
extern int esock;
extern int startuptime;

extern userlist user_list;
extern nicklist nick_list;
extern cloneslist clones_list;
extern modulelist module_list;
extern hooklist hook_list;
extern trustlist trust_list;
extern linklist link_list;
extern eclientlist eclient_list;
extern guestlist guest_list;
extern chanlist chan_list;
extern wchanlist wchan_list;
extern cflaglist cflag_list;
extern memberlist member_list;
extern limitlist limit_list;
extern botlist bot_list;
extern chanbotlist chanbot_list;
extern commandlist command_list;
#ifdef USE_FILTER
extern rulelist rule_list;
#endif
extern tblist tb_list;
extern fakelist fake_list;

extern MYSQL mysql,mysql2;
extern int verbose,vv,raws,eos;
extern int emerg,emerg_req;
extern struct pollfd ufds[ECL_MAXSOCK];
#ifdef USE_GNUTLS
extern gnutls_session_t session;
extern gnutls_certificate_credentials_t xcred;
#endif
#endif
