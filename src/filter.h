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

#ifndef _FILTER_H
#define _FILTER_H

#include "user.h"

#define DEFAULT_RULESFILE "filter.conf"

#define RULE_DROP   0
#define RULE_PASS   1

#define DIRECT_IN   0
#define DIRECT_OUT  1

struct ruleset {
    int rule;
    int direction;
    int quick;
    char from[NICKLEN+1];
    char to[NICKLEN+1];
    char action[128+1];
    char data[1024+1];
    int icase;
    struct ruleset *next,*prev;
    struct ruleset *lnext,*lprev;
};

typedef struct {
    int size;
    int enabled;
    TABLE(struct ruleset);
    struct ruleset *lhead, *ltail;
} rulelist;

int match_rules (struct ruleset *, struct ruleset *);
int filter_check (char *, int);
struct ruleset *add_rule (char *data);
void remove_rule (struct ruleset *);
int loadrulefile (void);

#endif
