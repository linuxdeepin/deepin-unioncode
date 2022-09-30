/*
kernel:
Linux frank-PC 4.9.0-deepin9-amd64 #1 SMP PREEMPT Deepin 4.9.29-4 (2017-07-17) x86_64 GNU/Linux

compiler:
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-linux-gnu/6/lto-wrapper
Target: x86_64-linux-gnu
Configured with: ../src/configure -v --with-pkgversion='Debian 6.3.0-11' --with-bugurl=file:///usr/share/doc/gcc-6/README.Bugs --enable-languages=c,ada,c++,java,go,d,fortran,objc,obj-c++ --prefix=/usr --program-suffix=-6 --program-prefix=x86_64-linux-gnu- --enable-shared --enable-linker-build-id --libexecdir=/usr/lib --without-included-gettext --enable-threads=posix --libdir=/usr/lib --enable-nls --with-sysroot=/ --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --with-default-libstdcxx-abi=new --enable-gnu-unique-object --disable-vtable-verify --enable-libmpx --enable-plugin --enable-default-pie --with-system-zlib --disable-browser-plugin --enable-java-awt=gtk --enable-gtk-cairo --with-java-home=/usr/lib/jvm/java-1.5.0-gcj-6-amd64/jre --enable-java-home --with-jvm-root-dir=/usr/lib/jvm/java-1.5.0-gcj-6-amd64 --with-jvm-jar-dir=/usr/lib/jvm-exports/java-1.5.0-gcj-6-amd64 --with-arch-directory=amd64 --with-ecj-jar=/usr/share/java/eclipse-ecj.jar --with-target-system-zlib --enable-objc-gc=auto --enable-multiarch --with-arch-32=i686 --with-abi=m64 --with-multilib-list=m32,m64,mx32 --enable-multilib --with-tune=generic --enable-checking=release --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu
Thread model: posix
gcc version 6.3.0 20170321 (Debian 6.3.0-11)

build:
gcc -g -x c -Wno-c90-c99-compat -std=c99 -shared -fPIC -O0 -o libsyscallpreload.so syscall_hook_x86-64.cc -I/usr/include/capstone -ldl -lcapstone -linprocdump

debug:
INTERCEPT_HOOK_CMDLINE_FILTER=$1 ST2_DEBUG_SYSCALL=1 LD_PRELOAD=./libsyscallpreload.so gdb --args $@

ref:
LD_TRACE_LOADED_OBJECTS, LD_VERBOSE, LD_DEBUG
// refer The GNU Assembler:
// https://www.eecs.umich.edu/courses/eecs373/readings/Assembler.pdf
*/

#include <assert.h>
#include <capstone.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define __USE_GNU
#include <dlfcn.h>
#include <link.h>
#include <errno.h>
#include <syscall.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/auxv.h>
#include <linux/mman.h>
#include <linux/futex.h>
#include <linux/unistd.h>

#define STUB_SIZE (16*1024*6)

void debug_dump(const char *fmt, ...);
void xabort_errno(int error_code, const char *msg);
long syscall_wrapper(long no, ...);
long syscall_no_intercept(long no, ...);
unsigned char* dump_maps(void);
unsigned char* find_gap_page_near_pc(uintptr_t pc, bool search_after_libc);


///////////////////////////////////////////////////////////////////////
// syscall dumper
#include "../cpu.h"
#include "../include/shared_mem_dump.h"

static MemoryDumper* g_dumper = NULL;

static int hook_ok = 0;
static int hook_failed = 0;
static unsigned char* trampoline_addr = 0;
static unsigned char* trampoline_addr_end = 0;
static void* vdso_addr = NULL;
static bool libc_found = false;
static bool debug_dumps_on = false;
static bool patch_all_objs = false;

///////////////////////////////////////////////////////////////////////
// syscall hook
long syscall_hook(USER_REGS* ctx)
{
    struct wrapper_ret {
        long v0;
        long a3;
    }ret;

    ret.a3 = 0;

    // NOTE: SYS_exit_group never return.

    if (!g_dumper) {
        // NOTE: delay to avoid crash in _dl_init of ld.so
        MemoryDumper* dumper = create_memory_dumper();
        if (dumper) {
            g_dumper = dumper;
            debug_dump("create dumper=%p\n", g_dumper);
        }
    }

    if (g_dumper && g_dumper->pid == syscall_no_intercept(SYS_getpid)) {
        unsigned char can_dump = is_number_in_set(ctx->v0 - __NR_Linux);

        ret.v0 = syscall_no_intercept(ctx->v0,
                ctx->a0, ctx->a1, ctx->a2, ctx->a3, ctx->a4, ctx->a5);
        __asm__ __volatile__("move %0, $a3" : "=r" (ret.a3));
        ctx->a3 = ret.a3;

        debug_dump("syscall %d: can_dump=%d\n", ctx->v0, can_dump);
        if (can_dump) {
            record_syscall(g_dumper, (int)ctx->v0, (long*)&ctx->a0, ret.v0, ctx);
        }
        switch (ctx->v0) {
        case SYS_mmap:
            if (ret.v0 >= 0) {
                send_cmd(SYS_update_maps, ctx->a2);
            }
            break;
        case SYS_mremap:
        // case SYS_munmap:
            if (ret.v0 >= 0) {
                send_cmd(SYS_update_maps, 0);
            }
            break;
        default:
            break;
        }
    }
    else {
        // fork before exec reach here!
        debug_dump("syscall %d: begin, args=%lx,%lx,%lx,%lx,%lx,%lx\n",
            ctx->v0, ctx->a0, ctx->a1, ctx->a2, ctx->a3, ctx->a4, ctx->a5);
        ret.v0 = syscall_no_intercept(ctx->v0,
                ctx->a0, ctx->a1, ctx->a2, ctx->a3, ctx->a4, ctx->a5);
        __asm__ __volatile__("move %0, $a3" : "=r" (ret.a3));
        ctx->a3 = ret.a3;
        debug_dump("syscall %d: ret=0x%lx/%ld, args=%lx,%lx,%lx,%lx,%lx,%lx\n",
            ctx->v0, ret.v0, ret.a3,
            ctx->a0, ctx->a1, ctx->a2, ctx->a3, ctx->a4, ctx->a5);
    }

    return ret.v0;
}

__asm(
    ".globl syscall_no_intercept\n\t"
    "syscall_no_intercept:\n\t"
    "movq %rdi, %rax\n\t" /* convert from linux ABI calling */
    "movq %rsi, %rdi\n\t" /* convention to syscall calling convention */
    "movq %rdx, %rsi\n\t"
    "movq %rcx, %rdx\n\t"
    "movq %r8, %r10 \n\t"
    "movq %r9, %r8\n\t"
    "movq 8(%rsp), %r9\n\t"
    "syscall\n\t"
    "ret\n\t"
);

