// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
build:
gcc -g -shared -fPIC -O0 -o libinprocdump.so inprocdump.c -ldl
*/
#include <assert.h>
#include <stdbool.h>
#include <elf.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__aarch64__)
#include "../aarch64/syscall.h"
#else
#include <syscall.h>
#endif
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/auxv.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <sys/user.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/unistd.h>
#include <linux/seccomp.h>
#include <time.h>
#include <sys/un.h>

#include "../include/shared_mem_dump.h"
#include "../include/event_man.h"
#include "../cpu.h"

#define SAFE_GUARD (1024)

static int g_pid = 0; // store current pid to avoid fork affect
static MemoryDumper *g_dumper = NULL;
static unsigned char *parameter_flags = NULL;
struct number_set trace_set[1];
static int get_stack_range(MemoryDumper *dumper, uintptr_t sp);

long syscall_no_hook(long syscall_number, ...);

static char debug_dumps_on = 0;

void debug_dump(const char *fmt, ...)
{
	int len;
	va_list ap;

	if (!debug_dumps_on)
		return;

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	if (len <= 0)
		return;

	char buf[len + 1];

	va_start(ap, fmt);
	len = vsprintf(buf, fmt, ap);
	va_end(ap);

	syscall_no_hook(SYS_write, 2, buf, len);
}

#if defined(__x86_64__)
#ifndef SYS_pkey_free
#define SYS_pkey_free (331)
#endif

static unsigned char syscall_buf[
    (SYS_pkey_free - __NR_Linux + 1) * 2 +
    (SYS_pkey_free - __NR_Linux + 1 + 7) / 8 + 256];

__asm (
    ".global syscall_no_hook\n\t"
    ".type   syscall_no_hook, @function\n\t"
    ".text\n\t"
    "syscall_no_hook:\n\t"
    "movq %rdi, %rax\n\t" /* convert from linux ABI calling */
    "movq %rsi, %rdi\n\t" /* convention to syscall calling convention */
    "movq %rdx, %rsi\n\t"
    "movq %rcx, %rdx\n\t"
    "movq %r8, %r10 \n\t"
    "movq %r9, %r8\n\t"
    "movq 8(%rsp), %r9\n\t"
    "syscall\n\t"
    "ret\n\t"
    ".size syscall_no_hook, .-syscall_no_hook\n\t"
);

#elif defined(__sw_64)
#ifndef SYS_execveat
#define SYS_execveat (513)
#endif
static unsigned char syscall_buf[
    (SYS_execveat - __NR_Linux + 1) * 2 +
    (SYS_execveat - __NR_Linux + 1 + 7) / 8 + 256];

__asm(
    ".global syscall_no_hook\n\t"
    ".type   syscall_no_hook, @function\n\t"
    ".text\n\t"
    "syscall_no_hook:\n\t"
    "bis $31, $16, $0\n\t"
    "bis $31, $17, $16\n\t"
    "bis $31, $18, $17\n\t"
    "bis $31, $19, $18\n\t"
    "bis $31, $20, $19\n\t"
    "bis $31, $21, $20\n\t"
    "ldl $21, 0($30)\n\t" // 6th argument of syscall
    "sys_call 0x83\n\t"  // sys_call never change ra register
    "ret $31, ($26), 0x1\n\t"
    ".size syscall_no_hook, .-syscall_no_hook\n\t"
);

#elif defined(__mips64)
#ifndef SYS_pkey_free
#define SYS_pkey_free (325 + __NR_Linux)
#endif

static unsigned char syscall_buf[
    (SYS_pkey_free - __NR_Linux + 1) * 2 +
    (SYS_pkey_free - __NR_Linux + 1 + 7) / 8 + 256];

__asm(
    ".globl syscall_no_hook\n\t"
    "syscall_no_hook:\n\t"
    "move $v0, $a0\n\t"
    "move $a0, $a1\n\t"
    "move $a1, $a2\n\t"
    "move $a2, $a3\n\t"
    "move $a3, $a4\n\t"
    "move $a4, $a5\n\t"
    "move $a5, $a6\n\t"
    "syscall\n\t"
    "jr $ra\n\t"
    "nop\n\t"
);
#elif defined(__aarch64__)
#ifndef SYS_fork
#define SYS_fork (1079 + __NR_Linux)
#endif

