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

#include "child.h"

#include <mysql/mysql.h>

extern MYSQL mysql;

int connect_to_db()
{
    mysql_init(&mysql);
    if (!mysql_real_connect(&mysql,me.mysql_host,me.mysql_login,me.mysql_passwd,me.mysql_db,0,NULL,0)) return 0;
    return 1;
}

int reconnect_to_db()
{
    mysql_close(&mysql);
    return connect_to_db();
}
