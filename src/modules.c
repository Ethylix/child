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


#include "modules.h"

#include "channel.h"
#include "core.h"
#include "hashmap.h"
#include "mem.h"
#include "string_utils.h"
#include "user.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Module *find_module(const char *name)
{
    struct hashmap_entry *entry;

    if (!name)
        return NULL;

    if (!HASHMAP_FIND(core_get_modules(), name, &entry))
        return NULL;

    return HASHMAP_ENTRY_VALUE(core_get_modules(), entry);
}

Hook *find_hook(const Module *mod, const char *name)
{
    Hook *tmp;

    LLIST_FOREACH_ENTRY(&mod->hooks, tmp, list_head) {
        if (!Strcmp(tmp->name,name))
            return tmp;
    }

    return NULL;
}

void unloadallmod(void)
{
    struct hashmap_entry *entry, *tmp_entry;
    Module *mod;

    HASHMAP_FOREACH_ENTRY_VALUE_SAFE(core_get_modules(), entry, tmp_entry, mod) {
        unloadmodule(mod->modname);
    }
}

Module *loadmodule(const char *name)
{
    char n[60];
    bzero(n,60);
    void *handle;
    void (*func)();
    const char *err = NULL;

    if (find_module(name)) {
        fprintf(stderr,"Error: trying to load module %s: already loaded\n",name);
        return NULL;
    }

    Module *mod;
    sprintf(n,"./src/modules/%s.so",name);

    handle = dlopen(n,RTLD_NOW|RTLD_GLOBAL); /* opening the mod */
    if (!handle) {
        printf("HANDLE ERROR: %s\n",dlerror());
        return NULL;
    }
    func = dlsym(handle,"child_init"); /* retrieving child_init() function */
    if (((err = dlerror()) != NULL) && !func) {
        fprintf(stderr,"ERROR LOADING MODULE %s: %s\n",name,err);
        dlclose(handle);
        return NULL;
    }

    mod = (Module *)malloc(sizeof(Module));
    memset(mod, 0, sizeof(*mod));

    strncpy(mod->modname,name,50);
    mod->modname[50] = '\0';
    mod->handle = handle; /* saving module handle */
    mod->nodreload = 0;
    LLIST_INIT(&mod->hooks);

    if (!HASHMAP_INSERT(core_get_modules(), mod->modname, mod, NULL)) {
        fprintf(stderr, "Failed to insert module \"%s\" into hashmap (duplicate entry?)\n", mod->modname);
        dlclose(handle);
        free(mod);
        return NULL;
    }

    func(mod);

    return mod;
}

int unloadmodule(const char *name)
{
    Module *mod;
    Hook *hook, *tmp_hook;

    mod = find_module(name);
    if (!mod) return 0;

    LLIST_FOREACH_ENTRY_SAFE(&mod->hooks, hook, tmp_hook, list_head) {
        DelHook(hook);
    }

    void (*func)();
    const char *err = NULL;
    func = dlsym(mod->handle,"child_cleanup"); /* retrieving child_cleanup() function */
    if (((err = dlerror()) != NULL) && !func)
        fprintf(stderr,"ERROR UNLOADING MODULE %s: %s\n",name,err);
    else
        func(); /* if it exists, execute it. */

    dlclose(mod->handle); /* closing module handle */

    HASHMAP_ERASE(core_get_modules(), mod->modname);
    free(mod);

    return 1;
}

Hook *AddHook(uint64_t hook_mask, int (*fptr)(Nick *, User *, Chan *, char *[]), char *name, char *modname)
{
    Module *mod;
    Hook *hook;

    if ((mod = find_module(modname)) == NULL) {
        fprintf(stderr, "Cannot add hook %s to module %s: module does not exist.\n", name, modname);
        return NULL;
    }

    if (find_hook(mod, name) != NULL) {
        fprintf(stderr,"Warning: Cannot add hook %s: hook alreay exists\n",name);
        return NULL;
    }

    hook = (Hook *)malloc(sizeof(Hook));
    memset(hook, 0, sizeof(*hook));

    strncpy(hook->name,name,50);
    strncpy(hook->modname,modname,50);
    hook->ptr = fptr;
    hook->hook_mask = hook_mask;

    LLIST_INSERT_TAIL(&mod->hooks, &hook->list_head);

    return hook;
}

void DelHook(Hook *hook)
{
    LLIST_REMOVE(&hook->list_head);
    free(hook);
}

int RunHooks(uint64_t hook_mask, Nick *nptr, User *uptr, Chan *chptr, char *parv[])
{
    struct hashmap_entry *entry;
    Module *mod;
    Hook *hook;
    int ret,modstop=0;

    HASHMAP_FOREACH_ENTRY_VALUE(core_get_modules(), entry, mod) {
        LLIST_FOREACH_ENTRY(&mod->hooks, hook, list_head) {
            if (hook->hook_mask & hook_mask) {
                if ((hook_mask & HOOK_RUNHOOK) == 0)
                    RunHooks(HOOK_RUNHOOK, NULL, NULL, NULL, (char *[]){hook->name,hook->modname});
                ret = hook->ptr(nptr,uptr,chptr,parv);
                if (ret == MOD_STOP) modstop = 1;
            }
        }
    }

    if (modstop == 1)
        return MOD_STOP;
    else
        return MOD_CONTINUE;
}
