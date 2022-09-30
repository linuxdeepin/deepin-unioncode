/*
cpu: phytium FT1500a

kernel: Linux kylin-laptop 4.4.58-20170818.kylin.5.desktop-generic #5 SMP Fri
        Aug 18 10:19:03 CST 2017 aarch64 aarch64 aarch64 GNU/Linux

compiler:
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/aarch64-linux-gnu/5/lto-wrapper
Target: aarch64-linux-gnu
Configured with: ../src/configure -v --with-pkgversion='Ubuntu/Linaro 5.3.1-14kord4' --with-bugurl=file:///usr/share/doc/gcc-5/README.Bugs --enable-languages=c,ada,c++,java,go,d,fortran,objc,obj-c++ --prefix=/usr --program-suffix=-5 --enable-shared --enable-linker-build-id --libexecdir=/usr/lib --without-included-gettext --enable-threads=posix --libdir=/usr/lib --enable-nls --with-sysroot=/ --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --with-default-libstdcxx-abi=new --enable-gnu-unique-object --disable-libquadmath --enable-plugin --with-system-zlib --disable-browser-plugin --enable-java-awt=gtk --enable-gtk-cairo --with-java-home=/usr/lib/jvm/java-1.5.0-gcj-5-arm64/jre --enable-java-home --with-jvm-root-dir=/usr/lib/jvm/java-1.5.0-gcj-5-arm64 --with-jvm-jar-dir=/usr/lib/jvm-exports/java-1.5.0-gcj-5-arm64 --with-arch-directory=aarch64 --with-ecj-jar=/usr/share/java/eclipse-ecj.jar --enable-multiarch --enable-fix-cortex-a53-843419 --disable-werror --enable-checking=release --build=aarch64-linux-gnu --host=aarch64-linux-gnu --target=aarch64-linux-gnu
Thread model: posix
gcc version 5.3.1 20160413 (Ubuntu/Linaro 5.3.1-14kord4)

build:
gcc -g -std=c++11 -shared -fPIC -O0 -o libsyscallpreload.so syscall_hook_arm64.cc -ldl -linprocdump -lstdc++

debug:
INTERCEPT_HOOK_CMDLINE_FILTER=$1 ST2_DEBUG_SYSCALL=1 LD_PRELOAD=./libsyscallpreload.so gdb --args $@

ref:
1. LD_TRACE_LOADED_OBJECTS, LD_VERBOSE, LD_DEBUG
2. refer The GNU Assembler: https://www.eecs.umich.edu/courses/eecs373/readings/Assembler.pdf
3. ARMv8-A A64 ISA Overview by Matteo Franchin
4. ARM Architecture Reference Manual ARMv8, for ARMv8-A architecture profile
*/

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <link.h>
#include <errno.h>
#include <time.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include "../aarch64/syscall.h"
#include <sys/mman.h>
#include <sys/auxv.h>
#include <sys/ucontext.h>
#include <linux/futex.h>
#include <linux/filter.h>
#include <linux/unistd.h>
#include <linux/seccomp.h>
#include <linux/prctl.h>

#define HOOK_AFTER_ENUM
#define STUB_SIZE (8*1024*8)

// 调试日志函数
void debug_dump(const char *fmt, ...);

extern "C" long syscall_wrapper(long no, ...);

// 该函数执行的系统调用不被记录，目的是给拦截库发起系统调用时候使用.
extern "C" long syscall_no_intercept(long no, ...);

///////////////////////////////////////////////////////////////////////
// syscall dumper
#include "../cpu.h"
#include "../include/shared_mem_dump.h"

// 描述系统调用的结果缓存的共享内存区
static MemoryDumper *g_dumper = NULL;

