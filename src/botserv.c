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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
USA.
*/


#include "botserv.h"

#include "mem.h"
#include "string_utils.h"

#include <stdlib.h>
#include <string.h>

extern botlist bot_list;
extern chanbotlist chanbot_list;

/* addBot function. It adds a new bot to be used with botserv to the database. It doesn't create the fakeuser.
 * params:
 *      nick: the nickname of the bot
 *      ident: its ident
 *      host: its host
 * return value: pointer to the new bot.
 */

Bot *addBot (char *nick, char *ident, char *host)
{
    Bot *newbot;
    newbot = (Bot *)malloc(sizeof(Bot));

    strncpy(newbot->nick,nick,NICKLEN);
    strncpy(newbot->ident,ident,NICKLEN);
    strncpy(newbot->host,host,HOSTLEN);

    LIST_INSERT_HEAD(bot_list, newbot, HASH(nick));

    return newbot;
}

/* addChanbot function. It adds a channel to be managed by botserv. It doesn't join the channel.
 * Note that channel option NOJOIN can't be changed unless the bot is unassigned.
 * params:
 *      name: the channel name. It must be already registered.
 *      bot: the bot name. It must already exist.
 * return value: pointer to the new botserv channel.
 */

Chanbot *addChanbot (char *name, char *bot)
{
    Chanbot *newchanbot;
    newchanbot = (Chanbot *)malloc(sizeof(Chanbot));

    strncpy(newchanbot->name,name,CHANLEN);
    strncpy(newchanbot->bot,bot,NICKLEN);

    LIST_INSERT_HEAD(chanbot_list, newchanbot, HASH(name));

    return newchanbot;
}

/* delBot function. It removes a bot from botserv database.
 * Note that it doesn't remove channels managed by the removed bot.
 * params:
 *      bot: the pointer to a bot struct, returned by find_bot function.
 * return value: none.
 */

void delBot (Bot *bot)
{
    LIST_REMOVE(bot_list, bot, HASH(bot->nick));
    free(bot);
}

/* delChanbot function. It removes a channel managed by a botserv from the database.
 * It *DOESN'T* remove the channel from the registered channels database.
 * params:
 *      chanbot: pointer to chanbot struct returned by find_chanbot function.
 * return value: none.
 */

void delChanbot (Chanbot *chanbot)
{
    LIST_REMOVE(chanbot_list, chanbot, HASH(chanbot->name));
    free(chanbot);
}

/* find_bot function. It searches a bot in the botserv database.
 * params:
 *      nick: nickname of the bot to find.
 * return value: If found, pointer to the searched bot. Otherwise, NULL pointer.
 */

Bot *find_bot (char *nick)
{
    Bot *tmp;
    LIST_FOREACH(bot_list, tmp, HASH(nick)) {
        if (!Strcmp(tmp->nick,nick))
            return tmp;
    }

    return NULL;
}

/* find_chanbot function. It searches a channel managed by botserv in the database.
 * params:
 *      name: the name of the channel to find.
 * return value: If found, pointer to the botserv channel. Otherwise, NULL pointer.
 */

Chanbot *find_chanbot (char *name)
{
    Chanbot *tmp;
    LIST_FOREACH(chanbot_list, tmp, HASH(name)) {
        if (!Strcmp(tmp->name, name))
            return tmp;
    }

    return NULL;
}
