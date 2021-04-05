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


#include "config.h"
#include "child.h"
#include "core.h"
#include "commands.h"
#include "modules.h"

#include <stdio.h>
#include <string.h>

void loadconf(int what)
{
    FILE *config_file; /* config file */
    char line[1024]; /* one config line */
    char module[2048]; /* module to load */
    char cmddata[512];

    /* first we open the config file */
    config_file = fopen(DEFAULT_CONFFILE,"r");
    if (!config_file) {
        fprintf(stderr,"Configuration file not found !\n");
        if (!what)
            exit(EXIT_FAILURE);
    }

    /* then we read one line at a time and search for values */
    if (get_core()->verbose) printf("Reading configuration\n");
    while(!feof(config_file)) {
        if (!fgets(line,1023,config_file)) break;
        if (sscanf(line,"ServerName \"%[^\"]\"",core_get_config()->name)) { if (get_core()->vv) printf("\tname = '%s'\n",core_get_config()->name); continue; }
        if (sscanf(line,"ServerID \"%[^\"]\"",core_get_config()->sid)) { if (get_core()->vv) printf("\tsid = '%s'\n",core_get_config()->sid); continue; }
        if (sscanf(line,"BotNick \"%[^\"]\"",core_get_config()->nick)) { if (get_core()->vv) printf("\tnick = '%s'\n",core_get_config()->nick); continue; }
        if (sscanf(line,"BotIdent \"%[^\"]\"",core_get_config()->ident)) { if (get_core()->vv) printf("\tident = '%s'\n",core_get_config()->ident); continue; }
        if (sscanf(line,"BotHost \"%[^\"]\"",core_get_config()->host)) { if (get_core()->vv) printf("\thost = '%s'\n",core_get_config()->host); continue; }
        if (sscanf(line,"RemoteServer %s",core_get_config()->server)) { if (get_core()->vv) printf("\tserver = '%s'\n",core_get_config()->server); continue; }
        if (sscanf(line,"Port %i",&core_get_config()->port)) { if (get_core()->vv) printf("\tport = '%i'\n",core_get_config()->port); continue; }
        if (sscanf(line,"LinkPass \"%[^\"]\"",core_get_config()->linkpass))  { if (get_core()->vv) printf("\tpass = '%s'\n",core_get_config()->linkpass); continue; }
        if (sscanf(line,"MaxClones %i",&core_get_config()->maxclones))  { if (get_core()->vv) printf("\tmaxclones = '%i'\n",core_get_config()->maxclones); continue; }
        if (sscanf(line,"NickExpire %i",&core_get_config()->nick_expire))  { if (get_core()->vv) printf("\tnick_expire = '%i'\n",core_get_config()->nick_expire); continue; }
        if (sscanf(line,"ChanExpire %i",&core_get_config()->chan_expire))  { if (get_core()->vv) printf("\tchan_expire = '%i'\n",core_get_config()->chan_expire); continue; }
        if (sscanf(line,"MaxChanPerUser %i",&core_get_config()->chanperuser)) { if (get_core()->vv) printf("\tchanperuser = '%i'\n",core_get_config()->chanperuser); continue; }
        if (sscanf(line,"LevelOper %i",&core_get_config()->level_oper))  { if (get_core()->vv) printf("\tlevel_oper = '%i'\n",core_get_config()->level_oper); continue; }
        if (sscanf(line,"LevelAdmin %i",&core_get_config()->level_admin))  { if (get_core()->vv) printf("\tlevel_admin = '%i'\n",core_get_config()->level_admin); continue; }
        if (sscanf(line,"LevelRoot %i",&core_get_config()->level_root))  { if (get_core()->vv) printf("\tlevel_root = '%i'\n",core_get_config()->level_root); continue; }
        if (sscanf(line,"LevelOwner %i",&core_get_config()->level_owner))  { if (get_core()->vv) printf("\tlevel_owner = '%i'\n",core_get_config()->level_owner); continue; }
        bzero(module,2048);
        if (sscanf(line,"LoadMod \"%[^\"]\"",module)) {
            char *tmod;
            tmod = strtok(module," ");
            while (tmod) {
                if (get_core()->vv)
                    printf("\tloading module '%s'\n",tmod);
                if (what)
                    unloadmodule(tmod);
                loadmodule(tmod);
                tmod = strtok(NULL," ");
            }
            continue;
        }
        bzero(cmddata,512);
        if (sscanf(line,"SetCmdLev \"%[^\"]\"",cmddata)) {
            if (get_core()->vv)
                printf("\tchanging command level '%s'\n",cmddata);
            setcmdlev2(cmddata);
            continue;
        }
        if (sscanf(line,"MysqlHost \"%[^\"]\"",core_get_config()->mysql_host)) { if (get_core()->vv) printf("\tmysql_host = '%s'\n",core_get_config()->mysql_host); continue; }
        if (sscanf(line,"MysqlDB \"%[^\"]\"",core_get_config()->mysql_db)) { if (get_core()->vv) printf("\tmysql_db = '%s'\n",core_get_config()->mysql_db); continue; }
        if (sscanf(line,"MysqlUser \"%[^\"]\"",core_get_config()->mysql_login)) { if (get_core()->vv) printf("\tmysql_login = '%s'\n",core_get_config()->mysql_login); continue; }
        if (sscanf(line,"MysqlPass \"%[^\"]\"",core_get_config()->mysql_passwd)) { if (get_core()->vv) printf("\tmysql_passwd = '%s'\n",core_get_config()->mysql_passwd); continue; }
        if (sscanf(line,"LogFile \"%[^\"]\"",core_get_config()->logfile)) { if (get_core()->vv) printf("\tlogfile = '%s'\n",core_get_config()->logfile); continue; }
        if (sscanf(line,"LimitTime %i",&core_get_config()->limittime)) { if (get_core()->vv) printf("\tlimittime = '%i'\n",core_get_config()->limittime); continue; }
        if (sscanf(line,"LocalAddr %s",core_get_config()->bindip)) { if (get_core()->vv) printf("\tbindip = '%s'\n",core_get_config()->bindip); continue; }
        if (sscanf(line,"SaveDB %i",&core_get_config()->savedb_interval)) { if (get_core()->vv) printf("\tsavedb_interval = '%i'\n",core_get_config()->savedb_interval); continue; }
        if (sscanf(line,"GuestPrefix \"%[^\"]\"",core_get_config()->guest_prefix)) { if (get_core()->vv) printf("\tguest_prefix = '%s'\n",core_get_config()->guest_prefix); continue; }
        if (sscanf(line,"AnonGlobal %i",&core_get_config()->anonymous_global)) { if (get_core()->vv) printf("\tanonymous_global = '%i'\n",core_get_config()->anonymous_global); continue; }
        if (sscanf(line,"SendMailPath \"%[^\"]\"",core_get_config()->sendmail)) { if (get_core()->vv) printf("\tsendmail = '%s'\n",core_get_config()->sendmail); continue; }
        if (sscanf(line,"SendFrom \"%[^\"]\"",core_get_config()->sendfrom)) { if (get_core()->vv) printf("\tsendfrom = '%s'\n",core_get_config()->sendfrom); continue; }
        if (sscanf(line,"UserCloak \"%[^\"]\"",core_get_config()->usercloak)) { if (get_core()->vv) printf("\tusercloak = '%s'\n",core_get_config()->usercloak); continue; }
        if (sscanf(line,"MessageFlood %i %i",&core_get_config()->maxmsgnb,&core_get_config()->maxmsgtime)) { if (get_core()->vv) printf("\tmaxmsgnb = '%i'\n\tmaxmsgtime = '%i'\n",core_get_config()->maxmsgnb,core_get_config()->maxmsgtime); continue; }
        if (sscanf(line,"IgnoreTime %i",&core_get_config()->ignoretime)) { if (get_core()->vv) printf("\tignoretime = '%i'\n",core_get_config()->ignoretime); continue; }
        if (sscanf(line,"MaxLoginAttempts %i",&core_get_config()->maxloginatt)) { if (get_core()->vv) printf("\tmaxloginatt = '%i'\n",core_get_config()->maxloginatt); continue; }
        if (sscanf(line,"ChanLevSAdmin %i",&core_get_config()->chlev_sadmin)) { if (get_core()->vv) printf("\tchlev_sadmin = '%i'\n",core_get_config()->chlev_sadmin); continue; }
        if (sscanf(line,"ChanLevAdmin %i",&core_get_config()->chlev_admin)) { if (get_core()->vv) printf("\tchlev_admin = '%i'\n",core_get_config()->chlev_admin); continue; }
        if (sscanf(line,"ChanLevOp %i",&core_get_config()->chlev_op)) { if (get_core()->vv) printf("\tchlev_op = '%i'\n",core_get_config()->chlev_op); continue; }
        if (sscanf(line,"ChanLevHalfop %i",&core_get_config()->chlev_halfop)) { if (get_core()->vv) printf("\tchlev_halfop = '%i'\n",core_get_config()->chlev_halfop); continue; }
        if (sscanf(line,"ChanLeget_core()->vvoice %i",&core_get_config()->chlev_voice)) { if (get_core()->vv) printf("\tchlev_voice = '%i'\n",core_get_config()->chlev_voice); continue; }
        if (sscanf(line,"ChanLevInvite %i",&core_get_config()->chlev_invite)) { if (get_core()->vv) printf("\tchlev_invite = '%i'\n",core_get_config()->chlev_invite); continue; }
        if (sscanf(line,"ChanLevNoStatus %i",&core_get_config()->chlev_nostatus)) { if (get_core()->vv) printf("\tchlev_nostatus = '%i'\n",core_get_config()->chlev_nostatus); continue; }
        if (sscanf(line,"ChanLevAKick %i",&core_get_config()->chlev_akick)) { if (get_core()->vv) printf("\tchlev_akick = '%i'\n",core_get_config()->chlev_akick); continue; }
        if (sscanf(line,"ChanLevAKickBan %i",&core_get_config()->chlev_akb)) { if (get_core()->vv) printf("\tchlev_akb = '%i'\n",core_get_config()->chlev_akb); continue; }
        if (sscanf(line,"EmailReg %i",&core_get_config()->emailreg)) { if (get_core()->vv) printf("\temailreg = '%i'\n",core_get_config()->emailreg); continue; }

        if (get_core()->vv) printf("\tUNKNOWN : %s",line);
    }

    fclose(config_file); 
    if (get_core()->verbose) printf("Configuration loaded\n");
}
