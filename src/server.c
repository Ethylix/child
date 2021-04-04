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


#include "server.h"

#include "child.h"
#include "core.h"
#include "string_utils.h"

Server *find_server(const char *name_or_sid)
{
    Server *tmp;

    LLIST_FOREACH_ENTRY(core_get_servers(), tmp, list_head) {
        if (!Strcmp(tmp->name, name_or_sid) || !Strcmp(tmp->sid, name_or_sid)) {
            return tmp;
        }
    }

    return NULL;
}

Server *add_server(const char *name, const char *sid, Server *hub) {
    Server *server;

    server = malloc(sizeof(*server));
    if (!server)
        return NULL;

    memset(server, 0, sizeof(*server));

    strncpy(server->name, name, SERVERNAMELEN);
    strncpy(server->sid, sid, SIDLEN);
    LLIST_INIT(&server->nicks);
    LLIST_INIT(&server->leafs);
    LLIST_INIT(&server->leaf_head);

    LLIST_INSERT_TAIL(core_get_servers(), &server->list_head);

    if (hub)
        LLIST_INSERT_TAIL(&hub->leafs, &server->leaf_head);

    return server;
}

void delete_server(Server *server) {
    if (!LLIST_EMPTY(&server->nicks)) {
        operlog("!!! Server has non-empty nick list on delete, this should never happen");
    }

    LLIST_REMOVE(&server->list_head);
    LLIST_REMOVE(&server->leaf_head);
    free(server);
}

void detach_server_recursive(Server *server) {
    Nick *nptr, *tmp_nptr;
    Server *sptr, *tmp_sptr;

    LLIST_FOREACH_ENTRY_SAFE(&server->nicks, nptr, tmp_nptr, server_head) {
        userquit(nptr->nick);
    }

    LLIST_FOREACH_ENTRY_SAFE(&server->leafs, sptr, tmp_sptr, leaf_head) {
        detach_server_recursive(sptr);
    }

    delete_server(server);
}