///////////////////////////////////////////////////////////////////////
// 系统调用拦截函数,在这里记录系统调用参数和系统调用的结果
extern "C" long syscall_hook(USER_REGS* ctx)
{
    // NOTE: SYS_exit_group never return.
    long ret = 0;

    if (!g_dumper) {
        // NOTE: delay create MemoryDumper to avoid crash in _dl_init of ld.so
        MemoryDumper* dumper = create_memory_dumper();
        if (dumper) {
            g_dumper = dumper;
            debug_dump("create dumper=%p\n", g_dumper);
        }
    }

    // NOTE: there's no SYS_getpid in sunway's; it use sys_call 0x9e
    if (g_dumper && g_dumper->pid == syscall_no_intercept(SYS_getpid)) {
        unsigned char can_dump = is_number_in_set(ctx->x8);

        ret = syscall_no_intercept(ctx->x8,
                ctx->x0, ctx->x1, ctx->x2, ctx->x3, ctx->x4, ctx->x5);
        if (can_dump) {
            record_syscall(g_dumper, (int)ctx->x8, (long*)&ctx->x0, ret, ctx);
        }
        switch (ctx->x8) {
        case SYS_mmap:
            if (ret >= 0) {
                send_cmd(SYS_update_maps, ctx->x2);
            }
            break;
        case SYS_mremap:
        // case SYS_munmap:
            if (ret >= 0) {
                send_cmd(SYS_update_maps, 0);
            }
            break;
        default:
            break;
        }
    }
    else {
        // fork before exec reach here!
        ret = syscall_no_intercept(ctx->x8,
                ctx->x0, ctx->x1, ctx->x2, ctx->x3, ctx->x4, ctx->x5);
        debug_dump("syscall %d: ret=0x%lx, args=%lx,%lx,%lx,%lx,%lx,%lx\n",
            ctx->x8, ret,
            ctx->x0, ctx->x1, ctx->x2, ctx->x3, ctx->x4, ctx->x5);
    }

    return ret;
}

// syscall_no_intercept 函数的汇编实现
__asm(
    ".globl syscall_no_intercept\n\t"
    "syscall_no_intercept:\n\t"
    "mov x8, x0\n\t"     // 从w0寄存器值拷贝系统调用号到w8寄存器
    "mov x0, x1\n\t"     // 从x1寄存器值拷贝第1个参数到x0寄存器
    "mov x1, x2\n\t"     // 从x2寄存器值拷贝第2个参数到x1寄存器
    "mov x2, x3\n\t"     // 从x3寄存器值拷贝第3个参数到x2寄存器
    "mov x3, x4\n\t"     // 从x4寄存器值拷贝第4个参数到x3寄存器
    "mov x4, x5\n\t"     // 从x5寄存器值拷贝第5个参数到x4寄存器
    "mov x5, x6\n\t"     // 从x6寄存器值拷贝第6个参数到x5寄存器
    "svc #0x0\n\t"       // 进入内核, 执行系统调用
    "ret\n\t"            // 返回函数
    "nop \n\t"           // 16字节对齐
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
    "nop\n\t"
);