__asm(
    ".globl syscall_wrapper\n\t"
    "syscall_wrapper:\n\t"
    "daddiu $sp, $sp,-512\n\t"
    "gssq $v1,$v0,16($sp)\n\t"
    "gssq $a1,$a0,32($sp)\n\t"
    "gssq $a3,$a2,48($sp)\n\t"
    "gssq $a5,$a4,64($sp)\n\t"
    "gssq $a7,$a6,80($sp)\n\t"
    "gssq $t1,$t0,96($sp)\n\t"
    "gssq $t3,$t2,112($sp)\n\t"
    "gssq $s1,$s0,128($sp)\n\t"
    "gssq $s3,$s2,144($sp)\n\t"
    "gssq $s5,$s4,160($sp)\n\t"
    "gssq $s7,$s6,176($sp)\n\t"
    "gssq $t9,$t8,192($sp)\n\t"
    "gssq $k1,$k0,208($sp)\n\t"
    "gssq $sp,$gp,224($sp)\n\t"
    "gssq $ra,$fp,240($sp)\n\t"
    "ld $t2, 504($sp)\n\t" //load previous ra to t2, see build_trampoline
    "sd $t2, 112($sp)\n\t"
    "move $a0,$sp\n\t"  // load USER_REGS pointer
    "move $s5,$sp\n\t"  // 拷贝 USER_REGS 地址到 s5 寄存器，用于发生崩溃的时候修复 pc,ra,sp
    "lui $t9,0\n\t"
    "ori $t9,$t9,0\n\t"
    "dsll32    $t9,$t9,0\n\t"
    "lui $t1,0\n\t"
    "ori $t1,$t1,0\n\t"
    "dsll32    $t1,$t1,0\n\t"
    "dsrl32    $t1,$t1,0\n\t"
    "or    $t9,$t9,$t1\n\t"
    "jalr $t9\n\t"      // call syscall_hook
    // "gslq $v1,$v0, 16($sp)\n\t"
    "gslq $a1,$a0, 32($sp)\n\t"
    "gslq $a3,$a2, 48($sp)\n\t"
    "gslq $a5,$a4, 64($sp)\n\t"
    "gslq $a7,$a6, 80($sp)\n\t"
    "gslq $t1,$t0, 96($sp)\n\t"
    "gslq $t3,$t2,112($sp)\n\t"
    "gslq $s1,$s0,128($sp)\n\t"
    "gslq $s3,$s2,144($sp)\n\t"
    "gslq $s5,$s4,160($sp)\n\t"
    "gslq $s7,$s6,176($sp)\n\t"
    "gslq $t9,$t8,192($sp)\n\t"
    "gslq $k1,$k0,208($sp)\n\t"
    "gslq $sp,$gp,224($sp)\n\t"
    "gslq $ra,$fp,240($sp)\n\t"
    "daddiu $sp, $sp, 512\n\t"
    "jr $ra\n\t"
    "nop\n\t"
);

// @return: number of dwords
static int store_address_in_t9(uintptr_t target_addr, uint32_t* walk)
{
    uint32_t* head = walk;
    uint32_t low = target_addr;
    uint32_t high = target_addr >> 32;
    // lui $t9(12), upper(high)
    // 0011 1100 0001 1001, imm
    *walk++ = 0x3c190000 | (high >> 16);

    // ori $t9, $t9, lower(high)
    // 0011 0111 0011 1001, imm
    *walk++ = 0x37390000 | (high & 0xffff);

    // dsll32 $t9, $t9, 32
    // 0000 0000 0001 1001 1100 1000 0011 1100
    *walk++ = 0x0019c83c;

    // lui $t1, upper(low)
    // 0011 1100 0000 1101, imm
    *walk++ = 0x3c0d0000 | (low >> 16);

    // ori $t1, $t1, lower(low)
    // 0011 0101 1010 1101, imm
    *walk++ = 0x35ad0000 | (low & 0xffff);

    if (low & 0x80000000) {
        // t1 will sign extend with 1, should clear them
        // dsll32 $t1, $t1, 32
        // 0000 0000 0000 1101 0110 1000 0011 1100
        *walk++ = 0x000d683c;

        // dsrl32 $t1, $t1, 32
        // 0000 0000 0000 1101 0110 1000 0011 1110
        *walk++ = 0x000d683e;
    }

    // t1 will sign extend with 0, just or is ok
    // or $t9, $t9, $t1
    // 0000 0011 0010 1101 1100 1000 0010 0101
    *walk++ = 0x032dc825;

    return (walk - head);
}

// @return: number of dwords
static int store_address_in_t3(uintptr_t target_addr, uint32_t* walk)
{
    uint32_t* head = walk;
    uint32_t low = target_addr;
    uint32_t high = target_addr >> 32;
    // lui $t3(12), upper(high)
    // 0011 1100 0000 l111, imm
    *walk++ = 0x3c0f0000 | (high >> 16);

    // ori $t3, $t3, lower(high)
    // 0011 0101 1110 1111, imm
    *walk++ = 0x35ef0000 | (high & 0xffff);

    // dsll32 $t3, $t3, 32
    // 0000 0000 0000 1111 0111 1000 0011 1100
    *walk++ = 0x000f783c;

    // lui $t1, upper(low)
    // 0011 1100 0000 1101, imm
    *walk++ = 0x3c0d0000 | (low >> 16);

    // ori $t1, $t1, lower(low)
    // 0011 0101 1010 1101, imm
    *walk++ = 0x35ad0000 | (low & 0xffff);

    if (low & 0x80000000) {
        // t1 will sign extend with 1, should clear them
        // dsll32 $t1, $t1, 32
        // 0000 0000 0000 1101 0110 1000 0011 1100
        *walk++ = 0x000d683c;

        // dsrl32 $t1, $t1, 32
        // 0000 0000 0000 1101 0110 1000 0011 1110
        *walk++ = 0x000d683e;
    }

    // t1 will sign extend with 0, just or is ok
    // or $t3, $t3, $t1
    // 0000 0001 1110 1101 0111 1000 0010 0101
    *walk++ = 0x01ed7825;

    return (walk - head);
}

static void fix_syscall_wrapper(uintptr_t target_addr)
{
    uintptr_t head = (uintptr_t)syscall_wrapper;
    int pagesize = sysconf(_SC_PAGESIZE);
    char* base = (char*)(head & ~(pagesize -1));
    int err = syscall_no_intercept(SYS_mprotect, base, pagesize*2, PROT_WRITE|PROT_EXEC|PROT_READ);
    if (!err) {
        uint32_t* walk = (uint32_t*)head;
        for (int i = 0; ; ++i) {
            if (walk[i] == 0x03a02025) {
                walk += i + 1;
                break;
            }
        }

        // $ra is the return address.
        store_address_in_t9(target_addr, walk);
    }

    syscall_no_intercept(SYS_mprotect, base, pagesize*2, PROT_EXEC|PROT_READ);
}

