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


#include "mem.h"

#include "botserv.h"
#include "channel.h"
#include "commands.h"
#include "modules.h"
#include "string_utils.h"
#include "trust.h"
#include "user.h"

#include <stdio.h>
#include <string.h>

void cleanup_reconnect()
{
    clear_wchans();
    clear_limits();
    clear_guests();
    clear_nicks();
    clear_fakes();
}

long get_mem(int which)
{
    long vsize,rss;
#ifdef __FreeBSD__
    kvm_t *kd;
    char *err = (char *)malloc(64*sizeof(char));
    bzero(err,64);
    struct kinfo_proc *ki;
    int pagesize,cnt,size;
    size = sizeof(pagesize);
    sysctlbyname("hw.pagesize",&pagesize,&size,NULL,0);

    kd = kvm_open(getbootfile(),"/dev/null",NULL,O_RDONLY,err);
    ki = kvm_getprocs(kd,KERN_PROC_PID,getpid(),&cnt);

    vsize = ki->ki_size/1024;
    rss = (ki->ki_rssize*pagesize)/1024;
    kvm_close(kd);
    free(err);
#else
    FILE *proc_stat = fopen("/proc/self/stat","r");
    if (!proc_stat) return -1;

    if (fscanf(proc_stat,"%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu %ld %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d",&vsize,&rss) == EOF) {
        fprintf(stderr, "Failed to read /proc/self/stat (EOF).\n");
        return -1;
    }
    fclose(proc_stat);
    vsize /= 1024;
    rss *= 4;
#endif
    switch(which) {
        case MEM_VSIZE:
            return vsize;
        case MEM_RSS:
            return rss;
        default:
            return -1;
    }
}

void InitMem()
{
    memset(&indata, 0, sizeof(indata));
    memset(&outdata, 0, sizeof(outdata));
}

int hash(const char *buffer)
{
    const char *ptr;
    int tmp, val = 0;
    char tltmp[1024];
    memset(tltmp,0,1024);
    ToLower(tltmp, buffer, 1023);
    ptr = tltmp;

    while (*ptr) {
        val = (val << 4) + (*ptr);
        if ((tmp = (val & 0xf0000000))) {
            val = val ^ (tmp >> 24);
            val = val ^ tmp;
        }
        ptr++;
    }

    if (val < 0) val *= -1;

    return val % MAX_HASH;
}