// syscall_wrapper 函数的汇编实现
// NOTE: 只有 X9-X15 可以安全使用.
__asm(
    ".globl syscall_wrapper\n\t"
    "syscall_wrapper:\n\t"
    "sub sp,  sp, #512\n\t" //构建一个512字节的局部变量区，用来保存 USER_REGS 数据
    // 注意 stp 的地址必须是 16 字节对齐
    "stp x0,  x1,  [sp, #0]\n\t"
    "stp x2,  x3,  [sp, #16]\n\t"
    "stp x4,  x5,  [sp, #32]\n\t"
    "stp x6,  x7,  [sp, #48]\n\t"
    "stp x8,  x9,  [sp, #64]\n\t" //X8 系统调用号 ,X9 指向 syscall_wrapper
    "stp x10, x11, [sp, #80]\n\t" //X10 保存 syscall 指令的 pc 值, X11 保存 syscall 指令所在函数的返回地址
    "stp x12, x13, [sp, #96]\n\t"
    "stp x14, x15, [sp, #112]\n\t"
    "stp x16, x17, [sp, #128]\n\t"//PIC/PLT
    "stp x18, x19, [sp, #144]\n\t"//Platform 寄存器
    "stp x20, x21, [sp, #160]\n\t"
    "stp x22, x23, [sp, #176]\n\t"
    "stp x24, x25, [sp, #192]\n\t"
    "stp x26, x27, [sp, #208]\n\t"
    "stp x28, x29, [sp, #224]\n\t"//FP 寄存器

    "str x30, [sp,#240]\n\t" //RA/LR 寄存器, 该值是 syscall 拦截跳板跳回到 syscall 指令的下一个指令的指令
    "mov x13, sp\n\t"
    "str x13, [sp,#248]\n\t" //保存 sp
    "str x10, [sp,#256]\n\t" //保存 pc, 见 build_trampoline 函数
    //"str pstate,  [sp,#264]\n\t" //TODO: 如何访问 pstate ?

    "ldr x13, [sp, #504]\n\t" // 加载 syscall 指令所在函数的返回地址, 见 build_trampoline 函数
    "str x13, [sp, #88 ]\n\t" // 拷贝到 USER_REGS->x11 里面.
    "mov x0, sp\n\t"          // 加载 USER_REGS 地址到 x0 寄存器
    "mov x28, sp\n\t"        // 拷贝 USER_REGS 地址到 x28 寄存器，用于发生崩溃的时候修复 pc,ra,sp
    "bl syscall_hook \n\t"

    // 以下汇编指令从栈里面恢复保存的所有寄存器, x0寄存器除外.
    // 注意 ldp 的地址必须是 16 字节对齐
    "ldr x1,       [sp,#8]\n\t"
    "ldp x2,  x3,  [sp,#16]\n\t"
    "ldp x4,  x5,  [sp,#32]\n\t"
    "ldp x6,  x7,  [sp,#48]\n\t"
    "ldp x8,  x9,  [sp,#64]\n\t"
    "ldp x10, x11, [sp,#80]\n\t"
    "ldp x12, x13, [sp,#96]\n\t"
    "ldp x14, x15, [sp,#112]\n\t"
    "ldp x16, x17, [sp,#128]\n\t"
    "ldp x18, x19, [sp,#144]\n\t"
    "ldp x20, x21, [sp,#160]\n\t"
    "ldp x22, x23, [sp,#176]\n\t"
    "ldp x24, x25, [sp,#192]\n\t"
    "ldp x26, x27, [sp,#208]\n\t"
    "ldp x28, x29, [sp,#224]\n\t"
    "ldr x30,      [sp,#240]\n\t"
    // "ldr pc,   [sp,#256]\n\t"

    "add sp,  sp, #512\n\t"   // 删除512字节的局部变量区
    "ret \n\t"                // 返回到 syscall 拦截跳板
    "nop \n\t"                // 16字节对齐
);

// @return: number of dwords
// 动态构造一段汇编指令把参数 target_addr 指向的64位地址值保存到 x10 寄存器;
// 返回动态构造的汇编指令的总字节数
static int store_address_in_x10(uintptr_t target_addr, uint32_t* walk)
{
    uint32_t* head = walk;
    uint32_t low = target_addr;
    uint32_t high = target_addr >> 32;

    /*
    mov    w9, #0x1234
    movk   w9, #0xffff, lsl #16
    mov    w10, #0x5678
    movk   w10, #0xabcd, lsl #16
    orr    x10, x9, x10, lsl #32
    */
    *walk++ = 0x52800009 | ((low & 0xffff) << 5);
    *walk++ = 0x72a00009 | ((low>>16) << 5);

    *walk++ = 0x5280000a | ((high & 0xffff) << 5);
    *walk++ = 0x72a0000a | ((high>>16) << 5);

    *walk++ = 0xaa0a812a;

    return (walk - head);
}

// @return: number of dwords
// 动态构造一段汇编指令把参数 target_addr 指向的64位地址值保存到 x9 寄存器;
// 返回动态构造的汇编指令的总字节数
// 这里的机器码有两种方式获得:
// 1, 手写汇编然后用gcc编译后再反汇编得到.
// 2, 看cpu的指令手册，按照指令格式手动组织.
static int store_address_in_x9(uintptr_t target_addr, uint32_t* walk)
{
    uint32_t* head = walk;
    uint32_t low = target_addr;
    uint32_t high = target_addr >> 32;

    /*
    mov    w11, #0x1234
    movk   w11, #0xffff, lsl #16
    mov    w9, #0x5678
    movk   w9, #0xabcd, lsl #16
    orr    x9, x11, x9, lsl #32
    */

    *walk++ = 0x5280000b | ((low & 0xffff) << 5);
    *walk++ = 0x72a0000b | ((low>>16) << 5);

    *walk++ = 0x52800009 | ((high & 0xffff) << 5);
    *walk++ = 0x72a00009 | ((high>>16) << 5);

    *walk++ = 0xaa098169;

    return (walk - head);
}

