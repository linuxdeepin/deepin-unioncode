// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
