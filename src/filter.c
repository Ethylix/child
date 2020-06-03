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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
USA.
*/

#include "filter.h"

#include "config.h"
#include "mem.h"
#include "string_utils.h"
#include "user.h"

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern rulelist rule_list;

extern int verbose, vv;

#ifdef USE_FILTER

#define strempty(x) (!(x) || (*(x) == '\0'))
#define flush_rules() { \
                        while (!LIST_EMPTY(rule_list)) \
                            remove_rule(LIST_HEAD(rule_list)); \
                      }

/* rule format: drop|pass in|out [quick] [from nick] [to nick] [action name] [data "c-regexp-stuff"] */

struct ruleset *add_rule (char *ruledata)
{
    struct ruleset ruleset;
    memset(&ruleset, 0, sizeof(ruleset));

    char *what, *direction, *stuff;
    char temp[1024];
    strncpy(temp, ruledata, 1023);
    what = temp;
    direction = SeperateWord(what);
    stuff = SeperateWord(direction);

    if (strempty(what) || strempty(direction))
        return NULL;

    if (!strcmp(what,"drop"))
        ruleset.rule = RULE_DROP;
    else if (!strcmp(what,"pass"))
        ruleset.rule = RULE_PASS;
    else
        return NULL;
    
    if (!strcmp(direction,"in"))
        ruleset.direction = DIRECT_IN;
    else if (!strcmp(direction,"out"))
        ruleset.direction = DIRECT_OUT;
    else
        return NULL;

    char *param, *data;
    param = stuff;
    stuff = SeperateWord(param);

    if (!strcmp(param, "quick")) {
        ruleset.quick = 1;
        param = stuff;
        stuff = SeperateWord(param);
    }

    for (; stuff; param = stuff, stuff = SeperateWord(stuff)) {
        if (strcmp(param, "data")) { /* not data */
            data = stuff;
            stuff = SeperateWord(data);
            if (strempty(data))
                return NULL;
            if (!strcmp(param, "from"))
                strncpy(ruleset.from, data, 32);
            else if (!strcmp(param, "to")) {
                if (*data == '\\' && *(data+1) == '#')
                    data++;
                strncpy(ruleset.to, data, 32);
            } else if (!strcmp(param, "action"))
                strncpy(ruleset.action, data, 32);
            else
                return NULL;
        } else {
            data = strchr(stuff, '"');
            if (strempty(data))
                return NULL;
            data++;
            char *tmp = strchr(data, '"');
            if (strempty(tmp))
                return NULL;
            *tmp = '\0';
            if (*(tmp+1) == 'i')
                ruleset.icase = 1;
            stuff = StripBlanks(tmp+1);
            strncpy(ruleset.data, data, 1024);
        }
    }

    struct ruleset *new_rule;
    new_rule = (struct ruleset *)calloc(1, sizeof(struct ruleset));
    memcpy(new_rule, &ruleset, sizeof(ruleset));
    LIST_INSERT_HEAD(rule_list, new_rule, HASH_INT(new_rule->direction));
    if (rule_list.size == 1)
        rule_list.ltail = new_rule;

    return new_rule;
}

void remove_rule (struct ruleset *rule)
{
    LIST_REMOVE(rule_list, rule, HASH_INT(rule->direction));
    if (rule_list.ltail == rule)
        rule_list.ltail = rule->lprev;
    free(rule);
}

int match_rules (struct ruleset *rule, struct ruleset *packet)
{
    int param = REG_EXTENDED|REG_NOSUB;

    if (rule->rule != packet->rule || rule->direction != packet->direction)
        return 0;

    if (!strempty(rule->from)) {
        if (Strcmp(rule->from, packet->from))
            return 0;
    }

    if (!strempty(rule->to)) {
        if (Strcmp(rule->to, packet->to))
            return 0;
    }

    if (!strempty(rule->action)) {
        if (Strcmp(rule->action, packet->action))
            return 0;
    }

    if (!strempty(rule->data)) {
        if (rule->icase)
            param |= REG_ICASE;
        if (!__match_regex(rule->data, packet->data, param))
            return 0;
    }

    return 1;
}

int filter_check (char *line, int direction)
{
    if (rule_list.enabled == 0)
        return -1;

    struct ruleset *rule, *last = NULL, packet;
    memset(&packet, 0, sizeof(packet));

    char *from, *action, *to, *data;
    char temp[1024];
    strncpy(temp, line, 1023);

    from = temp;
    action = SeperateWord(from);
    to = SeperateWord(action);
    data = SeperateWord(to);

    if (strempty(from))
        return -1;

    if (*from == ':')
        from++;

    packet.direction = direction;
    strncpy(packet.from, from, NICKLEN);
    if (action)
        strncpy(packet.action, action, 128);
    if (to)
        strncpy(packet.to, to, NICKLEN);
    if (data) {
        if (*data == ':')
            data++;
        strncpy(packet.data, data, 1023);
    }

    for (rule = rule_list.ltail; rule; rule = rule->lprev) {
        packet.rule = rule->rule;
        if ((match_rules(rule, &packet)) == 1) {
            last = rule;
            if (rule->quick == 1)
                return rule->rule;
        }
    }

    if (last)
        return last->rule;

    return -1;
}

int loadrulefile()
{
    FILE *fp;
    char line[1024];
    char *tmp;
    int failed = 0;

    flush_rules();

    fp = fopen(DEFAULT_RULESFILE, "r");
    if (!fp) {
        fprintf(stderr,"Rules file not found !\n");
        return 0;
    }

    if (verbose) printf("Loading filter rules\n");
    while (!feof(fp)) {
        if (!fgets(line, 1023, fp)) break;
        if ((tmp = strchr(line, '\r')) != NULL)
            *tmp = '\0';
        if ((tmp = strchr(line, '\n')) != NULL)
            *tmp = '\0';
        if (line[0] == '#' || line[0] == '\0')
            continue;
        if ((add_rule(line)) == NULL) {
            fprintf(stderr,"Cannot add rule `%s'\n",line);
            failed = 1;
            break;
        }
        else {
            if (vv)
                printf("%s\n",line);
        }
    }

    fclose(fp);

    if (failed) {
        fprintf(stderr, "Rules loading cancelled, flushing\n");
        flush_rules();
        return 0;
    }

    if (verbose) printf("Rules loaded\n");
    return 1;
}

#endif