//@return: number of bytes
//构造syscall指令拦截跳板,返回该跳板的总的字节数
static int build_trampoline(uintptr_t stub, uintptr_t ret_addr)
{
    uint32_t* walk = (uint32_t*)stub;
    int pagesize = sysconf(_SC_PAGESIZE);
    char* base = (char*)(stub & ~(pagesize -1));

    // 备份 $ra 寄存器里面保存的 syscall 指令所在的函数返回地址到栈上 $sp - 8 处;
    // 因为 $ra 继承器会被 call 指令修改.
    // str x30, [sp, #-8]
    *walk++ = 0xf81f83fe;

    // store address of current PC in x10 used in dump_syscall function
    // for backtrace call stack
    walk += store_address_in_x10(ret_addr - 4, walk);

    walk += store_address_in_x9((uintptr_t)syscall_wrapper, walk);

    // 调用 syscall_wrapper 函数，准备 USER_REGS 以调用C实现的 syscall_hook 函数
    // 来记录系统调用. call 指令值修改 ra 寄存器,不修改栈.
    // blr x9
    *walk++ = 0xd63f0120;
    //NOP
    *walk++ = 0xd503201f;

    // 从栈 $sp - 8 处恢复 $ra 寄存器值
    // ldr x30, [sp, #-8]
    *walk++ = 0xf85f83fe;

    // 返回到 syscall 指令的下一个指令.
    // b ret_addr
    *walk = ((ret_addr - (uintptr_t)walk) >> 2) & 0x3ffffff | 0x14000000;
    //NOP
    walk[1] = 0xd503201f;

    return (uint8_t*)walk + 8 - (uint8_t*)stub;
}

// 拦截 syscall 指令成功的个数
static int hook_ok = 0;
// 拦截 syscall 指令失败的个数
static int hook_failed = 0;
// syscall 指令拦截跳板区域首地址
static unsigned char* trampoline_addr = 0;
// syscall 指令拦截跳板区域尾地址
static unsigned char* trampoline_addr_end = 0;
unsigned char* dump_maps(void);
unsigned char* find_gap_page_near_pc(uintptr_t pc, bool search_after_libc);

inline bool is_in_br_range(uintptr_t pc, uintptr_t target)
{
    const uintptr_t max_disp = 128*1024*1024;
    if (pc < target) {
        return target - pc < max_disp;
    }
    else {
        return pc - target < max_disp;
    }
}

// 拦截 pc 位置的 syscall 指令
static int hack_syscall(uintptr_t pc)
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

    int len = 0;
    int pagesize = sysconf(_SC_PAGESIZE);
    char* base = (char*)(pc & ~(pagesize -1));
    uintptr_t trampoline = (uintptr_t)trampoline_addr;
    int err = syscall_no_intercept(SYS_mprotect, base,
            pagesize*2, PROT_WRITE|PROT_EXEC|PROT_READ);
    if (!err) {
        // 替换 syscall 指令为一个 b xxx 指令, 跳到syscall拦截跳板
        // b offset ;jmp range is +128MB/-128MB.
        *(uint32_t*)pc = (((uintptr_t)trampoline - pc) >> 2) & 0x3ffffff | 0x14000000;
    }
    else {
        debug_dump("failed to mprotect for %lx:%lx\n", pc, trampoline);
        return -2;
    }
    syscall_no_intercept(SYS_mprotect, base, pagesize*2, PROT_EXEC|PROT_READ);

    len = build_trampoline(trampoline, pc + 4);

    return len;
}

// enum all syscall instruction
static void *vdso_addr = nullptr;
static bool debug_dumps_on = false;
static bool patch_all_objs = false;

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
                PROT_READ | PROT_EXEC | PROT_WRITE,
                MAP_PRIVATE | MAP_ANON, -1, (off_t)0);
    if (expect_addr != nullptr) {
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
#ifdef HOOK_AFTER_ENUM
    char lib_path[256];
#endif
    unsigned char *base_addr;
    unsigned long text_offset;
    unsigned char *text_start;
    unsigned char *text_end;
    Elf64_Half text_section_index;
};

#ifdef HOOK_AFTER_ENUM
static int g_current = 0;
static intercept_desc g_libs[4];
#endif

/*
 * find_text,搜索指定模块的代码段首地址
 *
 * See: man elf
 */
static void find_text(struct intercept_desc* desc, int fd)
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