static unsigned char syscall_buf[
    (SYS_fork - __NR_Linux + 1) * 2 +
    (SYS_fork - __NR_Linux + 1 + 7) / 8 + 256];

__asm(
    ".globl syscall_no_hook\n\t"
    "syscall_no_hook:\n\t"
    "mov x8, x0\n\t"
    "mov x0, x1\n\t"
    "mov x1, x2\n\t"
    "mov x2, x3\n\t"
    "mov x3, x4\n\t"
    "mov x4, x5\n\t"
    "mov x5, x6\n\t"
    "svc #0x0\n\t"
    "ret\n\t"
    "nop \n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
);
#else
#error need define new arch
#endif

static unsigned char number_isset(const unsigned int i, const number_slot_t *const vec)
{
    return (vec[i / BITS_PER_SLOT] &
           ((number_slot_t)1 << (i % BITS_PER_SLOT))) > 0;
}

unsigned char is_number_in_set(const unsigned number)
{
    if (0 == trace_set[0].nslots) {
        return trace_set[0].not_flag;
    }
    return (((number / BITS_PER_SLOT < trace_set[0].nslots) &&
              number_isset(number, trace_set[0].vec)) ^ trace_set[0].not_flag);
}

static int open_socket(int *sock_fd)
{
    struct sockaddr_un addr;
    int sc, rc = -1;

    *sock_fd = syscall_no_hook(SYS_socket, AF_UNIX, SOCK_STREAM, 0);
    if (*sock_fd == -1) {
        debug_dump("failed to create socket: %s\n", strerror(errno));
        goto done;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path) - 1,
            SHARED_SOCKET_NAME, g_pid);
    debug_dump("socket: %s -> %d\n", addr.sun_path, *sock_fd);

    sc = syscall_no_hook(SYS_connect,
            *sock_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (sc == -1) {
        syscall_no_hook(SYS_close, *sock_fd);
        debug_dump("failed to connect: %s\n", strerror(errno));
        goto done;
    }

    rc = 0;

done:
    return rc;
}

int send_cmd(int cmd, uintptr_t arg)
{
    char buf[20];
    int sock_fd = 0;
    int nbytes = 0;

    *(int *)&buf[0] = cmd;
    *(uintptr_t *)&buf[4] = arg;

    if (open_socket(&sock_fd) < 0) {
        return -1;
    }

    nbytes = syscall_no_hook(SYS_write,
            sock_fd, buf, sizeof(int) + sizeof(uintptr_t));
    debug_dump("send_cmd:%d, %lx->%d\n", cmd, arg, nbytes);

    // get result
    *(int *)&buf[0] = -1;
    nbytes = syscall_no_hook(SYS_read, sock_fd, buf, sizeof(int));
    syscall_no_hook(SYS_close, sock_fd);

    return *(int *)&buf[0];
}

MemoryDumper *get_memory_dumper(void)
{
    return g_dumper;
}

