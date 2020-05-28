/*          
Child, Internet Relay Chat Services
Copyright (C) 2005-2007  David Lebrun (target0@geeknode.org)
            
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


#ifndef _PARTYLINE_H
#define _PARTYLINE_H

#define LogCommand() sendto_all_butone(eclient,"[%s] %s %s",eclient->nick,command,(tail) ? tail : "")

#define CheckPAuth() if (eclient->authed != 1) \
                        return
#define CheckAndLog() CheckPAuth(); \
                        LogCommand()

typedef struct eclient {
    char nick[NICKLEN + 1];
    char host[NI_MAXHOST];
    char addr[NI_MAXHOST];
    char port[NI_MAXSERV];
    int authed;
    int fd; /* hash key (HASH_INT) */
    char pchunkbuf[CHUNKSIZE];
    char *pchunkbufentry;
    char *pcurrentline,*pnextline;
    struct eclient *next,*prev;
    struct eclient *lnext,*lprev;
} Eclient;

typedef struct {
    int size;
    TABLE(Eclient);
    Eclient *lhead;
} eclientlist;

void ParseEclient (Eclient *);
Eclient *find_eclient (int);
Eclient *find_eclient_name (char *);
Eclient *AddEclient (int, struct sockaddr_storage, socklen_t);
void DeleteEclient (Eclient *);
void send_to (Eclient *, char *, ...);
void sendto_all(char *, ...);
void sendto_all_butone(Eclient *, char *, ...);
void p_auth (Eclient *, User *, char *, char *);
void p_canpl (Eclient *, User *, char *, char *);
void p_close (Eclient *, User *, char *, char *);
void p_deletenick (Eclient *, User *, char *, char *);
void p_die (Eclient *, User *, char *, char *);
void p_dropchan (Eclient *, User *, char *, char *);
void p_dropuser (Eclient *, User *, char *, char *);
void p_eject (Eclient *, User *, char *, char *);
void p_emerg (Eclient *, User *, char *, char *);
void p_exec (Eclient *, User *, char *, char *);
void p_getchaninfo (Eclient *, User *, char *, char *);
void p_getuserinfo (Eclient *, User *, char *, char *);
void p_gline (Eclient *, User *, char *, char *);
void p_help (Eclient *, User *, char *, char *);
void p_kill (Eclient *, User *, char *, char *);
void p_modlist (Eclient *, User *, char *, char *);
void p_modload (Eclient *, User *, char *, char *);
void p_modunload (Eclient *, User *, char *, char *);
void p_nicklist (Eclient *, User *, char *, char *);
void p_quit (Eclient *);
void p_raw (Eclient *, User *, char *, char *);
void p_rehash (Eclient *, User *, char *, char *);
void p_restart (Eclient *, User *, char *, char *);
void p_savedb (Eclient *, User *, char *, char *);
void p_set (Eclient *, User *, char *, char *);
void p_setchan (Eclient *, User *, char *, char *);
void p_setchanopt (Eclient *, User *, char *, char *);
void p_setnickflag (Eclient *, User *, char *, char *);
void p_setuser (Eclient *, User *, char *, char *);
void p_setuseropt (Eclient *, User *, char *, char *);
void p_who (Eclient *, User *, char *, char *);

#endif
