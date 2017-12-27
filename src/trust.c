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


#include <child.h>
#include <globals.h>
#include <trust.h>

Trust *find_trust_strict (char *host)
{
    Trust *tmp;
    LIST_FOREACH(trust_list, tmp, HASH(host)) {
        if (!Strcmp(tmp->host,host))
            return tmp;
    }

    return NULL;
}

Trust *find_trust(char *host)
{
    char *mask,*bits;
    Trust *tmp;

    LIST_FOREACH_ALL(trust_list, tmp) {
        if (!Strcmp(tmp->host,host))
            return tmp;

        if (strchr(host,':'))
            continue;

        mask = strdup(tmp->host);
        bits = strchr(mask,'/');
        if (!bits) {
            free(mask);
            continue;
        }
        *bits++ = '\0';
        if (match_ipmask(inet_addr(host),inet_addr(mask),atoi(bits))) {
            free(mask);
            return tmp;
        }
        free(mask);
    }

    return NULL;
}

Trust *AddTrust(char *host, int limit)
{
    Trust *new_trust;
    new_trust = (Trust *)malloc(sizeof(Trust));
    strncpy(new_trust->host,host,HOSTLEN);
    new_trust->limit = limit;

    LIST_INSERT_HEAD(trust_list, new_trust, HASH(host));
    return new_trust;
}

void DeleteTrust(Trust *trust)
{
    LIST_REMOVE(trust_list, trust, HASH(trust->host));
    free(trust);
}
