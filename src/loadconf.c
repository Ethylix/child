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
            child_clean();
    }

    /* then we read one line at a time and search for values */
    if (get_core()->verbose) printf("Reading configuration\n");
    while(!feof(config_file)) {
        if (!fgets(line,1023,config_file)) break;
        if (sscanf(line,"ServerName \"%[^\"]\"",me.name)) { if (get_core()->vv) printf("\tname = '%s'\n",me.name); continue; }
        if (sscanf(line,"ServerID \"%[^\"]\"",me.sid)) { if (get_core()->vv) printf("\tsid = '%s'\n",me.sid); continue; }
        if (sscanf(line,"BotNick \"%[^\"]\"",me.nick)) { if (get_core()->vv) printf("\tnick = '%s'\n",me.nick); continue; }
        if (sscanf(line,"BotIdent \"%[^\"]\"",me.ident)) { if (get_core()->vv) printf("\tident = '%s'\n",me.ident); continue; }
        if (sscanf(line,"BotHost \"%[^\"]\"",me.host)) { if (get_core()->vv) printf("\thost = '%s'\n",me.host); continue; }
        if (sscanf(line,"RemoteServer %s",me.server)) { if (get_core()->vv) printf("\tserver = '%s'\n",me.server); continue; }
        if (sscanf(line,"Port %i",&me.port)) { if (get_core()->vv) printf("\tport = '%i'\n",me.port); continue; }
        if (sscanf(line,"LinkPass \"%[^\"]\"",me.linkpass))  { if (get_core()->vv) printf("\tpass = '%s'\n",me.linkpass); continue; }
        if (sscanf(line,"MaxClones %i",&me.maxclones))  { if (get_core()->vv) printf("\tmaxclones = '%i'\n",me.maxclones); continue; }
        if (sscanf(line,"NickExpire %i",&me.nick_expire))  { if (get_core()->vv) printf("\tnick_expire = '%i'\n",me.nick_expire); continue; }
        if (sscanf(line,"ChanExpire %i",&me.chan_expire))  { if (get_core()->vv) printf("\tchan_expire = '%i'\n",me.chan_expire); continue; }
        if (sscanf(line,"MaxChanPerUser %i",&me.chanperuser)) { if (get_core()->vv) printf("\tchanperuser = '%i'\n",me.chanperuser); continue; }
        if (sscanf(line,"LevelOper %i",&me.level_oper))  { if (get_core()->vv) printf("\tlevel_oper = '%i'\n",me.level_oper); continue; }
        if (sscanf(line,"LevelAdmin %i",&me.level_admin))  { if (get_core()->vv) printf("\tlevel_admin = '%i'\n",me.level_admin); continue; }
        if (sscanf(line,"LevelRoot %i",&me.level_root))  { if (get_core()->vv) printf("\tlevel_root = '%i'\n",me.level_root); continue; }
        if (sscanf(line,"LevelOwner %i",&me.level_owner))  { if (get_core()->vv) printf("\tlevel_owner = '%i'\n",me.level_owner); continue; }
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
        if (sscanf(line,"MysqlHost \"%[^\"]\"",me.mysql_host)) { if (get_core()->vv) printf("\tmysql_host = '%s'\n",me.mysql_host); continue; }
        if (sscanf(line,"MysqlDB \"%[^\"]\"",me.mysql_db)) { if (get_core()->vv) printf("\tmysql_db = '%s'\n",me.mysql_db); continue; }
        if (sscanf(line,"MysqlUser \"%[^\"]\"",me.mysql_login)) { if (get_core()->vv) printf("\tmysql_login = '%s'\n",me.mysql_login); continue; }
        if (sscanf(line,"MysqlPass \"%[^\"]\"",me.mysql_passwd)) { if (get_core()->vv) printf("\tmysql_passwd = '%s'\n",me.mysql_passwd); continue; }
        if (sscanf(line,"LogFile \"%[^\"]\"",me.logfile)) { if (get_core()->vv) printf("\tlogfile = '%s'\n",me.logfile); continue; }
        if (sscanf(line,"LimitTime %i",&me.limittime)) { if (get_core()->vv) printf("\tlimittime = '%i'\n",me.limittime); continue; }
        if (sscanf(line,"LocalAddr %s",me.bindip)) { if (get_core()->vv) printf("\tbindip = '%s'\n",me.bindip); continue; }
        if (sscanf(line,"SaveDB %i",&me.savedb_interval)) { if (get_core()->vv) printf("\tsavedb_interval = '%i'\n",me.savedb_interval); continue; }
        if (sscanf(line,"GuestPrefix \"%[^\"]\"",me.guest_prefix)) { if (get_core()->vv) printf("\tguest_prefix = '%s'\n",me.guest_prefix); continue; }
        if (sscanf(line,"AnonGlobal %i",&me.anonymous_global)) { if (get_core()->vv) printf("\tanonymous_global = '%i'\n",me.anonymous_global); continue; }
        if (sscanf(line,"SendMailPath \"%[^\"]\"",me.sendmail)) { if (get_core()->vv) printf("\tsendmail = '%s'\n",me.sendmail); continue; }
        if (sscanf(line,"SendFrom \"%[^\"]\"",me.sendfrom)) { if (get_core()->vv) printf("\tsendfrom = '%s'\n",me.sendfrom); continue; }
        if (sscanf(line,"UserCloak \"%[^\"]\"",me.usercloak)) { if (get_core()->vv) printf("\tusercloak = '%s'\n",me.usercloak); continue; }
        if (sscanf(line,"MessageFlood %i %i",&me.maxmsgnb,&me.maxmsgtime)) { if (get_core()->vv) printf("\tmaxmsgnb = '%i'\n\tmaxmsgtime = '%i'\n",me.maxmsgnb,me.maxmsgtime); continue; }
        if (sscanf(line,"IgnoreTime %i",&me.ignoretime)) { if (get_core()->vv) printf("\tignoretime = '%i'\n",me.ignoretime); continue; }
        if (sscanf(line,"MaxLoginAttempts %i",&me.maxloginatt)) { if (get_core()->vv) printf("\tmaxloginatt = '%i'\n",me.maxloginatt); continue; }
        if (sscanf(line,"ChanLevSAdmin %i",&me.chlev_sadmin)) { if (get_core()->vv) printf("\tchlev_sadmin = '%i'\n",me.chlev_sadmin); continue; }
        if (sscanf(line,"ChanLevAdmin %i",&me.chlev_admin)) { if (get_core()->vv) printf("\tchlev_admin = '%i'\n",me.chlev_admin); continue; }
        if (sscanf(line,"ChanLevOp %i",&me.chlev_op)) { if (get_core()->vv) printf("\tchlev_op = '%i'\n",me.chlev_op); continue; }
        if (sscanf(line,"ChanLevHalfop %i",&me.chlev_halfop)) { if (get_core()->vv) printf("\tchlev_halfop = '%i'\n",me.chlev_halfop); continue; }
        if (sscanf(line,"ChanLeget_core()->vvoice %i",&me.chlev_voice)) { if (get_core()->vv) printf("\tchlev_voice = '%i'\n",me.chlev_voice); continue; }
        if (sscanf(line,"ChanLevInvite %i",&me.chlev_invite)) { if (get_core()->vv) printf("\tchlev_invite = '%i'\n",me.chlev_invite); continue; }
        if (sscanf(line,"ChanLevNoStatus %i",&me.chlev_nostatus)) { if (get_core()->vv) printf("\tchlev_nostatus = '%i'\n",me.chlev_nostatus); continue; }
        if (sscanf(line,"ChanLevAKick %i",&me.chlev_akick)) { if (get_core()->vv) printf("\tchlev_akick = '%i'\n",me.chlev_akick); continue; }
        if (sscanf(line,"ChanLevAKickBan %i",&me.chlev_akb)) { if (get_core()->vv) printf("\tchlev_akb = '%i'\n",me.chlev_akb); continue; }
        if (sscanf(line,"EmailReg %i",&me.emailreg)) { if (get_core()->vv) printf("\temailreg = '%i'\n",me.emailreg); continue; }

        if (get_core()->vv) printf("\tUNKNOWN : %s",line);
    }

    fclose(config_file); 
    if (get_core()->verbose) printf("Configuration loaded\n");
}