// 判断当前 syscall 指令是否可以安全拦截,　目前不能安全拦截的是 clone 和 vfork系统调用
static bool is_can_safe_hook(unsigned char* code)
{
    // skip hook SYS_clone, SYS_vfork, SYS_rt_sigreturn
    // and some syscall in libpthread.so load syscall number from memory like:
    /*
      0000000000006020 <setxid_signal_thread>:
          6020:       aa0003e3        mov     x3, x0
          6024:       b9410820        ldr     w0, [x1,#264]
          6028:       36300140        tbz     w0, #6, 6050 <setxid_signal_thread+0x30>
          602c:       d53bd040        mrs     x0, tpidr_el0
          6030:       d2800422        mov     x2, #0x21                       // #33
          6034:       d11bc000        sub     x0, x0, #0x6f0
          6038:       d2801068        mov     x8, #0x83                       // #131
          603c:       b980d021        ldrsw   x1, [x1,#208]
          6040:       b980d400        ldrsw   x0, [x0,#212]
          6044:       d4000001        svc     #0x0
          ...
          c6b58:      d2801b88        mov     x8, #0xdc                       // #220
          c6b5c:      d4000001        svc     #0x0

          d2802088        mov     x8, #0x104                                  // #260
    */

    unsigned int num = 0;
    unsigned int* walk = (unsigned int*)code;
    // the maximum syscall parameter is 6
    for (int i = 0; (i<6) && (0==num); ++i) {
        // parse mov x8, #imm
        // bit 0-4: x8
        // bit 5-20: imm16
        --walk;
        if ((*walk & 0xffe0001f) != 0xd2800008) continue;
        num = (*walk >> 5) & 0xffff;
    }

    if (0 == num || SYS_clone == num || SYS_tgkill == num) {
        return false;
    }

    return true;
}

/*
 * patch_text
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
static void patch_text(struct intercept_desc *desc)
{
    unsigned char *code = desc->text_start;

    while (code <= desc->text_end) {
        // 逐个指令进行反汇编，直到发现一个 syscall 指令
        if (*(uint32_t*)code == 0xd4000001 /*svc #0x0*/) {
            if (is_can_safe_hook(code)) {
                int len = hack_syscall((uintptr_t)code);
                if (len > 0) {
                    debug_dump("hack:%p(%ld) to %p,%d\n",
                            code, code - desc->text_start, trampoline_addr, len);
                    ++hook_ok;
                    trampoline_addr += len;
                    if (trampoline_addr + len > trampoline_addr_end) {
                        // current block is full, need allocate new block
                        trampoline_addr = nullptr;
                        trampoline_addr_end = nullptr;
                    }
                }
                else {
                    ++hook_failed;
                    debug_dump("failed to hack:%p(%ld) -> %d, %d\n",
                            code, code - desc->text_start, len, errno);
                    break; //disable hook this so
                }
            }
            else {
                debug_dump("skip hack:%p to %p\n", code, trampoline_addr);
            }
        }

        code += 4;
    }
    debug_dump("hook finished (ok=%d, failed=%d)\n", hook_ok, hook_failed);
}