// @return: number of dwords
static int store_address_in_t0(uintptr_t target_addr, uint32_t* walk)
{
    uint32_t* head = walk;
    uint32_t low = target_addr;
    uint32_t high = target_addr >> 32;
    // $ra is the return address.
    // load 64bit address

    // lui $t0(12), upper(high)
    // 0011 1100 0000 1100, imm
    *walk++ = 0x3c0c0000 | (high >> 16);

    // ori $t0, $t0, lower(high)
    // 0011 0101 1000 1100, imm
    *walk++ = 0x358c0000 | (high & 0xffff);

    // dsll32 $t0, $t0, 32
    // 0000 0000 0000 1100 0110 0000 0011 1100
    *walk++ = 0x000c603c;

    // lui $t1, upper(low)
    // 0011 1100 0000 1101, imm
    *walk++ = 0x3c0d0000 | (low >> 16);

    // ori $t1, $t1, lower(low)
    // 0011 0101 1010 1101, imm
    *walk++ = 0x35ad0000 | (low & 0xffff);

    if (low & 0x80000000) {
        // t1 will sign extend with 1, should clear them
        // dsll32 $t1, $t1, 32
        // 0000 0000 0000 1101 0110 1000 0011 1100
        *walk++ = 0x000d683c;

        // dsrl32 $t1, $t1, 32
        // 0000 0000 0000 1101 0110 1000 0011 1110
        *walk++ = 0x000d683e;
    }

    // t1 will sign extend with 0, just or is ok
    // or $t0, $t0, $t1
    // 0000 0001 1000 1101 0110 0000 0010 0101
    *walk++ = 0x018d6025;

    return (walk - head);
}

//@return: number of bytes
static int build_trampoline(uintptr_t stub, uintptr_t hook_wrapper, uintptr_t ret_addr, uint32_t occupied)
{
    uint32_t* walk = (uint32_t*)stub;
    int pagesize = sysconf(_SC_PAGESIZE);
    char* base = (char*)(stub & ~(pagesize -1));

    // backup original $ra to $sp - 8;
    // sd $sp(29) $ra(31), -8
    // 1111 1111 1011 1111  1111 1111 1111 1000
    *walk++ = 0xffbffff8;

    // store address of current PC in t3 used in dump_syscall function
    // for backtrace call stack
    walk += store_address_in_t3(ret_addr - 4, walk);

    walk += store_address_in_t0(hook_wrapper, walk);

    // jarl $t0 // call syscall_wrapper function in preload dynamic shared object
    // 0000 0001 1000 0000 1111 1000 0000 1001
    *walk++ = 0x0180f809;
    *walk++ = 0; //nop

    // restore original $ra from $sp - 8
    // ld $sp(29) $ra(31), -8
    // 1101 1111 1011 1111 1111 1111 1111 1000
    *walk++ = 0xdfbffff8;

    if (occupied > 0) {
        *walk++ = occupied;
        ret_addr += 4;
    }

#if 1
    *walk++ = ((ret_addr & 0x0fffffff) >> 2) | 0x08000000;
#else
    walk += store_address_in_t0(ret_addr, walk);

    // jr $t0 // jump to ret_addr
    // 0000 0001 1000 0000 0000 0000 0000 1000
    *walk++ = 0x01800008;
#endif
    *walk++ = 0; //nop

    return (uint8_t*)walk - (uint8_t*)stub;
}

#define is_in_br_range(pc, target) ((pc>>28) == (target>>28))

static int hack_syscall(uintptr_t pc, int is_jmp_insn)
{
    if (!is_in_br_range(pc, (uintptr_t)trampoline_addr)) {
        trampoline_addr = find_gap_page_near_pc(pc, false);
        if (!is_in_br_range(pc, (uintptr_t)trampoline_addr)) {
            trampoline_addr = find_gap_page_near_pc(pc, true);
            if (!is_in_br_range(pc, (uintptr_t)trampoline_addr)) {
                return -1;
            }
        }

        trampoline_addr_end = trampoline_addr + STUB_SIZE;
    }

    int pagesize = sysconf(_SC_PAGESIZE);
    char* base = (char*)(pc & ~(pagesize -1));
    uintptr_t trampoline = (uintptr_t)trampoline_addr;
    uint32_t occupied = *(uint32_t*)(pc + 4);
    int err = syscall_no_intercept(SYS_mprotect, base, pagesize*2, PROT_WRITE|PROT_EXEC|PROT_READ);
    if (!err) {
        // NOTE: jal will overwrite ra cause dead loop, so only can use j
        // j trampoline, jmp range is 256MB.
        *(uint32_t*)pc = (((uintptr_t)trampoline & 0x0fffffff) >> 2) | 0x08000000;

        if (occupied && !is_jmp_insn) {
            //insert NOP to fill into delay slot
            occupied = *(uint32_t*)(pc + 4);
            *(uint32_t*)(pc + 4) = 0;
        }
        else {
            occupied = 0;
        }
    }
    else {
        debug_dump("failed to mprotect for %lx:%lx\n", pc, trampoline);
        return 0;
    }
    syscall_no_intercept(SYS_mprotect, base, pagesize*2, PROT_EXEC|PROT_READ);

    return build_trampoline(trampoline, (uintptr_t)syscall_wrapper, pc + 4, occupied);
}

// enum all syscall instruction

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

    syscall_no_intercept(SYS_write, 2, buf, len);
}

void xabort_errno(int error_code, const char *msg)
{
    static const char main_msg[] = "error:\n";

    if (msg != NULL) {
        /* not using libc - inline strlen */
        size_t len = 0;
        while (msg[len] != '\0')
            ++len;
        syscall_no_intercept(SYS_write, 2, msg, len);
    }

    if (error_code != 0) {
        char buf[0x10];
        size_t len = 1;
        char *c = buf + sizeof(buf) - 1;

        /* not using libc - inline sprintf */
        do {
            *c-- = error_code % 10;
            ++len;
            error_code /= 10;
        } while (error_code != 0);
        *c = ' ';

        syscall_no_intercept(SYS_write, 2, c, len);
    }

    syscall_no_intercept(SYS_write, 2, main_msg, sizeof(main_msg) - 1);
    syscall_no_intercept(SYS_exit_group, 1);

    __builtin_unreachable();
}

static inline int syscall_error_code(long result)
{
    if (result < 0 && result >= -0x1000)
        return (int)-result;

    return 0;
}

void xabort_on_syserror(long syscall_result, const char *msg)
{
    if (syscall_error_code(syscall_result) != 0)
        xabort_errno(syscall_error_code(syscall_result), msg);
}

void xabort(const char *msg)
{
    xabort_errno(0, msg);
}

