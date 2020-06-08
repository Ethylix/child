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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
USA.
*/


#ifndef _BOTSERV_H
#define _BOTSERV_H

#include "channel.h"
#include "mem.h"
#include "user.h"

typedef struct bot {
    char nick[NICKLEN+1]; /* hash key */
    char ident[NICKLEN+1];
    char host[HOSTLEN+1];
} Bot;
    
Bot *add_bot(const char *nick, const char *ident, const char *host);
void remove_bot(Bot *);
Bot *find_bot(const char *nick);

#endif
