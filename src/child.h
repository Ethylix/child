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


#ifndef _CHILD_H
#define _CHILD_H


#include <config.h>

#include "server.h"
#include "user.h"

#define HASHMAX 65000
#define CONF_LOAD   0
#define CONF_REHASH 1

#define DEFAULT_CONFFILE "child.conf"

char *strcasestr(const char *, const char *);

void child_die(int);
void child_restart(int);
void child_clean(void) __attribute__((noreturn));

/* loadconf.c */
void loadconf(int);

/* md5.c */
char *md5_hash(char *);

/* parseline.c */
int parse_line(char *line);

#endif