// 对参数 path 指定的模块里面的所有 syscall 指令进行拦截.
void patch_syscalls(struct intercept_desc* desc, const char* path)
{
    debug_dump("patch_syscalls in %s "
        "at base_addr 0x%016" PRIxPTR "\n",
        path,
        (uintptr_t)desc->base_addr);

    int fd = syscall_no_intercept(SYS_openat, AT_FDCWD, path, O_RDONLY);

    find_text(desc, fd);

    debug_dump(
        "%s .text mapped at 0x%016" PRIxPTR " - 0x%016" PRIxPTR " \n",
        path,
        (uintptr_t)desc->text_start,
        (uintptr_t)desc->text_end);

    syscall_no_intercept(SYS_close, fd);

    patch_text(desc);
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

            if (0 == strcmp(strtab + item->st_name, "__kernel_gettimeofday")) {
                memcpy(&g_gettimeofday, &target, sizeof(uintptr_t));
            } else if (0 == strcmp(strtab + item->st_name, "__kernel_clock_gettime")) {
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
// 从当前PC处附近正负4兆字节处搜索一段空闲的内存块,用作syscall拦截跳板区域
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
    unsigned char* found = nullptr;
    int eof = 0;
    int size = 256;
    int remain = 0;
    int fd = syscall_no_intercept(SYS_openat, AT_FDCWD, "/proc/self/maps", O_RDONLY);
    if (fd <= 0) {
        debug_dump("failed find gap:%lx,%lx -> %d\n", gap_start, gap_end, errno);
        return nullptr;
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
    unsigned char* found = nullptr;
    int eof = 0;
    int size = 256;
    int remain = 0;
    int fd = syscall_no_intercept(SYS_openat, AT_FDCWD, "/proc/self/maps", O_RDONLY);
    if (fd <= 0) {
        debug_dump("failed find gap:%lx,%lx -> %d\n", gap_start, gap_end, errno);
        return nullptr;
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

    if (str_match(name, len, libc)) {
        debug_dump(" - libc found\n");
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

    struct intercept_desc patches;
    patches.base_addr = (uint8_t*)info->dlpi_addr;

#ifdef HOOK_AFTER_ENUM
    strcpy(patches.lib_path, path);
    memcpy(&g_libs[g_current], &patches, sizeof(patches));
    ++g_current;
#else
    patch_syscalls(&patches, path);
#endif

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

static const char* cmdline = nullptr;
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
    cmdline = argv[0];

    if (!syscall_hook_in_process_allowed()) {
        debug_dump("syscall_hook_in_process_allowed=false\n");
        return;
    }

    debug_dumps_on = getenv("ST2_DEBUG_SYSCALL") != NULL;
    g_hook_vdso = getenv("ST2_HOOK_VDSO") != NULL;

    vdso_addr = (void *)(uintptr_t)getauxval(AT_SYSINFO_EHDR);

    dl_iterate_phdr(analyze_object, NULL);

    // NOTE: there's no SYS_getpid in sunway's; it use sys_call 0x9e
#ifdef HOOK_AFTER_ENUM
    struct timespec begin;
    clock_gettime(CLOCK_REALTIME, &begin);
    debug_dump("hook start for pid=%d, time=(%ld,%ld)\n",
            getpid(), begin.tv_sec, begin.tv_nsec);
    for (int i = 0; i<g_current; ++i) {
        patch_syscalls(g_libs + i, g_libs[i].lib_path);
    }
#endif

    //NOTE: read trampoline to verify cache flush!
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
    __asm__ __volatile__("mov %0, x30" : "=r" (pc));

    if (g_dumper && g_dumper->pid == syscall_no_intercept(SYS_getpid)) {
        ctx->x11 = ctx->ra;
        ctx->x10 = pc;
        record_syscall(g_dumper, (int)ctx->x8, (long*)&ctx->x0, result, ctx);
    }
}

// 注意 stp 的地址必须是 16 字节对齐
#define syscall_arg2(no, arg1, arg2)  do {               \
    __asm(                                               \
    "sub sp,  sp, #512\n\t"\
    "stp x0,  x1,  [sp, #0]\n\t"\
    "stp x2,  x3,  [sp, #16]\n\t"\
    "stp x4,  x5,  [sp, #32]\n\t"\
    "stp x6,  x7,  [sp, #48]\n\t"\
    "stp x8,  x9,  [sp, #64]\n\t"\
    "stp x14, x15, [sp, #112]\n\t"\
    "stp x16, x17, [sp, #128]\n\t"\
    "stp x18, x19, [sp, #144]\n\t"\
    "stp x20, x21, [sp, #160]\n\t"\
    "stp x22, x23, [sp, #176]\n\t"\
    "stp x24, x25, [sp, #192]\n\t"\
    "stp x26, x27, [sp, #208]\n\t"\
    "stp x28, x29, [sp, #224]\n\t"\
    "str x30, [sp,#240]\n\t" \
    "mov x13, sp\n\t" \
    "str x13, [sp,#248]\n\t" \
     );                                                  \
    __asm__ __volatile__("mov %0, sp" : "=r" (ctx));   \
    ctx->x8 = no;                                        \
    assert((long)ctx == ctx->sp);                        \
}while(0)

#pragma GCC visibility push(default)
int clock_gettime(clockid_t clk_id, struct timespec* tp)
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
        __asm__ __volatile__ ("add sp, sp, #512\n\t");
    }
    return ret;
}

int gettimeofday(struct timeval* tv, struct timezone* tz)
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
        __asm__ __volatile__ ("add sp, sp, #512\n\t");
    }
    return ret;
}
#pragma GCC visibility pop
