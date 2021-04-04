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


#include "logging.h"

#include "string_utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void mylog (char *file, char *msg, ...)
{
    char tmp[1024];
    char buf[1024];
    va_list val;
    FILE *index;
    time_t tm;
    ircsprintf(buf,1023,msg,val);

    tm = time(NULL);

    snprintf(tmp,1023,"[ %s] %s\n",ctime(&tm),buf);

    /* The ctime() function returns the result with a trailing '\n' */

    *(strstr(tmp,"\n")) = ' ';
    index = fopen(file,"a+");
    if (!index) return;
    fputs(tmp,index);
    fclose(index);
}
