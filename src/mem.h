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


#ifndef _MEM_H
#define _MEM_H

#include "undef.h"

/* for the get_mem() function */
#define MEM_VSIZE 1
#define MEM_RSS 2

#define TABLESZ 5003
#define MAX_HASH TABLESZ

#define LIST_EMPTY(x) ((x).size == 0)

#define LIST_INSERT_HEAD(x, y, z) { \
                                    (y)->prev = (y)->next = NULL; \
                                    if ((x).table[z]) { \
                                        (x).table[z]->prev = (y); \
                                        (y)->next = (x).table[z]; \
                                    } \
                                    (x).table[z] = (y); \
                                    (y)->lprev = (y)->lnext = NULL; \
                                    if ((x).lhead) { \
                                        (x).lhead->lprev = (y); \
                                        (y)->lnext = (x).lhead; \
                                    } \
                                    (x).lhead = (y); \
                                    (x).size++; \
                                  }

#define LIST_REMOVE(x, y, z) { \
                                if ((y) == (x).table[z]) { \
                                    (x).table[z] = (y)->next; \
                                    if ((x).table[z]) \
                                        (x).table[z]->prev = NULL; \
                                } else { \
                                    if ((y)->next) \
                                        (y)->next->prev = (y)->prev; \
                                    (y)->prev->next = (y)->next; \
                                } \
                                if ((y) == (x).lhead) { \
                                    (x).lhead = (y)->lnext; \
                                    if ((x).lhead) \
                                        (x).lhead->lprev = NULL; \
                                } else { \
                                    if ((y)->lnext) \
                                        (y)->lnext->lprev = (y)->lprev; \
                                    (y)->lprev->lnext = (y)->lnext; \
                                } \
                                (x).size--; \
                              }

#define LIST_FOREACH(x, y, z) for ((y) = (x).table[z]; (y); (y) = (y)->next)
#define LIST_FOREACH_ALL(x, y) for ((y) = (x).lhead; (y); (y) = (y)->lnext)

#define LIST_HEAD(x) ((x).lhead)

#define LIST_NEXT(x) ((x)->next)
#define LIST_LNEXT(x) ((x)->lnext)

#define LIST_INIT(x) memset(&x, 0, sizeof(x))

#define HASH(x) hash(x)
#define HASH_INT(x) ((x) % TABLESZ)

#define TABLE(x) x *table[TABLESZ]

long get_mem (int);
void InitMem (void);
void FreeAllMem (void);
void cleanup_reconnect (void);
int hash(char *buffer);

#endif
