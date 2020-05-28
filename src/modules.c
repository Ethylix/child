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


#include "modules.h"

#include "channel.h"
#include "mem.h"
#include "string_utils.h"
#include "user.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern hooklist hook_list;
extern modulelist module_list;

Module *find_module (char *name)
{
    Module *tmp;
    LIST_FOREACH(module_list, tmp, HASH(name)) {
        if (!Strcmp(tmp->modname,name))
            return tmp;
    }

    return NULL;
}

Hook *find_hook (char *name, char *modname)
{
    Hook *tmp;
    LIST_FOREACH(hook_list, tmp, HASH(modname)) {
        if (!Strcmp(tmp->name,name) && !Strcmp(tmp->modname,modname))
            return tmp;
    }

    return NULL;
}

void unloadallmod()
{
    Module *mod,*next;
    for (mod = LIST_HEAD(module_list); mod; mod = next) {
        next = LIST_LNEXT(mod);
        unloadmodule(mod->modname);
    }
}

Module *loadmodule(char *name)
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
    strncpy(mod->modname,name,50);
    mod->modname[50] = '\0';
    mod->handle = handle; /* saving module handle */
    mod->nodreload = 0;

    LIST_INSERT_HEAD(module_list, mod, HASH(mod->modname));

    func(mod);

    return mod;
}

int unloadmodule(char *name)
{
    Module *mod;
    Hook *hook, *next;;

    mod = find_module(name);
    if (!mod) return 0;

    for (hook = hook_list.table[HASH(name)]; hook; hook = next) {
        next = hook->next;
        if (!Strcmp(hook->modname,name))
            DelHook(hook->name,hook->modname);
    }

    void (*func)();
    const char *err = NULL;
    func = dlsym(mod->handle,"child_cleanup"); /* retrieving child_cleanup() function */
    if (((err = dlerror()) != NULL) && !func)
        fprintf(stderr,"ERROR UNLOADING MODULE %s: %s\n",name,err);
    else
        func(); /* if it exists, execute it. */

    dlclose(mod->handle); /* closing module handle */

    LIST_REMOVE(module_list, mod, HASH(mod->modname));
    free(mod);

    return 1;
}

Hook *AddHook(long int what, int (*fptr)(Nick *, User *, Chan *, char *[]), char *name, char *modname)
{
    Hook *hook;
    if (find_hook(name,modname)) {
        fprintf(stderr,"Warning: Cannot add hook %s: hook alreay exists\n",name);
        return NULL;
    }

    hook = (Hook *)malloc(sizeof(Hook));
    strncpy(hook->name,name,50);
    strncpy(hook->modname,modname,50);
    hook->ptr = fptr;
    hook->hook = what;

    LIST_INSERT_HEAD(hook_list, hook, HASH(modname));

    return hook;
}

int DelHook(char *name, char *modname)
{
    Hook *hook = find_hook(name,modname);
    if (!hook) return 0;

    LIST_REMOVE(hook_list, hook, HASH(modname));
    free(hook);

    return 1;
}

int RunHooks(long int what, Nick *nptr, User *uptr, Chan *chptr, char *parv[])
{
    Hook *hook;
    int ret,modstop=0;

    LIST_FOREACH_ALL(hook_list, hook) {
        if (hook->hook & what) {
            ret = hook->ptr(nptr,uptr,chptr,parv);
            if (ret == MOD_STOP) modstop = 1;
        }
    }

    if (modstop == 1)
        return MOD_STOP;
    else
        return MOD_CONTINUE;
}
