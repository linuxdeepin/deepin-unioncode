// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _SHARED_EME_DUMP_H
#define _SHARED_EME_DUMP_H
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHARED_FILE_NAME              "/tmp/emd-syscallbuf-%d-%d"
#define SHARED_SOCKET_NAME            "/tmp/st2sock-%d.bin"

#if !defined(__mips__)
#define __NR_Linux                    (0)
#else
#include <asm/unistd.h>
#endif

#define SYS_init_buffers              (__NR_Linux + 1000 - 7) // least magic
#define SYS_flush_buffers             (__NR_Linux + 1000 - 6)
#define SYS_share_name                (__NR_Linux + 1000 - 5)
#define SYS_enable_dump               (__NR_Linux + 1000 - 4)
#define SYS_update_maps               (__NR_Linux + 1000 - 3)
#define SYS_dump_x11                  (__NR_Linux + 1000 - 2)
#define SYS_dump_dbus                 (__NR_Linux + 1000 - 1)
#define SHARED_BUFFER_SIZE            (1024 * 1024)
#define SYSCALL_PAGE_ADDR             (0x70000000)

typedef unsigned int number_slot_t;
#define BITS_PER_SLOT (sizeof(number_slot_t) * 8)

struct number_set {
	number_slot_t   *vec;
	unsigned int    nslots;
	unsigned char   not_flag;
};

typedef struct tagMemoryDumper {
    int fd;
    int pid;   /* store current pid to avoid fork affect */
    int mutex; /* at least 4bytes as futex, or EINVAL */
    int syscall_count;
    int size;
    int current;
    int page_size;
    int max_stack_size;
    uintptr_t stack_begin;
    uintptr_t stack_end;
    int max_param_size;
    struct number_set syscall;
}MemoryDumper;

#define BUFFER_HEAD(d)                   (char *)(d + 1)


unsigned char is_number_in_set(const unsigned number);
int send_cmd(int cmd, uintptr_t arg);
void lock_buffers(MemoryDumper *dumper);
void unlock_buffers(MemoryDumper *dumper);
MemoryDumper *create_memory_dumper(void);
MemoryDumper *get_memory_dumper(void);
void record_event_simple(MemoryDumper *dumper,
        int event_type, const char *msg, int msg_len);
void record_syscall(MemoryDumper *dumper,
        int nr, long *args, long result, void *cpu);

#ifdef __cplusplus
}
#endif

#endif /* end #ifndef _SHARED_EME_DUMP_H */
