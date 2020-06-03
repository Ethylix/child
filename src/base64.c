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


#include <arpa/inet.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* Code ripped from UnrealIRCD (www.unrealircd.com) */

const char Base64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char Pad64 = '=';
char base64_to_int6_map[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,
    -1, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, -1, 63, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

long base64_to_int(char *b64)
{                   
    int v = base64_to_int6_map[(u_char)*b64++];
            
    if (!b64)
        return 0;
        
    while (*b64)
    {           
        v <<= 6;    
        v += base64_to_int6_map[(u_char)*b64++];
    }           

    return v;
}

int b64_decode(char const *src, char *target, size_t targsize)
{
    int tarindex, state, ch;
    char *pos;

    state = 0;
    tarindex = 0;

    while ((ch = *src++) != '\0') {
        if (isspace(ch))
            continue;

        if (ch == Pad64)
            break;

        pos = strchr(Base64, ch);
        if (pos == 0)
            return (-1);

        switch (state) {
        case 0:
            if (target) {
                if ((size_t)tarindex >= targsize)
                    return (-1);
                target[tarindex] = (pos - Base64) << 2;
            }
            state = 1;
            break;
        case 1:
            if (target) {
                if ((size_t)tarindex + 1 >= targsize)
                    return (-1);
                target[tarindex]   |=  (pos - Base64) >> 4;
                target[tarindex+1]  = ((pos - Base64) & 0x0f)
                            << 4 ;
            }
            tarindex++;
            state = 2;
            break;
        case 2:
            if (target) {
                if ((size_t)tarindex + 1 >= targsize)
                    return (-1);
                target[tarindex]   |=  (pos - Base64) >> 2;
                target[tarindex+1]  = ((pos - Base64) & 0x03)
                            << 6;
            }
            tarindex++;
            state = 3;
            break;
        case 3:
            if (target) {
                if ((size_t)tarindex >= targsize)
                    return (-1);
                target[tarindex] |= (pos - Base64);
            }
            tarindex++;
            state = 0;
            break;
        default:
            abort();
        }
    }
    if (ch == Pad64) {
        ch = *src++;
        switch (state) {
        case 0:
        case 1:
            return (-1);

        case 2:
            for ((void)NULL; ch != '\0'; ch = *src++)
                if (!isspace(ch))
                    break;
            if (ch != Pad64)
                return (-1);
            ch = *src++;
        case 3:
            for ((void)NULL; ch != '\0'; ch = *src++)
                if (!isspace(ch))
                    return (-1);

            if (target && target[tarindex] != 0)
                return (-1);
        }
    } else {
        if (state != 0)
            return (-1);
    }

    return (tarindex);
}

const char *decode_ip(char *buf)
{
    char targ[25];
    char blah[128];
    bzero(blah,128);

    b64_decode(buf, targ, 25);
    if (strlen(buf) <= 8)
        return inet_ntoa(*(struct in_addr *)targ);
    else
        return(inet_ntop(AF_INET6,targ,blah,INET6_ADDRSTRLEN));
}