void* xmmap_anon(size_t size, void* expect_addr)
{
    long addr = syscall_no_intercept(SYS_mmap,
                expect_addr, size,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, (off_t)0);
    if (expect_addr != NULL) {
        if (expect_addr != (void*)addr) {
            debug_dump("expect_addr %p, but got %p, size=0x%x\n",
                    expect_addr, addr, size);
        }
    }

    xabort_on_syserror(addr, __func__);

    return (void *) addr;
}

void* xmremap(void *addr, size_t old, size_t new_size)
{
    long new_addr = syscall_no_intercept(SYS_mremap, addr,
                old, new_size, MREMAP_MAYMOVE);

    xabort_on_syserror(new_addr, __func__);

    return (void *) new_addr;
}

void xmunmap(void *addr, size_t len)
{
    long result = syscall_no_intercept(SYS_munmap, addr, len);

    xabort_on_syserror(result, __func__);
}

long xlseek(long fd, unsigned long off, int whence)
{
    long result = syscall_no_intercept(SYS_lseek, fd, off, whence);

    xabort_on_syserror(result, __func__);

    return result;
}

void xread(long fd, void *buffer, size_t size)
{
    long result = syscall_no_intercept(SYS_read, fd, buffer, size);

    if (result != (long)size)
        xabort_errno(syscall_error_code(result), __func__);
}

struct intercept_desc {
    const char *path;
    unsigned char *base_addr;
    unsigned long text_offset;
    unsigned char *text_start;
    unsigned char *text_end;
    Elf64_Half text_section_index;
    int func_count;
    int* func_table;
};

/*
 * find_sections
 *
 * See: man elf
 */
static void find_sections(struct intercept_desc* desc, int fd)
{
    Elf64_Ehdr elf_header;

    xread(fd, &elf_header, sizeof(elf_header));

    Elf64_Shdr sec_headers[elf_header.e_shnum];

    xlseek(fd, elf_header.e_shoff, SEEK_SET);
    xread(fd, sec_headers, elf_header.e_shnum * sizeof(Elf64_Shdr));

    char sec_string_table[sec_headers[elf_header.e_shstrndx].sh_size];

    xlseek(fd, sec_headers[elf_header.e_shstrndx].sh_offset, SEEK_SET);
    xread(fd, sec_string_table,
        sec_headers[elf_header.e_shstrndx].sh_size);

    bool text_section_found = false;

    for (Elf64_Half i = 0; i < elf_header.e_shnum; ++i) {
        const Elf64_Shdr *section = &sec_headers[i];
        char *name = sec_string_table + section->sh_name;

        debug_dump("looking at section: \"%s\" type: %ld\n",
            name, (long)section->sh_type);
        if (strcmp(name, ".text") == 0) {
            text_section_found = true;
            desc->text_offset = section->sh_offset;
            desc->text_start = desc->base_addr + section->sh_addr;
            desc->text_end = desc->text_start + section->sh_size - 1;
            desc->text_section_index = i;
        } else if (section->sh_type == SHT_SYMTAB ||
            section->sh_type == SHT_DYNSYM) {
            debug_dump("found symbol table: %s\n", name);
            // add_table_info(&desc->symbol_tables, section);
        } else if (section->sh_type == SHT_RELA) {
            debug_dump("found relocation table: %s\n", name);
            // add_table_info(&desc->rela_tables, section);
        }
    }

    if (!text_section_found)
        xabort("text section not found");
}

struct intercept_disasm_context {
    csh handle;
    cs_insn *insn;
    const unsigned char *begin;
    const unsigned char *end;
};

struct intercept_disasm_result {
    bool is_syscall;
    /* Length in bytes, zero if disasm was not successful. */
    unsigned char length;
    uint32_t mnemonic;
};

int nop_vsnprintf(char *str, size_t size, const char *format, va_list ap) { return 0; }

/*
 * intercept_disasm_init -- should be called before disassembling a region of
 * code. The context created contains the context capstone needs ( or generally
 * the underlying disassembling library, if something other than capstone might
 * be used ).
 *
 * One must pass this context pointer to intercept_disasm_destroy following
 * a disassembling loop.
 */
struct intercept_disasm_context *
intercept_disasm_init(struct intercept_disasm_context *context,
        const unsigned char *begin, const unsigned char *end)
{
    // context = (struct intercept_disasm_context*)xmmap_anon(sizeof(*context));
    context->begin = begin;
    context->end = end;

    /*
     * Initialize the disassembler.
     * The handle here must be passed to capstone each time it is used.
     */
#if defined(__x86_64__)
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &context->handle) != CS_ERR_OK)
#else
    if (1)
#endif
        xabort("cs_open");

    cs_option(context->handle, CS_OPT_SKIPDATA, CS_OPT_ON);

    /*
     * Kindly ask capstone to return some details about the instruction.
     * Without this, it only prints the instruction, and we would need
     * to parse the resulting string.
     */
    if (cs_option(context->handle, CS_OPT_DETAIL, CS_OPT_ON) != 0)
        xabort("cs_option - CS_OPT_DETAIL");

    /*
     * Overriding the printing routine used by capstone,
     * see comments above about nop_vsnprintf.
     */
    cs_opt_mem x;
    x.malloc = malloc;
    x.free = free;
    x.calloc = calloc;
    x.realloc = realloc;
    x.vsnprintf = nop_vsnprintf;
    if (cs_option(context->handle, CS_OPT_MEM, (size_t)&x) != 0)
        xabort("cs_option - CS_OPT_MEM");

    if ((context->insn = cs_malloc(context->handle)) == NULL)
        xabort("cs_malloc");

    return context;
}

/*
 * intercept_disasm_destroy -- see comments for above routine
 */
void
intercept_disasm_destroy(struct intercept_disasm_context *context)
{
    cs_free(context->insn, 1);
    cs_close(&context->handle);
}

