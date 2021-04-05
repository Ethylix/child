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


#include "nick.h"

#include "core.h"
#include "core_api.h"
#include "hashmap.h"
#include "string_utils.h"
#include "user.h"

static Nick *find_nick(const char *name_or_uid)
{
    Nick *tmp;
    struct hashmap_entry *entry;

    // TODO(target0): do a proper O(1) lookup.
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_nicks(), entry, tmp) {
        if (!Strcmp(tmp->nick, name_or_uid) || !Strcmp(tmp->uid, name_or_uid))
            return tmp;
    }

    return NULL;
}

static Nick *new_nick(char *nick, char *ident, char *host, char *uid, char *hiddenhost, long int umodes, char *reshost)
{
    Nick *new_nick;

    new_nick = (Nick *)malloc(sizeof(Nick));
    memset(new_nick, 0, sizeof(*new_nick));

    strncpy(new_nick->nick, nick, NICKLEN);
    strncpy(new_nick->ident,ident,NICKLEN);
    strncpy(new_nick->host,host,HOSTLEN);
    strncpy(new_nick->uid, uid, UIDLEN);
    strncpy(new_nick->hiddenhost,hiddenhost,HOSTLEN);
    strncpy(new_nick->reshost,reshost,HOSTLEN);
    new_nick->umodes = umodes;
    new_nick->msgnb = 0;
    new_nick->msgtime = 0;
    new_nick->ignored = 0;
    new_nick->ignoretime = 0;
    new_nick->loginattempts = 0;
    new_nick->lasttry = 0;
    LLIST_INIT(&new_nick->wchans);
    LLIST_INIT(&new_nick->server_head);

    if (!HASHMAP_INSERT(core_get_nicks(), new_nick->nick, new_nick, NULL)) {
        fprintf(stderr, "Failed to insert new nick \"%s\" into hashmap (duplicate entry?)\n", new_nick->nick);
        free(new_nick);
        return NULL;
    }

    // TODO: move to event processing function
    Clone *clone;
    if ((clone = find_clone(reshost)) != NULL)
        clone->count++;
    else {
        clone = (Clone *)malloc(sizeof(Clone));
        memset(clone, 0, sizeof(*clone));

        strncpy(clone->host, reshost, HOSTLEN);
        clone->count = 1;
        if (!HASHMAP_INSERT(core_get_clones(), clone->host, clone, NULL)) {
            fprintf(stderr, "Failed to insert new clone \"%s\" into hashmap (duplicate entry?)\n", clone->host);
            free(clone);
        }
    }

    return new_nick;
}

static void delete_nick(Nick *nptr)
{
    Clone *clone;

    if (!HASHMAP_ERASE(core_get_nicks(), nptr->nick))
        return;

    if ((clone = find_clone(nptr->reshost)) != NULL) {
        clone->count--;
        if (clone->count == 0) {
            HASHMAP_ERASE(core_get_clones(), clone->host);
            free(clone);
        }
    }
    LLIST_REMOVE(&nptr->server_head);
    free(nptr);
}

static void clear_nicks(void)
{
    struct hashmap_entry *entry, *tmp_entry;
    Nick *nptr;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_nicks(), entry, tmp_entry, nptr) {
        delete_nick(nptr);
    }
}

static struct core_api nick_api = {
    .find_nick = find_nick,
    .new_nick = new_nick,
    .delete_nick = delete_nick,
    .clear_nicks = clear_nicks,
};

REGISTER_CORE_API(&nick_api);