MemoryDumper *create_memory_dumper(void)
{
    char path[256];
    int fd = 0;
    void *map_addr = NULL;
    int flags = MAP_SHARED;
    MemoryDumper *dumper = NULL;
    int size = 0;
    char *data = NULL;
    char *shared_buffer_size = NULL;
    int nonce = send_cmd(SYS_share_name, 0);
    if (nonce < 0) {
        return NULL;
    }

    snprintf(path, sizeof(path) - 1,
            SHARED_FILE_NAME, g_pid, nonce);
    shared_buffer_size = getenv("ST2_SYSCALL_BUFFER_SIZE");
    if (shared_buffer_size) {
        size = atoi(shared_buffer_size);
        if (size < SHARED_BUFFER_SIZE) size = SHARED_BUFFER_SIZE;
    } else {
        size = SHARED_BUFFER_SIZE;
    }

#if defined (__aarch64__)
    fd = syscall_no_hook(SYS_openat, AT_FDCWD, path, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
#else
    fd = syscall_no_hook(SYS_open, path, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
#endif
    if (fd < 0) {
        return NULL; // xabort_errno(errno, "Failed to open shmem file!\n");
    }
    debug_dump("shmem file:%s, size=%d\n", path, size);

    send_cmd(SYS_init_buffers, size);

    map_addr = (void *)syscall_no_hook(SYS_mmap,
            NULL, size, PROT_READ | PROT_WRITE, flags, fd, 0);
    syscall_no_hook(SYS_close, fd);
    // Remove the fs name so that we don't have to worry about
    // cleaning up this segment in error conditions.
    syscall_no_hook(SYS_unlink, path);

    if (MAP_FAILED == map_addr) {
        return NULL; // xabort_errno(errno, "Failed to mmap shmem region\n!");
    }

    dumper = (MemoryDumper *)map_addr;

    dumper->pid = g_pid;
    dumper->mutex = 1;
    assert(dumper->size + sizeof(*dumper) == (unsigned)size);

    // load syscall filter set
    trace_set[0].nslots = dumper->syscall.nslots;
    trace_set[0].not_flag = dumper->syscall.not_flag;
    data = BUFFER_HEAD(dumper);
    size = sizeof(number_slot_t) * trace_set[0].nslots;
    if (trace_set[0].nslots > 0) {
        trace_set[0].vec = (number_slot_t *)&syscall_buf[0];
        memcpy(trace_set[0].vec, data, size);
        data += size;
    } else {
        trace_set[0].vec = NULL;
    }
    parameter_flags = &syscall_buf[0] + size;
    memcpy(parameter_flags, data + sizeof(int), *(int *)data);

    send_cmd(SYS_enable_dump, 0);

    g_dumper = dumper;

    return dumper;
}

void lock_buffers(MemoryDumper *dumper)
{
    int s;
    // int tid = syscall_no_hook(SYS_gettid);
    // debug_dump("[%d] try to lock\n", tid);
    for (;;) {
        // returns true if the comparison is successful and newval was written
        if (__sync_bool_compare_and_swap(&dumper->mutex, 1, 0)) {
            // debug_dump("[%d] lock ok\n", tid);
            break;
        }

        /* Futex is not available; wait */
        s = syscall_no_hook(SYS_futex,
                &dumper->mutex, FUTEX_WAIT, 0, NULL, NULL, 0);
        if (s == -1 && errno != EAGAIN) {
            // debug_dump("[%d] FUTEX_WAIT failed, errno=%d\n", tid, errno);
            break;
        }
        // try again for there are more than one thread waited.
    }
}

void unlock_buffers(MemoryDumper *dumper)
{
    // int tid = syscall_no_hook(SYS_gettid);
    // debug_dump("[%d] try to unlock\n", tid);
    if (__sync_bool_compare_and_swap(&dumper->mutex, 0, 1)) {
        if (syscall_no_hook(SYS_futex,
                &dumper->mutex, FUTEX_WAKE, 1, NULL, NULL, 0) < 0) {
            // debug_dump("[%d] FUTEX_WAKE failed, errno=%d\n", tid, errno);
        }
        else {
            // debug_dump("[%d] unlock ok\n", tid);
        }
    }
    else {
        assert(0);
    }
}

long get_sp(void)
{
    long sp = 0;
#if defined(__x86_64__)
    __asm__ volatile("\t movq %%rsp, %0" :
            "=r"(sp));
#elif defined(__sw_64)
    __asm__ volatile("\t mov $30, %0" :
            "=r"(sp));
#elif defined(__mips64)
    __asm__ volatile("\t move %0, $sp" :
            "=r"(sp));
#elif defined(__aarch64__)
    __asm__ volatile("\t mov %0, sp" :
            "=r"(sp));
#endif
    return sp;
}

void get_regs(char* p)
{
#if defined(__x86_64__)
    __asm__ volatile("\t movq %%rsp, %0" :
            "=r"(((struct user_regs_struct *)(p))->rsp));
    __asm__ volatile("\t movq %%rbp, %0" :
            "=r"(((struct user_regs_struct *)(p))->rbp));
    __asm__ volatile("\t lea (%%rip), %%rax\n\t movq %%rax,%0" :
                "=r"(((struct user_regs_struct *)(p))->rip));
#elif defined(__sw_64)
    __asm__ volatile("\t mov $26, %0" :
            "=r"(((USER_REGS *)(p))->ra));
    __asm__ volatile("\t mov $30, %0" :
            "=r"(((USER_REGS *)(p))->sp));
#elif defined(__mips64)
    __asm__ volatile("\t move %0, $ra" :
            "=r"(((USER_REGS *)(p))->ra));
    __asm__ volatile("\t move %0, $sp" :
            "=r"(((USER_REGS *)(p))->sp));
#elif defined(__aarch64__)
    __asm__ volatile("\t mov %0, x30" :
            "=r"(((USER_REGS *)(p))->ra));
    __asm__ volatile("\t mov %0, sp" :
            "=r"(((USER_REGS *)(p))->sp));
#endif
}

void record_event_simple(MemoryDumper* dumper, int type, const char* msg, int len)
{
    int tid = syscall_no_hook(SYS_gettid);
    int msg_len = len > (int)(sizeof(long)) ? len:0;
    char *sp = NULL;
    char *sp_end = NULL;
    int stack_size = 0;
    char *walk = NULL;
    int current = 0;
    const int cpu_size = sizeof(USER_REGS) + sizeof(USER_FPREGS);

    lock_buffers(dumper);

    assert(msg_len <= EVENT_EXTRA_INFO_SIZE);
    assert(DUMP_REASON_signal < type && type < DUMP_REASON_ptrace);

    current = dumper->current;

    // check stack range
    sp = (char *)(get_sp() & (~(dumper->page_size - 1)));
    if ((uintptr_t)sp < dumper->stack_begin ||
         (uintptr_t)sp > dumper->stack_end) {
        stack_size = get_stack_range(dumper, (uintptr_t)sp);
        sp_end = sp + stack_size;
    } else {
        sp_end = sp + dumper->max_stack_size;
        if (sp_end > (char *)(dumper->stack_end)) {
            sp_end = (char *)(dumper->stack_end);
        }
        if (sp_end > sp) stack_size = (sp_end - sp);
        else stack_size = 0;
    }

    if (current + stack_size + cpu_size + msg_len + SAFE_GUARD > dumper->size) {
        send_cmd(SYS_flush_buffers, current);
        current = 0;
    }
    debug_dump("dump-event begin:tid=%d,type=%d;pos=%d;\n",
        tid, type, current);

    walk = BUFFER_HEAD(dumper) + current;
    syscall_no_hook(SYS_clock_gettime, CLOCK_REALTIME, (struct timespec *)walk);
    walk += sizeof(struct timespec);

    *(short *)walk = type;
    *(short *)(walk + 2) = 1;
    walk += sizeof(int);

    *(int *)walk = tid | (msg_len<<16); // extra data is msg
    walk += sizeof(int);
    memcpy(walk, msg, msg_len);
    walk += msg_len;

    *(short *)walk = tid;
    *(short *)(walk + 2) = cpu_size;
    walk += sizeof(int);

    get_regs(walk);
    walk += cpu_size;

    // TODO: dump TLS (.tbss + .tdata, _tls_get_addr)
    *(int *)walk = 0;
    walk += sizeof(int);

    // stack is always zero
    *(char **)walk = sp;
    *(int *)(walk + sizeof(sp)) = stack_size;
    walk += sizeof(char *) + sizeof(int);
    if (stack_size > 0) {
        memcpy(walk, sp, stack_size);
        walk += stack_size;
    }

    *(int *)walk = 0; // heap count always zero;
    walk += sizeof(int);

    dumper->current = walk - BUFFER_HEAD(dumper);
    assert(dumper->current < dumper->size);

    debug_dump("dump-event end:tid=%d,type=%d;pos=%d;\n",
        tid, type, dumper->current);
    unlock_buffers(dumper);
}

typedef struct tagVmSegment {
    uintptr_t start;
    uintptr_t end;
}VmSegment;

static char *get_line(char *buf, int size)
{
    for (int i = 0; i < size; ++i) {
        if (buf[i] == '\n') {
            return buf + i;
        }
    }

    return NULL;
}

static int get_stack_range(MemoryDumper *dumper, uintptr_t sp)
{
    char buf[4096];
    char *line = NULL;
    char *line_end = NULL;
    char *stop = NULL;
    uintptr_t start = 0;
    uintptr_t end = 0;
    int eof = 0;
    int size = 256;
    int remain = 0;

#if defined(__aarch64__)
    int fd = syscall_no_hook(SYS_openat, AT_FDCWD, "/proc/self/maps", O_RDONLY);
#else
    int fd = syscall_no_hook(SYS_open, "/proc/self/maps", O_RDONLY);
#endif
    if (fd <= 0) {
        return 256;
    }

    while (!eof) {
        int bytes = syscall_no_hook(SYS_read,
                fd, buf + remain, sizeof(buf) - remain);
        if (bytes <= 0) {
            eof = 1;
            break;
        }
        remain += bytes;

        line = buf;

        while (remain > 0) {
            line_end = get_line(line, remain);
            if (NULL == line_end) {
                memmove(buf, line, remain);
                break;
            }
            start = strtol(line, &stop, 16);
            end = strtol(stop + 1, &stop, 16);
            if (start <= sp && sp <= end) {
                eof = 1;
                size = end - sp;
                break;
            }
            remain -= (line_end + 1 - line);
            line = line_end + 1;
        }
    }
    syscall_no_hook(SYS_close, fd);

    if (size > dumper->max_stack_size) {
        size = dumper->max_stack_size;
    }

    return size;
}

static int my_strlen(char* buf, int max)
{
    int i = 0;
    while (buf[i] > 0 && i < max) {
        ++i;
    }

    if (0 == buf[i]) {
        ++i;
    }

    return i;
}

/*
 * binary layout for one event, always 4bytes align:
 * time(sizeof(struct timespec)), event_type(2byte), thread_num(2byte),
 * current_tid(2byte), event_extra_size(2byte)[,event_extra_data]
 * {
 *   tid(2byte), cpu_context_size(2byte)[, user_regs_struct+user_fpregs_struct],
 *    tls_size(4byte)[, tls_data]
 *    stack_addr(uintptr_t), stack_length(4byte)[,stack_data]
 * }
 *
 * ...
 *
 * {
 *   tid(2byte), cpu_context_size(2byte)[, user_regs_struct+user_fpregs_struct],
 *    tls_size(4byte)[, tls_data]
 *    stack_addr(uintptr_t), stack_length(4byte)[,stack_data]
 * }
 * heap_count(4byte)[, addr(uintptr_t)+size(4byte)+heap_data, ...]
 */
void record_syscall(MemoryDumper *dumper, int nr, long* args, long result, void *cpu)
{
    VmSegment segs[12];
    VmSegment *seg = &segs[0];
    int heap_count = 0;
    int heap_size = 0;
    unsigned char arg_flag = 0;
    unsigned char arg_count = 0;
    char *sp = NULL;
    char *sp_end = NULL;
    int stack_size = 0;
    char *walk = NULL;
    const int cpu_size = sizeof(USER_REGS) + sizeof(USER_FPREGS);
    int current = 0;
    int syscall_parameter_size = 0;
    const uintptr_t mask = ~3;
    USER_REGS *ctx = (USER_REGS*)cpu;
    int tid = syscall_no_hook(SYS_gettid);

    lock_buffers(dumper);

    // the follow memcpy maybe crashed so store current position first!
    current = dumper->current;
    syscall_parameter_size = dumper->max_param_size;

    // check stack range
#if defined(__x86_64__)
    sp = (char *)(ctx->rsp & (~(dumper->page_size - 1)));
#else
    sp = (char *)(ctx->sp & (~(dumper->page_size - 1)));
#endif
    if ((uintptr_t)sp < dumper->stack_begin ||
         (uintptr_t)sp > dumper->stack_end) {
        stack_size = get_stack_range(dumper, (uintptr_t)sp);
        sp_end = sp + stack_size;
    } else {
        sp_end = sp + dumper->max_stack_size;
        if (sp_end > (char *)(dumper->stack_end)) {
            sp_end = (char *)(dumper->stack_end);
        }
        if (sp_end > sp) stack_size = (sp_end - sp);
        else stack_size = 0;
    }

    // check if dump parameter of syscall
    arg_flag = parameter_flags[2 * (nr -  __NR_Linux)];
    arg_count = parameter_flags[2 * (nr - __NR_Linux) + 1];
    if ((arg_flag != 0) && (syscall_parameter_size > 0)) {
        // parameter is void*, size indicate in next parameter
        uint8_t size_indicate_by_next = arg_flag & 0x80;
        for (unsigned char i = 0; i < arg_count; ++i) {
            if (arg_flag & 1) {
                uintptr_t addr = args[i];
                if ((addr > 0) &&
                    ((addr < (uintptr_t)sp) ||
                     (addr > (uintptr_t)sp_end))) {
                    seg->start = addr;
                    if (size_indicate_by_next) {
                        if (args[i + 1] > 0) {
                            seg->end = addr + args[i + 1];
                            ++seg;
                        }
                        break;
                    }
                    seg->end = addr + my_strlen((char *)addr, syscall_parameter_size);
                    ++seg;
                }
            }
            arg_flag >>= 1;
        }

        if (seg != &segs[0]) {
            heap_count = seg - &segs[0];
            seg = &segs[0];
            for (int i = 0; i < heap_count; ++i) {
                // make size is 4byte align
                heap_size += ((seg[i].end - seg[i].start + 3) & mask) +
                    sizeof(int) + sizeof(uintptr_t);
            }
        }
    }

    if (current + stack_size + cpu_size + heap_size + SAFE_GUARD > dumper->size) {
        send_cmd(SYS_flush_buffers, current);
        current = 0;
    }
    debug_dump("dump-syscall begin:tid=%d,syscall=%d,%lx;pos=%d;stack=%p,%d;\n",
        tid, nr, result, current, sp, stack_size);

    walk = BUFFER_HEAD(dumper) + current;
    syscall_no_hook(SYS_clock_gettime, CLOCK_REALTIME, (struct timespec *)walk);
    walk += sizeof(struct timespec);

    *(short *)walk = nr;
    *(short *)(walk + 2) = 1;
    walk += sizeof(int);

    *(int *)walk = tid | (sizeof(long)<<16); // extra data is syscall result
    walk += sizeof(int);
    *(long *)walk = result;
    walk += sizeof(long);

    *(short *)walk = tid;
    *(short *)(walk + 2) = cpu_size;
    walk += sizeof(int);

    memcpy(walk, ctx, sizeof(USER_REGS));
#if defined(__mips64) || defined(__sw_64)
    ((USER_REGS*)walk)->sp += 512; //see syscall_wrapper in mips64/sunway64
    ((USER_REGS*)walk)->ra = ctx->t2; //see syscall_wrapper in mips64/sunway64
    ((USER_REGS*)walk)->pc = ctx->t3; // see build_trampoline in mips64/sunway64
#elif defined(__aarch64__)
    ((USER_REGS*)walk)->sp += 512; //see syscall_wrapper in arm64
    ((USER_REGS*)walk)->ra = ctx->x11; //see syscall_wrapper in arm64
    ((USER_REGS*)walk)->pc = ctx->x10; // see build_trampoline in arm64
#endif
    walk += cpu_size;

    // TODO: dump TLS (.tbss + .tdata, _tls_get_addr)
    *(int *)walk = 0;
    walk += sizeof(int);

    // stack
    *(char **)walk = sp;
    *(int *)(walk + sizeof(sp)) = stack_size;
    walk += sizeof(char *) + sizeof(int);

    if (stack_size > 0) {
        memcpy(walk, sp, stack_size);
        walk += stack_size;
    }

    *(int *)walk = heap_count;
    walk += sizeof(int);
    for (int i = 0; i < heap_count; ++i) {
        int size = seg->end - seg->start;
        int align = ((size + 3) & mask) - size;
        *(uintptr_t *)walk = seg->start;
        *(int *)(walk + sizeof(uintptr_t)) = size + align;
        memcpy(walk + sizeof(uintptr_t) + sizeof(int),
                (void *)seg->start, size);
        walk += size + sizeof(int) + sizeof(uintptr_t);
        if (align > 0) {
            memset(walk, 0xcc, align);
            walk += align;
        }
        ++seg;
    }

    dumper->current = walk - BUFFER_HEAD(dumper);
    assert(dumper->current < dumper->size);
    ++dumper->syscall_count;

    debug_dump("dump-syscall end:tid=%d,syscall=%d;pos=%d;heap_count=%d;\n",
        tid, nr, dumper->current, heap_count);

    unlock_buffers(dumper);
}

static __attribute__((constructor)) void init_func(int argc, char **argv)
{
    (void)argc;

    if (!strcmp(argv[0], "gdb")) {
        return;
    }

#if defined(__sw_64)
    g_pid = syscall_no_hook(SYS_getxpid);
#else
    g_pid = syscall_no_hook(SYS_getpid);
#endif

    if (getenv("ST2_DEBUG_DUMP")) {
        debug_dumps_on = 1;
        debug_dump("libinprocdump start g_pid=%d\n", g_pid);
    }
}