static bool is_jmp(struct intercept_disasm_context *context)
{
    bool jmp = false;
    switch(context->insn->id) {
        case MIPS_INS_BEQ:
        case MIPS_INS_BEQC:
        case MIPS_INS_BEQL:
        case MIPS_INS_BEQZALC:
        case MIPS_INS_BEQZC:
        case MIPS_INS_BGEC:
        case MIPS_INS_BGEUC:
        case MIPS_INS_BGEZ:
        case MIPS_INS_BGEZAL:
        case MIPS_INS_BGEZALC:
        case MIPS_INS_BGEZALL:
        case MIPS_INS_BGEZALS:
        case MIPS_INS_BGEZC:
        case MIPS_INS_BGEZL:
        case MIPS_INS_BGTZ:
        case MIPS_INS_BGTZALC:
        case MIPS_INS_BGTZC:
        case MIPS_INS_BGTZL:
        case MIPS_INS_BLEZ:
        case MIPS_INS_BLEZALC:
        case MIPS_INS_BLEZC:
        case MIPS_INS_BLEZL:
        case MIPS_INS_BLTC:
        case MIPS_INS_BLTUC:
        case MIPS_INS_BLTZ:
        case MIPS_INS_BLTZAL:
        case MIPS_INS_BLTZALC:
        case MIPS_INS_BLTZALL:
        case MIPS_INS_BLTZALS:
        case MIPS_INS_BLTZC:
        case MIPS_INS_BLTZL:
        case MIPS_INS_BNE:
        case MIPS_INS_BNEC:
        case MIPS_INS_BNEGI:
        case MIPS_INS_BNEG:
        case MIPS_INS_BNEL:
        case MIPS_INS_BNEZALC:
        case MIPS_INS_BNEZC:
        case MIPS_INS_BNVC:
        case MIPS_INS_BNZ:
        case MIPS_INS_BZ:
        case MIPS_INS_BEQZ:
        case MIPS_INS_B:
        case MIPS_INS_BNEZ:
        case MIPS_INS_J:
        case MIPS_INS_JR:
        case MIPS_INS_NOP:
            jmp = true;
            break;
        default:
            jmp = false;
            break;
    }

    return jmp;
}

/*
 * intercept_disasm_next_instruction - Examines a single instruction
 * in a text section. This is only a wrapper around capstone specific code,
 * collecting data that can be used later to make decisions about patching.
 */
struct intercept_disasm_result
intercept_disasm_next_instruction(struct intercept_disasm_context *context,
                    const unsigned char *code)
{
    struct intercept_disasm_result result = {0, };
    const unsigned char *start = code;
    size_t size = (size_t)(context->end - code + 1);
    uint64_t address = (uint64_t)code;

    if (!cs_disasm_iter(context->handle, &start, &size,
        &address, context->insn)) {
        result.length = 0;
        return result;
    }

    result.length = context->insn->size;
	result.is_syscall = (context->insn->id == X86_INS_SYSCALL);
    result.mnemonic = context->insn->id;

    return result;
}

static int get_syscall_no(unsigned char* code)
{
    int no = *(int*)code;
    return no;
}

static bool get_function_range(struct intercept_desc* desc,
        unsigned char* code, unsigned char** begin, unsigned char** end)
{
    int offset = (code - desc->base_addr);
    for (int i = 0; i < desc->func_count; ++i) {
        if ( offset >= desc->func_table[i] && offset < desc->func_table[i+1]) {
            *begin = desc->base_addr + desc->func_table[i];
            *end = desc->base_addr + desc->func_table[i+1] - 4;
            return true;
        }
    }

    return false;
}

static bool is_valid_syscall_no(unsigned char* code)
{
    if (code[-5] == 0xb8) {
        int number = *(int *)(code - 4);
        /* disable hook SYS_clone=0x38, SYS_vfork=0x3a, SYS_rt_sigreturn=0xf */
        if (SYS_clone == number ||
            SYS_vfork == number ||
            SYS_rt_sigreturn == number ||
            number < 0 ||
            number > SYS_pkey_free) {
            return false;
        }

        return true;
    }

    // NOTE: ignore syscall like in function syscall@@GLIBC_2.2.5
    // Should trap these syscall into syscall-stop and seccomp
    return false;
}

/*
 * crawl_text
 * Crawl the text section, disassembling it all.
 * This routine collects information about potential addresses to patch.
 *
 * The addresses of all syscall instructions are stored, together with
 * a description of the preceding, and following instructions.
 *
 * A lookup table of all addresses which appear as jump destination is
 * generated, to help determine later, whether an instruction is suitable
 * for being overwritten -- of course, if an instruction is a jump destination,
 * it can not be merged with the preceding instruction to create a
 * new larger one.
 *
 * Note: The actual patching can not yet be done in this disassembling phase,
 * as it is not known in advance, which addresses are jump destinations.
 */
static void crawl_text(struct intercept_desc *desc)
{
#define SYSCALL_INS_SIZE 2

    unsigned char *code = desc->text_start;

    struct intercept_disasm_context ctx, *context = &ctx;
    struct intercept_disasm_result result = {0,};

    intercept_disasm_init(context, desc->text_start, desc->text_end);

    while (code <= desc->text_end) {
		struct intercept_disasm_result result;
		result = intercept_disasm_next_instruction(context, code);
		if (result.length == 0) {
			++code;
			continue;
		}

        if (result.is_syscall && is_valid_syscall_no(code - SYSCALL_INS_SIZE)) {
            int len = hack_syscall((uintptr_t)code, result.is_jump);
            if (len > 0) {
                debug_dump("hack:%p(%d) to %p,%d\n",
                        code, get_syscall_no(code - SYSCALL_INS_SIZE), trampoline_addr, len);
                ++hook_ok;
                trampoline_addr += len;
                if (trampoline_addr + len + 4 > trampoline_addr_end) {
                    // current block is full, need allocate new block
                    trampoline_addr = NULL;
                    trampoline_addr_end = NULL;
                }
            }
            else {
                ++hook_failed;
                debug_dump("failed to hack:%p(%d) -> %d\n",
                        code, get_syscall_no(code - SYSCALL_INS_SIZE), len);
                break; //disable hook this so
            }
        }
        else {
            debug_dump("skip hack:%p(%d) to %p\n",
                    code, get_syscall_no(code - SYSCALL_INS_SIZE), trampoline_addr);
        }

        code +=result.length;
    }
    intercept_disasm_destroy(context);
    debug_dump("hook finished (ok=%d, failed=%d)\n", hook_ok, hook_failed);
}

void find_syscalls(struct intercept_desc* desc, const char* path)
{
    debug_dump("find_syscalls in %s "
        "at base_addr 0x%016" PRIxPTR "\n",
        path,
        (uintptr_t)desc->base_addr);

    int fd = syscall_no_intercept(SYS_open, path, O_RDONLY);

    find_sections(desc, fd);

    debug_dump(
        "%s .text mapped at 0x%016" PRIxPTR " - 0x%016" PRIxPTR " \n",
        desc->path,
        (uintptr_t)desc->text_start,
        (uintptr_t)desc->text_end);

    syscall_no_intercept(SYS_close, fd);

    if (!desc->func_table) {
        // load function range table
        // table file generated by this commands
        // objdump -t xx | grep -- "\.text" | awk '{ print $1}' | sort | uniq
        // xx is a debug file in /usr/lib/debug/.build-id/xx/yy..
        char path[256];
        FILE* pf = NULL;
        char* basename = strrchr(desc->path, '/');
        if (basename) {
            snprintf(path, sizeof(path), "%s/.config/%s.table",
                getenv("HOME"), basename + 1);
        }
        else {
            snprintf(path, sizeof(path), "%s/.config/%s.table",
                getenv("HOME"), desc->path);
        }
        pf = fopen(path, "rb");
        if (pf) {
            int size = 0;
            fseek(pf, 0, SEEK_END);
            size = ftell(pf);
            fseek(pf, 0, SEEK_SET);
            desc->func_count = size/17;
            if (desc->func_count > 0) {
                desc->func_table = (int*)malloc(desc->func_count*sizeof(int) + sizeof(int));
                for (int i = 0; i<desc->func_count; ++i) {
                    fgets(path, sizeof(path), pf);
                    desc->func_table[i] = strtol(path, NULL, 16);
                }
                desc->func_table[desc->func_count] = desc->text_end - desc->base_addr;
            }
            fclose(pf);
        }
        else {
            desc->func_count = 0;
            debug_dump("failed to open %s\n", path);
        }
    }

    crawl_text(desc);
}

