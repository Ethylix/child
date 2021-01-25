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

#ifndef _SERVER_H
#define _SERVER_H

#include "llist.h"

#define SERVERNAMELEN   64
#define SIDLEN          3

typedef struct server {
    char name[SERVERNAMELEN + 1];
    char sid[SIDLEN + 1];
    struct llist_head nicks;
    struct llist_head list_head;
} Server;

Server *find_server(const char *);
Server *add_server(const char *, const char *);
void delete_server(Server *);

#endif
