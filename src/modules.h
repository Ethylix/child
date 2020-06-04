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


#ifndef _MODULES_H
#define _MODULES_H

#include "channel.h"
#include "user.h"

#ifndef __unused
#define __unused __attribute__((unused))
#endif

#define HOOK_JOIN           0x0001
/* nptr  : sender
 * uptr  : sender's account (if exists)
 * chptr : channel joined
 * parv  : NULL
 */

#define HOOK_NICKCHANGE     0x0002
/* nptr    : new nick
 * uptr    : new nick's account (if exists)
 * chptr   : NULL
 * parv[0] : old nick (string)
 */

#define HOOK_NICKCREATE     0x0004
/* nptr  : nick
 * uptr  : nick's account (if exists)
 * chptr : NULL
 * parv  : NULL
 */

#define HOOK_PING           0x0008
/* nptr  : NULL
 * uptr  : NULL
 * chptr : NULL
 * parv  : NULL
 */

#define HOOK_QUIT           0x0010
/* nptr  : sender
 * uptr  : sender's account (if exists)
 * chptr : NULL
 * parv  : NULL
 *
 * NOTE: This hook runs before the nick is removed
 */

#define HOOK_UMODE          0x0020
/* nptr    : target
 * uptr    : target's account (if exists)
 * chptr   : NULL
 * parv[0] : new umodes
 */

#define HOOK_PRE_PRIVMSG    0x0080
/* nptr    : sender
 * uptr    : sender's account (if exists)
 * chptr   : channel, only if the target is a channel...
 * parv[0] : target name (nick or channel)
 * parv[1] : message
 * XXX DEPRECATED XXX (use *Command() instead)
 */

#define HOOK_PRIVMSG        0x0100
/* same as HOOK_PRE_PRIVMSG, but this is run after the commands parsing
 *
 * NOTE: If you want add a "nick", "chan", "oper" or "host" command,
 *       use HOOK_PRE_PRIVMSG instead. Otherwise, your command will
 *       never be executed.
 * XXX DEPRECATED XXX (use *Command() instead)
 */

#define HOOK_RAW            0x0200
/* nptr    : NULL
 * uptr    : NULL
 * chptr   : NULL
 * parv[0] : sender
 * parv[1] : command
 * parv[2] : tail
 *
 * Raw example :
 *  :target0 PRIVMSG #opers :moo
 *
 * parv[0] = ":target0"
 * parv[1] = "PRIVMSG"
 * parv[2] = "#opers :moo"
 */

#define HOOK_PRE_JOIN       0x0400
#define HOOK_KILL           0x0800
#define HOOK_LOADDB         0x1000
#define HOOK_SAVEDB         0x2000
#define HOOK_CONNECTED      0x4000
// next hook value = 0x40000 (1 << 18)

#define MOD_STOP        0
#define MOD_CONTINUE    1

typedef struct hook {
    char name[50+1];
    char modname[50+1]; /* hash key */
    long int hook;
    int (*ptr)(Nick *, User *, Chan *, char *[]);
    struct hook *next,*prev;
    struct hook *lnext,*lprev;
} Hook;

typedef struct module_ {
    char modname[50+1]; /* hash key */
    void *handle;
    int nodreload;
    struct module_ *next,*prev;
    struct module_ *lnext,*lprev;
} Module;

typedef struct {
    int size;
    TABLE(Hook);
    Hook *lhead;
} hooklist;

Module *find_module (char *);
Hook *find_hook (char *, char *);
Module *loadmodule (char *);
int unloadmodule (char *);
Hook *AddHook (long int, int (*fptr)(Nick *, User *, Chan *, char **), char *, char *);
int DelHook (char *, char *);
int RunHooks (long int, Nick *, User *, Chan *, char *[]);
void unloadallmod (void);

#endif