/*
 * get_any_used_vaddr - find a virtual address that is expected to
 * be a used for the object file mapped into memory.
 *
 * An Elf64_Phdr struct contains information about a segment in an on object
 * file. This routine looks for a segment with type LOAD, that has a non-zero
 * size in memory. The p_vaddr field contains the virtual address where this
 * segment should be loaded to. This of course is relative to the base address.
 *
 * typedef struct
 * {
 *   Elf64_Word p_type;            Segment type
 *   Elf64_Word p_flags;        Segment flags
 *   Elf64_Off p_offset;        Segment file offset
 *   Elf64_Addr p_vaddr;        Segment virtual address
 *   Elf64_Addr p_paddr;        Segment physical address
 *   Elf64_Xword p_filesz;        Segment size in file
 *   Elf64_Xword p_memsz;        Segment size in memory
 *   Elf64_Xword p_align;        Segment alignment
 * } Elf64_Phdr;
 *
 *
 */
static uintptr_t get_any_used_vaddr(const struct dl_phdr_info *info)
{
    const Elf64_Phdr *pheaders = info->dlpi_phdr;

    for (Elf64_Word i = 0; i < info->dlpi_phnum; ++i) {
        if (pheaders[i].p_type == PT_LOAD && pheaders[i].p_memsz != 0)
            return info->dlpi_addr + pheaders[i].p_vaddr;
    }

    return 0; /* not found */
}

/*
 * get_name_from_proc_maps
 * Tries to find the path of an object file loaded at a specific
 * address.
 *
 * The paths found are stored in BSS, in the paths variable. The
 * returned pointer points into this variable. The next_path
 * pointer keeps track of the already "allocated" space inside
 * the paths array.
 */
static const char* get_name_from_proc_maps(uintptr_t addr)
{
    static char paths[0x10000];
    static char *next_path = paths;
    const char *path = NULL;

    char line[0x2000];
    FILE *maps;

    if ((next_path >= paths + sizeof(paths) - sizeof(line)))
        return NULL; /* No more space left */

    if ((maps = fopen("/proc/self/maps", "r")) == NULL)
        return NULL;

    while ((fgets(line, sizeof(line), maps)) != NULL) {
        unsigned char *start;
        unsigned char *end;

        /* Read the path into next_path */
        if (sscanf(line, "%p-%p %*s %*x %*x:%*x %*u %s",
            (void **)&start, (void **)&end, next_path) != 3)
            continue;

        if (addr < (uintptr_t)start)
            break;

        if ((uintptr_t)start <= addr && addr < (uintptr_t)end) {
            /*
             * Object found, setting the return value.
             * Adjusting the next_path pointer to point past the
             * string found just now, to the unused space behind it.
             * The next string found (if this routine is called
             * again) will be stored there.
             */
            path = next_path;
            next_path += strlen(next_path) + 1;
            break;
        }
    }

    fclose(maps);

    return path;
}


/*
 * get_object_path - attempt to find the path of the object in the
 * filesystem.
 *
 * This is usually supplied by dl_iterate_phdr in the dl_phdr_info struct,
 * but sometimes that does not contain it.
 */
static const char* get_object_path(const struct dl_phdr_info *info)
{
    if (info->dlpi_name != NULL && info->dlpi_name[0] != '\0') {
        return info->dlpi_name;
    } else {
        uintptr_t addr = get_any_used_vaddr(info);
        if (addr == 0)
            return NULL;
        return get_name_from_proc_maps(addr);
    }
}

static bool is_vdso(uintptr_t addr, const char *path)
{
    return addr == (uintptr_t)vdso_addr || strstr(path, "vdso") != NULL;
}

static bool g_hook_vdso = false;
static int (*g_clock_gettime)(clockid_t clk_id, void *tp) = NULL;
static int (*g_gettimeofday)(struct timeval *tv, void *tz) = NULL;

static void
locate_vdso_functions(uintptr_t addr)
{
    char *fd = (char *)addr;
    Elf64_Ehdr elf_header;

    memcpy(&elf_header, fd, sizeof(elf_header));

    Elf64_Shdr sec_headers[elf_header.e_shnum];

    fd = (char *)addr + elf_header.e_shoff;
    memcpy(sec_headers, fd, elf_header.e_shnum * sizeof(Elf64_Shdr));

    char sec_string_table[sec_headers[elf_header.e_shstrndx].sh_size];

    fd = (char *)addr + sec_headers[elf_header.e_shstrndx].sh_offset;
    memcpy(sec_string_table, fd,
        sec_headers[elf_header.e_shstrndx].sh_size);

    int count = 0;
    Elf64_Sym *item = NULL;
    char *strtab = NULL;
    uintptr_t target = 0;
    for (Elf64_Half i = 0; i < elf_header.e_shnum; ++i) {
        const Elf64_Shdr *section = &sec_headers[i];
        char *name = sec_string_table + section->sh_name;

        if (section->sh_type == SHT_STRTAB && NULL == strtab) {
            strtab = (char *)addr + section->sh_addr;
            debug_dump("found syttab at section: \"%s\" addr: %lx\n",
                name, section->sh_addr);
        } else if (section->sh_type == SHT_DYNSYM) {
            count = section->sh_size / sizeof(Elf64_Sym);
            item = (Elf64_Sym *)((char *)addr + section->sh_addr);
            debug_dump("found dynsym at section: \"%s\" addr: %lx, count:%d\n",
                name, section->sh_addr, count);
        }
    }

    for (int i = 0; i < count; ++i) {
        if (item->st_value > 0) {
            debug_dump("enum:%d, %s, %lx\n",
                i, strtab + item->st_name, item->st_value);

            target = item->st_value + addr;

            if (0 == strcmp(strtab + item->st_name, "__vdso_gettimeofday")) {
                memcpy(&g_gettimeofday, &target, sizeof(uintptr_t));
            } else if (0 == strcmp(strtab + item->st_name, "__vdso_clock_gettime")) {
                memcpy(&g_clock_gettime, &target, sizeof(uintptr_t));
            }
        }
        ++item;
    }
}

