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


#include "trust.h"

#include "core.h"
#include "hashmap.h"
#include "mem.h"
#include "net.h"
#include "string_utils.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

Trust *find_trust_strict (const char *host)
{
    struct hashmap_entry *entry;

    if (!host)
        return NULL;

    if (!HASHMAP_FIND(core_get_trusts(), host, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(core_get_trusts(), entry);
}

Trust *find_trust(const char *host)
{
    char *mask,*bits;
    Trust *tmp;
    struct hashmap_entry *entry;

    // TODO(target0): improve this.
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_trusts(), entry, tmp) {
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

Trust *AddTrust(const char *host, int limit)
{
    Trust *new_trust;
    new_trust = (Trust *)malloc(sizeof(Trust));
    memset(new_trust, 0, sizeof(*new_trust));

    strncpy(new_trust->host,host,HOSTLEN);
    new_trust->limit = limit;

    if (!HASHMAP_INSERT(core_get_trusts(), new_trust->host, new_trust, NULL)) {
        fprintf(stderr, "Failed to insert new trust \"%s\" into hashmap (duplicate entry?)\n", new_trust->host);
        free(new_trust);
        return NULL;
    }
    return new_trust;
}

void DeleteTrust(Trust *trust)
{
    HASHMAP_ERASE(core_get_trusts(), trust->host);
    free(trust);
}
