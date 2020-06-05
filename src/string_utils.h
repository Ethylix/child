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

#ifndef _STRING_UTILS_H
#define _STRING_UTILS_H

#include <stdarg.h>
#include <stdio.h>

#define Strstr(x,y) strcasestr(x,y)
#define ircsprintf(a,b,c,d) va_start(d,c); \
                            vsnprintf(a,b,c,d); \
                            va_end(d)

#define match_regex(x, y) __match_regex(x, y, REG_EXTENDED|REG_NOSUB)

int IsCharInString (char, char *);
char *SeperateWord (char *);
char *StripBlanks (char *);
int Strcmp (const char *, const char *);
char *strtosql(char *, char *, int);
int __match_regex (char *, char *, int);
char *parse_range (char *, char *);
char *gen_rand_string (char *, char *, int);
void ToLower(char *, const char *, unsigned int);

#endif  // _STRING_UTILS_H