/*
 * str_match - matching library names.
 * The first string (name) is not null terminated, while
 * the second string (expected) is null terminated.
 * This allows matching e.g.: "libc-2.25.so\0" with "libc\0".
 * If name_len is 4, the comparison is between: "libc" and "libc".
 */
static bool str_match(const char *name, size_t name_len,
        const char *expected)
{
    return name_len == strlen(expected) &&
        strncmp(name, expected, name_len) == 0;
}

static const char* get_lib_short_name(const char *name)
{
    const char *slash = strrchr(name, '/');
    if (slash != NULL)
        name = slash + 1;

    return name;
}

static char *get_line(char *buf, int size)
{
    for (int i = 0; i < size; ++i) {
        if (buf[i] == '\n') {
            return buf + i;
        }
    }

    return NULL;
}

#define MAX_CACHED (100)
unsigned char* find_gap_page_near_pc(uintptr_t pc, bool search_after_libc)
{
    // search all /proc/#pid/maps
    char buf[4096];
    uintptr_t segments[MAX_CACHED];
    int count = 0;
    char *line = NULL;
    char *line_end = NULL;
    char *stop = NULL;
    uintptr_t start = 0;
    uintptr_t end = 0;
    uintptr_t gap_start = 0;
    uintptr_t gap_end = 0;
    unsigned char* found = NULL;
    int eof = 0;
    int size = 256;
    int remain = 0;
    int fd = syscall_no_intercept(SYS_open, "/proc/self/maps", O_RDONLY);
    if (fd <= 0) {
        debug_dump("failed find gap:%lx,%lx -> %d\n", gap_start, gap_end, errno);
        return NULL;
    }

    while (!eof) {
        int bytes = syscall_no_intercept(SYS_read,
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

            if (!search_after_libc) {
                // need cached
                if (count < MAX_CACHED) {
                    segments[count] = start;
                    segments[count + 1] = end;
                    count += 2;
                }
                else {
                    // remove the first segment
                    memmove(&segments[0], &segments[2],
                            sizeof(segments) - 2*sizeof(segments[0]));
                    segments[MAX_CACHED-2] = start;
                    segments[MAX_CACHED-1] = end;
                }
            }

            if (start <= pc && pc <= end) {
                if (!search_after_libc) {
                    eof = 1;
                    gap_end = start;
                    gap_start = gap_end - STUB_SIZE;
                    debug_dump("%d gap0:%p,%p; pc=%p\n", count, (void*)gap_start, (void*)gap_end, (void*)pc);
                    break;
                }

                gap_start = end;
                gap_end = gap_start + STUB_SIZE;
                debug_dump("gap1:%p,%p; pc=%p\n", (void*)gap_start, (void*)gap_end, (void*)pc);
            }
            else if (gap_start > 0) {
                if ((gap_end <= start) && is_in_br_range(pc, gap_start)) {
                    eof = 1;
                    found = (unsigned char*)xmmap_anon(gap_end - gap_start, (void*)gap_start);
                    break;
                }
                gap_start = end;
                gap_end = gap_start + STUB_SIZE;
                debug_dump("gap2:%p,%p; pc=%p\n", (void*)gap_start, (void*)gap_end, (void*)pc);
                if (!is_in_br_range(pc, gap_start)) {
                    eof = 1; // will never match from here!
                    break;
                }
            }

            // skip to next line
            remain -= (line_end + 1 - line);
            line = line_end + 1;
        }
    }

    if (!search_after_libc && count > 3) {
        for (int i = count - 3; i >= 0; i -=2) {
            if ((gap_start >= segments[i]) && is_in_br_range(pc, gap_start)) {
                debug_dump("%d gap4:%p,%p; pc=%p\n", i, (void*)gap_start, (void*)gap_end, (void*)pc);
                found = (unsigned char*)xmmap_anon(gap_end - gap_start, (void*)gap_start);
                break;
            }
            gap_end = segments[i - 1];
            gap_start = gap_end - STUB_SIZE;
            debug_dump("gap3:%p,%p; pc=%p\n", (void*)gap_start, (void*)gap_end, (void*)pc);
            if (!is_in_br_range(pc, gap_start)) {
                eof = 1; // will never match from here!
                break;
            }
        }
    }

    syscall_no_intercept(SYS_close, fd);

    if (!found) {
        debug_dump("failed to find gap:pc=%p, search_after_libc=%d\n",
                (void*)pc, search_after_libc);
    }

    return found;
}

unsigned char* dump_maps(void)
{
    // search all /proc/#pid/maps
    char buf[4096];
    int count = 0;
    char *line = NULL;
    char *line_end = NULL;
    char *stop = NULL;
    uintptr_t start = 0;
    uintptr_t end = 0;
    uintptr_t gap_start = 0;
    uintptr_t gap_end = 0;
    unsigned char* found = NULL;
    int eof = 0;
    int size = 256;
    int remain = 0;
    int fd = syscall_no_intercept(SYS_open, "/proc/self/maps", O_RDONLY);
    if (fd <= 0) {
        debug_dump("failed find gap:%lx,%lx -> %d\n", gap_start, gap_end, errno);
        return NULL;
    }

    while (!eof) {
        int bytes = syscall_no_intercept(SYS_read,
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

            *line_end = 0;
            debug_dump("%s\n", line);

            // skip to next line
            remain -= (line_end + 1 - line);
            line = line_end + 1;
        }
    }

    syscall_no_intercept(SYS_close, fd);

    return found;
}

/*
 * should_patch_object
 * Decides whether a particular loaded object should should be targeted for
 * hotpatching.
 * Always skipped: [vdso], and the syscall_intercept library itself.
 * Besides these two, if patch_all_objs is true, everything object is
 * a target. When patch_all_objs is false, only libraries that are parts of
 * the glibc implementation are targeted, i.e.: libc and libpthread.
 */
static bool should_patch_object(uintptr_t addr, const char *path)
{
    static uintptr_t self_addr;
    if (self_addr == 0) {
        Dl_info self;
        if (!dladdr((void *)&syscall_no_intercept, &self))
            xabort("self dladdr failure");
        self_addr = (uintptr_t)self.dli_fbase;
    }

    static const char libc[] = "libc";
    static const char pthr[] = "libpthread";
    static const char caps[] = "libcapstone";

    if (is_vdso(addr, path)) {
        locate_vdso_functions(addr);
        debug_dump(" - skipping: is_vdso\n");
        return false;
    }

    const char *name = get_lib_short_name(path);
    size_t len = strcspn(name, "-.");

    if (len == 0)
        return false;

    if (addr == self_addr) {
        debug_dump(" - skipping: matches self\n");
        return false;
    }

    if (str_match(name, len, caps)) {
        debug_dump(" - skipping: matches capstone\n");
        return false;
    }

    if (str_match(name, len, libc)) {
        debug_dump(" - libc found\n");
        libc_found = true;
        return true;
    }

    if (patch_all_objs)
        return true;

    if (str_match(name, len, pthr)) {
        debug_dump(" - libpthread found\n");
        return true;
    }

    debug_dump(" - skipping, patch_all_objs == false\n");
    return false;
}

