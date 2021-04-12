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


#ifndef _CORE_API_H
#define _CORE_API_H

#include "nick.h"

struct core_api {
    // Nick functions (connected users).
    Nick* (*find_nick)(const char *name_or_uid);
    Nick* (*new_nick)(char *nick, char *ident, char *host, char *uid, char *hiddenhost, long int umodes, char *reshost);
    void (*delete_nick)(Nick *nptr);
    void (*clear_nicks)(void);

    // User functions (registered users).

    // Wchan functions (active channels).

    // Chan functions (registered channels).

    // Events.

    // Database.
    int (*load_all_db)(void);
};

struct core_api *get_core_api(void);
void register_core_api(struct core_api *api);

#define REGISTER_CORE_API(arg)              \
    __attribute__((constructor))            \
    static void __register_core_api(void)   \
    {                                       \
        register_core_api(arg);             \
    }

#endif
