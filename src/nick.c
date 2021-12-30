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

struct nick {
    char name[NICKLEN + 1]; /* hash key */
    char ident[NICKLEN + 1];
    char host[HOSTLEN + 1];
    char uid[UIDLEN + 1];
    char svid[SVIDLEN + 1];
    char hiddenhost[HOSTLEN + 1];
    char reshost[HOSTLEN + 1];
    long int umodes;
    time_t msgtime;
    int msgnb;
    bool ignored;
    time_t ignoretime;
    int loginattempts;
    int lasttry;
    struct user *account;

    struct nick_llist_wrapper wrapper;
};

const char *nick_name(const Nick *nptr)
{
    return nptr->name;
}

void nick_set_name(Nick *nptr, const char *name)
{
    strncpy(nptr->name, name, NICKLEN);
    nptr->name[NICKLEN] = '\0';
}

const char *nick_ident(const Nick *nptr)
{
    return nptr->ident;
}

void nick_set_ident(Nick *nptr, const char *ident)
{
    strncpy(nptr->ident, ident, NICKLEN);
    nptr->ident[NICKLEN] = '\0';
}

const char *nick_host(const Nick *nptr)
{
    return nptr->host;
}

void nick_set_host(Nick *nptr, const char *host)
{
    strncpy(nptr->host, host, HOSTLEN);
    nptr->host[HOSTLEN] = '\0';
}

const char *nick_uid(const Nick *nptr)
{
    return nptr->uid;
}

const char *nick_svid(const Nick *nptr)
{
    return nptr->svid;
}

void nick_set_svid(Nick *nptr, const char *svid)
{
    strncpy(nptr->svid, svid, SVIDLEN);
    nptr->svid[SVIDLEN] = '\0';
}

const char *nick_hiddenhost(const Nick *nptr)
{
    return nptr->hiddenhost;
}

void nick_set_hiddenhost(Nick *nptr, const char *hiddenhost)
{
    strncpy(nptr->hiddenhost, hiddenhost, HOSTLEN);
    nptr->hiddenhost[HOSTLEN] = '\0';
}

const char *nick_reshost(const Nick *nptr)
{
    return nptr->reshost;
}

void nick_set_reshost(Nick *nptr, const char *reshost)
{
    strncpy(nptr->reshost, reshost, HOSTLEN);
    nptr->reshost[HOSTLEN] = '\0';
}

long int nick_umodes(const Nick *nptr)
{
    return nptr->umodes;
}

void nick_set_umodes(Nick *nptr, long int umodes)
{
    nptr->umodes = umodes;
}

time_t nick_msgtime(const Nick *nptr)
{
    return nptr->msgtime;
}

void nick_set_msgtime(Nick *nptr, time_t time)
{
    nptr->msgtime = time;
}

int nick_msgnb(const Nick *nptr)
{
    return nptr->msgnb;
}

void nick_set_msgnb(Nick *nptr, int msgnb)
{
    nptr->msgnb = msgnb;
}

bool nick_ignored(const Nick *nptr)
{
    return nptr->ignored;
}

void nick_set_ignored(Nick *nptr, bool ignored)
{
    nptr->ignored = ignored;
}

time_t nick_ignoretime(const Nick *nptr)
{
    return nptr->ignoretime;
}

void nick_set_ignoretime(Nick *nptr, time_t ignoretime)
{
    nptr->ignoretime = ignoretime;
}

int nick_loginattempts(const Nick *nptr)
{
    return nptr->loginattempts;
}

void nick_set_loginattempts(Nick *nptr, int loginattempts)
{
    nptr->loginattempts = loginattempts;
}

time_t nick_lasttry(const Nick *nptr)
{
    return nptr->lasttry;
}

void nick_set_lasttry(Nick *nptr, time_t lasttry)
{
    nptr->lasttry = lasttry;
}

struct user *nick_account(const Nick *nptr)
{
    return nptr->account;
}

void nick_set_account(Nick *nptr, User *account)
{
    nptr->account = account;
}

struct nick_llist_wrapper *nick_llist_wrapper(Nick *nptr)
{
    return &nptr->wrapper;
}

/** Look an account up from a Nick
 * @param   nick    pointer to a Nick
 * @returns an User struct
 * @note    This is an helper function to ease the migration to an account based
 *          paradigm.
 */
User *find_account(const Nick *nptr)
{
    return nptr->account ?: find_user(nptr->name);
}

static Nick *find_nick(const char *name_or_uid)
{
    Nick *tmp;
    struct hashmap_entry *entry;

    // TODO(target0): do a proper O(1) lookup.
    HASHMAP_FOREACH_ENTRY_VALUE(core_get_nicks(), entry, tmp) {
        if (!Strcmp(tmp->name, name_or_uid) || !Strcmp(tmp->uid, name_or_uid))
            return tmp;
    }

    return NULL;
}

static Nick *new_nick(char *nick, char *ident, char *host, char *uid, char *hiddenhost, long int umodes, char *reshost)
{
    Nick *new_nick;

    new_nick = malloc(sizeof(*new_nick));
    memset(new_nick, 0, sizeof(*new_nick));

    strncpy(new_nick->name, nick, NICKLEN);
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

    new_nick->wrapper.nick = new_nick;
    LLIST_INIT(&new_nick->wrapper.wchans);
    LLIST_INIT(&new_nick->wrapper.server_head);

    if (!HASHMAP_INSERT(core_get_nicks(), new_nick->name, new_nick, NULL)) {
        fprintf(stderr, "Failed to insert new nick \"%s\" into hashmap (duplicate entry?)\n", new_nick->name);
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

    if (!HASHMAP_ERASE(core_get_nicks(), nptr->name))
        return;

    if ((clone = find_clone(nptr->reshost)) != NULL) {
        clone->count--;
        if (clone->count == 0) {
            HASHMAP_ERASE(core_get_clones(), clone->host);
            free(clone);
        }
    }
    LLIST_REMOVE(&nptr->wrapper.server_head);
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
