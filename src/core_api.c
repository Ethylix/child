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


#include "core_api.h"

#include <assert.h>

static struct core_api core_api;

struct core_api *get_core_api(void)
{
    return &core_api;
}

void register_core_api(struct core_api *api)
{
    assert((sizeof(struct core_api) % sizeof(void *)) == 0);

    // Copy non-zero function pointers from @api to core_api.
    // For this to work, the following needs to be true:
    // - Undefined function pointers in @api are initialized to zero.
    //    - Works nicely with static data.
    // - struct core_api only holds function pointers.
    //    - Partially verified with the above assert().
    for (size_t offset = 0; offset < sizeof(struct core_api); offset += sizeof(void *)) {
        void **core_api_ptr = (void **)((char *)&core_api + offset);
        void *api_ptr = *((void **)((char *)api + offset));
        if (api_ptr)
            *core_api_ptr = api_ptr;
    }
}