static int analyze_object(struct dl_phdr_info *info, size_t size, void *data)
{
    (void) data;
    (void) size;
    const char *path;

    debug_dump("analyze_object called on \"%s\" at 0x%016" PRIxPTR "\n",
        info->dlpi_name, info->dlpi_addr);

    if ((path = get_object_path(info)) == NULL)
        return 0;

    debug_dump("analyze %s\n", path);

    if (!should_patch_object(info->dlpi_addr, path))
        return 0;

    // struct intercept_desc *patches = allocate_next_obj_desc();
    struct intercept_desc patches;
    memset(&patches, 0, sizeof(patches));
    patches.base_addr = (uint8_t*)info->dlpi_addr;
    patches.path = path;
    find_syscalls(&patches, path);
    // allocate_trampoline_table(&patches);
    // create_patch_wrappers(&patches);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

//override glibc's API
#if 0
int _brk(void* addr)
{
    int ret = syscall_no_intercept(SYS_brk, addr);
    return ret;
}
#endif

static const char* cmdline = NULL;
static int cmdline_match(const char *filter)
{
    if (filter == NULL)
        return 1;

    size_t flen = strlen(filter);
    size_t clen = strlen(cmdline);

    if (flen > clen)
        return 0; /* cmdline can't contain filter */

    /*
     * If cmdline is longer, it must end with a slash + filter:
     * "./somewhere/a.out" matches "a.out"
     * "./a.out" matches "a.out"
     * "./xa.out" does not match "a.out"
     *
     * Of course if cmdline is not longer, the slash is not needed:
     * "a.out" matches "a.out"
     */
    if (clen > flen && cmdline[clen - flen - 1] != '/')
        return 0;

    return strcmp(cmdline + clen - flen, filter) == 0;
}


static int syscall_hook_in_process_allowed(void)
{
    static bool is_decided;
    static int result;

    if (is_decided)
        return result;

    if (cmdline == NULL)
        return 0;

    result = cmdline_match(getenv("INTERCEPT_HOOK_CMDLINE_FILTER"));
    is_decided = true;

    return result;
}

static void __attribute__((constructor)) intercept(int argc, char **argv)
{

    //NOTE: 3.10.0 #1 SMP PREEMPT Fri Dec 14 15:20:30 CST 2018 00022-g5b64ed3 mips64
    //argv[0] always point to "/bin/bash"
    //char path[PATH_MAX];
    //snprintf(path, sizeof(path), "/proc/%d/exe", syscall_no_intercept(SYS_getpid));
    //readlink(path, cmdline, PATH_MAX);
    cmdline = argv[0];

    debug_dumps_on = getenv("ST2_DEBUG_SYSCALL") != NULL;
    if (!syscall_hook_in_process_allowed()) {
        debug_dump("syscall_hook_in_process_allowed=false, %s\n", cmdline);
        return;
    }

    fix_syscall_wrapper((uintptr_t)syscall_hook);

    g_hook_vdso = getenv("ST2_HOOK_VDSO") != NULL;

    vdso_addr = (void *)(uintptr_t)getauxval(AT_SYSINFO_EHDR);
    dl_iterate_phdr(analyze_object, NULL);

    if (debug_dumps_on) {
        dump_maps();
    }
}

static void __attribute__((destructor)) uninit_process(void)
{
}

static void syscall_dump(USER_REGS* ctx, long result)
{
    long pc = 0;
    __asm__ __volatile__("move %0, $ra" : "=r" (pc));

    if (g_dumper && g_dumper->pid == syscall_no_intercept(SYS_getpid)) {
        ctx->t2 = ctx->ra;
        ctx->t3 = pc;
        record_syscall(g_dumper, (int)ctx->v0, (long*)&ctx->a0, result, ctx);
    }
}

#define syscall_arg2(no, arg1, arg2)  do {               \
    __asm(                                               \
    "daddiu $sp, $sp,-512\n\t"                           \
    "gssq $v1,$v0,16($sp)\n\t"                           \
    "gssq $a1,$a0,32($sp)\n\t"                           \
    "gssq $a3,$a2,48($sp)\n\t"                           \
    "gssq $a5,$a4,64($sp)\n\t"                           \
    "gssq $a7,$a6,80($sp)\n\t"                           \
    "gssq $t1,$t0,96($sp)\n\t"                           \
    "gssq $t3,$t2,112($sp)\n\t"                          \
    "gssq $s1,$s0,128($sp)\n\t"                          \
    "gssq $s3,$s2,144($sp)\n\t"                          \
    "gssq $s5,$s4,160($sp)\n\t"                          \
    "gssq $s7,$s6,176($sp)\n\t"                          \
    "gssq $t9,$t8,192($sp)\n\t"                          \
    "gssq $k1,$k0,208($sp)\n\t"                          \
    "gssq $sp,$gp,224($sp)\n\t"                          \
    "gssq $ra,$fp,240($sp)\n\t"                          \
     );                                                  \
    __asm__ __volatile__("move %0, $sp" : "=r" (ctx));   \
    ctx->v0 = no;                                        \
    assert((long)ctx == ctx->sp);                        \
}while(0)

#pragma GCC visibility push(default)
int clock_gettime(clockid_t clk_id, /*struct timespec*/void *tp)
{
    int ret = 0;
    USER_REGS* ctx = NULL;
    if (g_hook_vdso) {
        syscall_arg2(SYS_clock_gettime, clk_id, tp);
    }
    if (g_clock_gettime) ret = g_clock_gettime(clk_id, tp);
    else ret = syscall_no_intercept(SYS_clock_gettime, clk_id, tp);
    if (g_hook_vdso) {
        syscall_dump(ctx, ret);
        __asm__ __volatile__ ("daddiu $sp, $sp,512\n\t");
    }
    return ret;
}

int gettimeofday(/*struct timeval*/void *tv, /*struct timezone*/void *tz)
{
    int ret = 0;
    USER_REGS* ctx = NULL;
    if (g_hook_vdso) {
        syscall_arg2(SYS_gettimeofday, tv, tz);
    }
    if (g_gettimeofday) ret = g_gettimeofday(tv, tz);
    else ret = syscall_no_intercept(SYS_gettimeofday, tv, tz);
    if (g_hook_vdso) {
        syscall_dump(ctx, ret);
        __asm__ __volatile__ ("daddiu $sp, $sp,512\n\t");
    }
    return ret;
}
#pragma GCC visibility pop
