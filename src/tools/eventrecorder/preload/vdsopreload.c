/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuchanghui<liuchanghui@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <memory.h>
#include <sys/time.h>
#include <sys/syscall.h>

/* build:
gcc -g -O0 -o libvdsopreload.so vdsopreload.c -fPIC -shared -D_GNU_SOURCE -ldl

  usage:
  LD_PRELOAD=libdbuspreload.so dbus-send
*/

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    return syscall(SYS_clock_gettime, clk_id, tp);
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    return syscall(SYS_gettimeofday, tv, tz);
}

#if defined(__x86_64__)
time_t time(time_t *tloc)
{
    return syscall(SYS_time, tloc);
}
#endif

#if 0
int getcpu(unsigned *cpu, unsigned *node, struct getcpu_cache *tcache)
{
    return syscall(SYS_getcpu, cpu, node, tcache);
}
#endif

static void __attribute__((constructor)) init_process(void) {
}
