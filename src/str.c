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


#include <globals.h>
#include <child.h>

int IsCharInString(char chr, char *string)
{
    for (; *string; string++) {
        if (chr == *string)
            return 1;
    }
    return 0;
} 

char *SeperateWord(char *sentence)
{
    if (sentence == NULL)
        return NULL;

    while (*sentence != ' ' && *sentence != '\0' && *sentence != '\n')
        sentence++;

    if (*sentence == '\0' || *sentence == '\n') {
        *sentence = '\0';
        return (NULL);
    }

    if (*sentence == ' ') {
        *sentence = '\0';
        sentence++;
    }

    return (StripBlanks(sentence));
}

char *StripBlanks(char *paddedstring)
{   
    if (paddedstring == NULL)
        return NULL;

    while (*paddedstring == ' ')
        paddedstring++;

     return paddedstring;
}

int Strcmp(const char *s1, const char *s2)
{
    if (s1 && s2 && *s1 != '\0' && *s2 != '\0')
        return strcasecmp(s1,s2);
    else
        return 1;
}

char *strtosql (char *dst, char *src, int len)
{
    int i,j;

    for (i=0,j=0;i<len;i++,j++) {
        if (src[i] == '\\' || src[i] == '\'' || src[i] == '"')
            dst[j++] = '\\';
        dst[j] = src[i];
    }

    return dst;
}

int __match_regex (char *reg, char *str, int param)
{
    int status;
    regex_t re;
    if (regcomp(&re,reg,param) != 0)
        return 0;

    status = regexec(&re,str,0,NULL,0);
    regfree(&re);
    if (status != 0) return 0;
    return 1;
}

char *parse_range (char *dst, char *range)
{
    unsigned int i;
    int j=0,k;

    for (i=0;i<strlen(range);i++) {
        if (i == 0) {
            if (range[i] == '-')
                dst[j++] = '-';
            else if (range[i+1] != '-')
                dst[j++] = range[i];
        }
        if (i > 0) {
            if (range[i] != '-' && (range[i-1] != '-' || i-1 == 0) && range[i+1] != '-')
                dst[j++] = range[i];
            if (range[i] != '-' && range[i-1] == '-' && range[i+1] == '-' && i-1 != 0)
                return NULL;
            if (range[i] != '-' && range[i-1] == '-' && i-1 != 0)
                continue;
        }
        if (range[i] != '-' && range[i+1] == '-') {
            if (range[i+2] == '-')
                return NULL; 
            if (range[i+2] < range[i])
                return NULL;
            for (k = range[i]; k <= range[i+2]; k++)
                dst[j++] = k;
        }
    }

    dst[j] = '\0';
    return dst;
}

char *gen_rand_string (char *dst, char *range, int size)
{   
    char text[256];
    int i;
 
    init_srandom();
   
    bzero(text,256);
    parse_range(text,range);
    
    for (i=0;i<size;i++)
        dst[i] = text[random() % (strlen(text))];
    dst[i] = '\0';
    
    return dst;
}

void ToLower (char *dst, char *buf, unsigned int size)
{
    unsigned int i;

    if (!dst || !buf)
        return;

    for (i=0;i<strlen(buf) && i < size;i++)
        dst[i] = tolower(buf[i]);
}
