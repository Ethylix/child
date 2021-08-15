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

#include "base64.h"
#include "channel.h"
#include "child.h"
#include "commands.h"
#include "core.h"
#include "core_api.h"
#include "hashmap.h"
#include "logging.h"
#include "mem.h"
#include "net.h"
#include "modules.h"
#include "string_utils.h"
#include "user.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

int sasl_start_session (Nick *, User *, Chan *, char **);
int sasl_finish_session (Nick *, User *, Chan *, char **);

void child_init()
{
    AddHook(HOOK_SASL,&sasl_start_session,"sasl_start_session","sasl");
    AddHook(HOOK_NICKCREATE,&sasl_finish_session,"sasl_finish_session","sasl");
}

void child_cleanup()
{}

/** Starts an SASL session
 * An SASL session works in two steps:
 * - sasl_start_session: AUTHENTICATE during registration, this delegates the authentication to services and sets the SVID on the irc
 * - sasl_finish_session: Post-registration. Child will receive the UID command in which the SVID will be set. When Child gets it, it can enforces the different nick options set on the account.
 * 
 * References:
 * - https://gist.github.com/grawity/8389307
 * - https://ircv3.net/specs/extensions/sasl-3.1.html
 */
int sasl_start_session (__unused Nick *nptr, __unused User *uptr, __unused Chan *cptr, char *parv[])
{
    char *sender = parv[0] + 1; // remove leading :
    char *split[10];
    int i = 0;
    User *user;

    char *pch = strtok(parv[1], " ");

    while (pch != NULL && i < 10) {
        split[i] = pch;
        pch = strtok(NULL, " ");
        i++;
    }

    // If we didn't parse that many arguments, the request is invalid.
    // To the client, it'll appear as an SASL authentication time out.
    if (i < 4)
        return MOD_CONTINUE;

    char *target = split[0];
    char *uid = split[1];
    char *command = split[2]; // H/S/C/D
    char *tail = split[3];
    int ret;
    char *authcid, *password;

    char decoded[401];
    memset(decoded, 0, 401);

    // parse
    if (strcmp(command, "S") == 0) {
        // We only support PLAIN for now
        if (strcmp(tail, "PLAIN")) {
            get_core_api()->send_raw(":%s SASL %s %s M PLAIN", target, sender, uid);
            get_core_api()->send_raw(":%s SASL %s %s D F", target, sender, uid);
            return MOD_CONTINUE;
        }
        get_core_api()->send_raw(":%s SASL %s %s C +", target, sender, uid);
    } else if (strcmp(command, "C") == 0) {
        ret = b64_decode(split[3], decoded, 400);
        if (ret < 0) {
            get_core_api()->send_raw(":%s SASL %s %s D F", target, sender, uid);
            return MOD_CONTINUE;
        }

        authcid = memchr(decoded, '\0', ret);
        if (authcid == NULL) {
            get_core_api()->send_raw(":%s SASL %s %s D F", target, sender, uid);
            return MOD_CONTINUE;
        }
        authcid++;

        password = memchr(authcid, '\0', ret);
        if (password == NULL) {
            get_core_api()->send_raw(":%s SASL %s %s D F", target, sender, uid);
            return MOD_CONTINUE;
        }
        password++;

        // check that user exists and password matches
        user = find_user(authcid);
        if (!user) {
            get_core_api()->send_raw(":%s SASL %s %s D F", target, sender, uid);
            return MOD_CONTINUE;
        } 

        char *md5pass = md5_hash(password);
        if (Strcmp(md5pass,user->md5_pass)) {
            // wrong password
            get_core_api()->send_raw(":%s SASL %s %s D F", target, sender, uid);
            return MOD_CONTINUE;
        }
        free(md5pass);

        // TODO:    create a helper function to log an user in and use it here
        //          the function should also be used in nick_identify/nick_register
        get_core_api()->send_raw(":%s SVSLOGIN %s %s %s", target, sender, uid, authcid);
        get_core_api()->send_raw(":%s SASL %s %s D S", target, sender, uid);
    }

    return MOD_CONTINUE;
}

/** Enforces user settings and sets svid on the nick 
 * This is the second and last step of the SASL session: Child gets an UID containing the SVID from the IRCD. 
 * It then enforces everything as is set on the User: vhosts, +r, ...
 */
int sasl_finish_session (Nick *nptr, User *uptr, __unused Chan *cptr, __unused char *parv[]) {
    if (!Strcmp(nptr->svid, "0"))
        return MOD_CONTINUE;

    if (!uptr)
        uptr = find_user(nptr->svid);

    // TODO: refactor, coming from nick_identify()
    NoticeToUser(nptr,"You are now identified");
    nptr->loginattempts = 0;
    nptr->lasttry = 0;

    if (uptr->email[0] == '\0')
        NoticeToUser(nptr,"Please set a valid email address with /msg C nick set email email@domain.xx");

    uptr->authed = 1;
    uptr->lastseen = time(NULL);
    get_core_api()->send_raw("SVS2MODE %s +r",nptr->nick);
    SetUmode(nptr, UMODE_REGISTERED);

    if (uptr->vhost[0] != '\0') {
        get_core_api()->send_raw("CHGHOST %s %s",nptr->nick,uptr->vhost);
        strncpy(nptr->hiddenhost, uptr->vhost, HOSTLEN);
        NoticeToUser(nptr,"Your vhost \2%s\2 has been activated",uptr->vhost);
    } else if (HasOption(uptr, UOPT_CLOAKED)) {
        get_core_api()->send_raw("CHGHOST %s %s%s", nptr->nick, uptr->nick, core_get_config()->usercloak);
        char host[HOSTLEN + NICKLEN + 1];
        bzero(host, HOSTLEN+NICKLEN+1);
        snprintf(host, HOSTLEN + NICKLEN + 1, "%s%s", uptr->nick, core_get_config()->usercloak);
        strncpy(nptr->hiddenhost, host, HOSTLEN);
        NoticeToUser(nptr,"Your cloak has been activated");
    }

    return MOD_CONTINUE;
}
