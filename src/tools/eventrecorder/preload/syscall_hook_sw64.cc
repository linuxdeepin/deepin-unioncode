/*
cpu: CDSW421

kernel: Linux deepin-PC 4.4.15-aere-deepin #108 SMP Wed Dec 26 14:50:33 CST 2018 sw_64 GNU/Linux

compiler:
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/libexec/gcc/sw_64sw6-sunway-linux-gnu/5.3.0/lto-wrapper
Target: sw_64sw6-sunway-linux-gnu
Configured with: ../gcc-5.3.0/configure --prefix=/usr --with-gmp=/usr/lib_for_gcc --with-mpfr=/usr/lib_for_gcc --with-mpc=/usr/lib_for_gcc --enable-shared --enable-languages=c,c++,fortran,go,java,lto --build=sw_64sw6-sunway-linux-gnu --host=sw_64sw6-sunway-linux-gnu --target=sw_64sw6-sunway-linux-gnu --disable-multilib --enable-c99 --enable-long-long --enable-threads=posix --enable-__cxa_atexit --disable-bootstrap --disable-nls --enable-multiarch --enable-browser-plugin --enable-gnu-indirect-function --enable-linker-build-id --enable-linker-no-relax
Thread model: posix
Version: SWREACH GCC5.3.0-54 (20181101) by xwb on trunk

build:
gcc -g -std=c++11 -shared -fPIC -O0 -o libsyscallpreload.so syscall_hook_sw64.cc -ldl -linprocdump

debug:
INTERCEPT_HOOK_CMDLINE_FILTER=$1 ST2_DEBUG_SYSCALL=1 LD_PRELOAD=./libsyscallpreload.so gdb --args $@

ref:
LD_TRACE_LOADED_OBJECTS, LD_VERBOSE, LD_DEBUG
// refer The GNU Assembler:
// https://www.eecs.umich.edu/courses/eecs373/readings/Assembler.pdf
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
#include <syscall.h>
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
    struct wrapper_ret {
        long v0;
        long a3;
        long a4; // store euid for geteuid
    }ret;

    ret.a3 = 0;

    // NOTE: SYS_exit_group never return.

    if (!g_dumper) {
        // NOTE: delay create MemoryDumper to avoid crash in _dl_init of ld.so
        MemoryDumper* dumper = create_memory_dumper();
        if (dumper) {
            g_dumper = dumper;
            debug_dump("create dumper=%p\n", g_dumper);
        }
    }

    // NOTE: there's no SYS_getpid in sunway's; it use sys_call 0x9e
    if (g_dumper && g_dumper->pid == syscall_no_intercept(SYS_getxpid)) {
        unsigned char can_dump = is_number_in_set(ctx->v0);

        ret.v0 = syscall_no_intercept(ctx->v0,
                ctx->a0, ctx->a1, ctx->a2, ctx->a3, ctx->a4, ctx->a5);
        // asm("stl $19,64($15)"); //"stl a3, ret.a3";
        // 从a3寄存器读取系统调用状态,如果是1表示该系统调用失败
        __asm__ __volatile__("mov $19, %0" : "=r" (ret.a3));
        // 从a4寄存器读取euid,目前发现geteuid会用到a4寄存器
        __asm__ __volatile__("mov $20, %0" : "=r" (ret.a4));
        ctx->a3 = ret.a3;
        ctx->a4 = ret.a4;

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
        ret.v0 = syscall_no_intercept(ctx->v0,
                ctx->a0, ctx->a1, ctx->a2, ctx->a3, ctx->a4, ctx->a5);
        // asm("stl $19,64($15)"); //"stl a3, ret.a3";
        // 从a3寄存器读取系统调用状态,如果是1表示该系统调用失败
        __asm__ __volatile__("mov $19, %0" : "=r" (ret.a3));
        // 从a4寄存器读取euid,目前发现geteuid会用到a4寄存器
        __asm__ __volatile__("mov $20, %0" : "=r" (ret.a4));
        debug_dump("syscall %d: ret=0x%lx/%ld, args=%lx,%lx,%lx,%lx,%lx,%lx\n",
            ctx->v0, ret.v0, ret.a3,
            ctx->a0, ctx->a1, ctx->a2, ctx->a3, ctx->a4, ctx->a5);
        ctx->a3 = ret.a3;
        ctx->a4 = ret.a4;
    }

    return ret.v0;
}

#if defined(__sw_64)
// syscall_no_intercept 函数的汇编实现
__asm(
    ".globl syscall_no_intercept\n\t"
    "syscall_no_intercept:\n\t"
    "bis $31, $16, $0\n\t"      // 从a0寄存器值拷贝系统调用号到v0寄存器
    "bis $31, $17, $16\n\t"     // 从a1寄存器值拷贝第1个参数到a0寄存器
    "bis $31, $18, $17\n\t"     // 从a2寄存器值拷贝第2个参数到a1寄存器
    "bis $31, $19, $18\n\t"     // 从a3寄存器值拷贝第3个参数到a2寄存器
    "bis $31, $20, $19\n\t"     // 从a4寄存器值拷贝第4个参数到a3寄存器
    "bis $31, $21, $20\n\t"     // 从a5寄存器值拷贝第5个参数到a4寄存器
    "ldl $21, 0($30)\n\t"       // 从堆栈读取第6个参数到a5寄存器
    "sys_call 0x83\n\t"         // 进入内核, 执行系统调用
    "ret $31, ($26), 0x1\n\t"   // 返回函数
);

// syscall_wrapper 函数的汇编实现
__asm(
    ".globl syscall_wrapper\n\t"
    "syscall_wrapper:\n\t"
    "ldi $30, -512($30)\n\t" //构建一个512字节的局部变量区，用来保存 USER_REGS 数据
    "stl $0,   0($30)\n\t"   //保存 v0 寄存器
    "stl $1,   8($30)\n\t"   //保存 t0 寄存器
    "stl $2,   16($30)\n\t"  //保存 t1 寄存器
    "stl $3,   24($30)\n\t"  //保存 t2 寄存器
    "stl $4,   32($30)\n\t"  //保存 t3 寄存器, t3寄存器用来保存syscall指令的pc值, 见 build_trampoline 函数
    "stl $5,   40($30)\n\t"  //保存 t4 寄存器
    "stl $6,   48($30)\n\t"  //保存 t5 寄存器
    "stl $7,   56($30)\n\t"  //保存 t6 寄存器
    "stl $8,   64($30)\n\t"  //保存 t7 寄存器
    "stl $9,   72($30)\n\t"  //保存 s0 寄存器
    "stl $10,  80($30)\n\t"  //保存 s1 寄存器
    "stl $11,  88($30)\n\t"  //保存 s2 寄存器
    "stl $12,  96($30)\n\t"  //保存 s3 寄存器
    "stl $13,  104($30)\n\t" //保存 s4 寄存器
    "stl $14,  112($30)\n\t" //保存 s5 寄存器
    "stl $15,  120($30)\n\t" //保存s6(fp)寄存器
    "stl $19,  128($30)\n\t" //保存 a3 寄存器
    "stl $20,  136($30)\n\t" //保存 a4 寄存器
    "stl $21,  144($30)\n\t" //保存 a5 寄存器
    "stl $22,  152($30)\n\t" //保存 t8 寄存器
    "stl $23,  160($30)\n\t" //保存 t9 寄存器
    "stl $24,  168($30)\n\t" //保存 t10 寄存器
    "stl $25,  176($30)\n\t" //保存 t11 寄存器
    "stl $26,  184($30)\n\t" //保存 ra 寄存器, 该值是 syscall 拦截跳板跳回到 syscall 指令的下一个指令的指令
    "stl $27,  192($30)\n\t" //保存 t12 寄存器, 该值是 syscall指令所在函数的首地址
    "stl $28,  200($30)\n\t" //保存 at 寄存器
    "stl $30,  208($30)\n\t" //保存 sp 寄存器
    "stl $27,  216($30)\n\t" //保存 ps(pv)寄存器值，等同t12值
    "stl $23,  224($30)\n\t" //保存 t9 寄存器
    "stl $29,  232($30)\n\t" //保存 gp 寄存器
    "stl $16,  240($30)\n\t" //保存 a0 寄存器
    "stl $17,  248($30)\n\t" //保存 a1 寄存器
    "stl $18,  256($30)\n\t" //保存 a2 寄存器

    "ldl $3, 504($30)\n\t"   // 加载 syscall 指令所在函数的返回地址, 见 build_trampoline 函数
    "stl $3, 24($30)\n\t"    // 拷贝到 USER_REGS->t2 里面.
    "bis $31, $30, $16\n\t"  // 加载 USER_REGS 地址到 a0 寄存器
    "bis $31, $30, $14\n\t"  // 拷贝 USER_REGS 地址到 s5 寄存器，用于发生崩溃的时候修复 pc,ra,sp

    // 下面的7个汇编指令是把 syscall_hook 函数地址保存到 t12 寄存器
    // 因为地址值是64位的,一次只能处理16位的立即数,所以要经过4次load指令,
    // 一次逻辑左移和一次或操作.
    "ldi $27,4660($31)\n\t"
    "maskll $27, 2, $27\n\t" // 总是把符号扩展位所在字节清零
    "ldih $27,22136($27)\n\t"
    "ldi $0,4660($31)\n\t"
    "ldih $0,22136($0)\n\t"
    "sll $0,0x20,$0\n\t"
    "bis $0,$27,$27\n\t"

    "call $26, ($27)\n\t"   // 调用 syscall_hook 函数

    // 以下汇编指令从栈里面恢复保存的所有寄存器, v0寄存器除外.
    "ldl $1,   8($30)\n\t"   //t0
    "ldl $2,   16($30)\n\t"
    "ldl $3,   24($30)\n\t"
    "ldl $4,   32($30)\n\t"
    "ldl $5,   40($30)\n\t"
    "ldl $6,   48($30)\n\t"
    "ldl $7,   56($30)\n\t"
    "ldl $8,   64($30)\n\t"
    "ldl $9,   72($30)\n\t"  //s0
    "ldl $10,  80($30)\n\t"
    "ldl $11,  88($30)\n\t"
    "ldl $12,  96($30)\n\t"
    "ldl $13,  104($30)\n\t"
    "ldl $14,  112($30)\n\t"
    "ldl $15,  120($30)\n\t" //s6/fp
    "ldl $19,  128($30)\n\t" //恢复 a3,此时的 a3 保存了系统调用的成功失败状态.
    "ldl $20,  136($30)\n\t"
    "ldl $21,  144($30)\n\t"
    "ldl $22,  152($30)\n\t" //t8
    "ldl $23,  160($30)\n\t"
    "ldl $24,  168($30)\n\t"
    "ldl $25,  176($30)\n\t"
    "ldl $26,  184($30)\n\t" //ra
    "ldl $27,  192($30)\n\t" //t12
    "ldl $28,  200($30)\n\t" //at
    "ldl $30,  208($30)\n\t" //sp;
    "ldl $27,  216($30)\n\t" //t12 as ps/pv;
    "ldl $23,  224($30)\n\t" //t9
    "ldl $29,  232($30)\n\t" //gp;
    "ldl $16,  240($30)\n\t" //a0
    "ldl $17,  248($30)\n\t"
    "ldl $18,  256($30)\n\t"

    "ldi $30,  512($30)\n\t" // 删除512字节的局部变量区
    "ret $31, ($26), 0x1\n\t" // 返回到 syscall 拦截跳板
);
#else
#error need define new arch implement
#endif

// @return: number of dwords
// 动态构造一段汇编指令把参数 target_addr 指向的64位地址值保存到 t3 寄存器;
// 返回动态构造的汇编指令的总字节数
static int store_address_in_t3(uintptr_t target_addr, uint32_t* walk)
{
    uint32_t* head = walk;
    uint32_t low = target_addr;
    uint32_t high = target_addr >> 32;

    /*
    34 12 3f f8    ldi    $r1,4660($r31)
    78 56 21 fc    ldih    $r1,22136($r1)
    34 12 5f f8    ldi    $r2,4660($r31)
    78 56 42 fc    ldih    $r2,22136($r2)
    02 09 44 48    sll    $r2,0x20,$r2
    44 07 41 40    bis    $r2,$r1,$r4
    */
    *walk++ = 0xf83f0000 | (low & 0xffff);
    if (low & 0x00008000) {
        // t0 will sign extend with 1, should clear them
        // maskll  $r1,0x2,$r1
        *walk++ = 0x48204c61;
    }
    *walk++ = 0xfc210000 | (low >> 16);
    *walk++ = 0xf85f0000 | (high & 0xffff);
    *walk++ = 0xfc420000 | (high >> 16);
    *walk++ = 0x48440902;
    *walk++ = 0x40410744;

    return (walk - head);
}

// @return: number of dwords
// 动态构造一段汇编指令把参数 target_addr 指向的64位地址值保存到 t0 寄存器;
// 返回动态构造的汇编指令的总字节数
// 这里的机器码有两种方式获得:
// 1, 手写汇编然后用gcc编译后再反汇编得到.
// 2, 看cpu的指令手册，按照指令格式手动组织.
static int store_address_in_t0(uintptr_t target_addr, uint32_t* walk)
{
    uint32_t* head = walk;
    uint32_t low = target_addr;
    uint32_t high = target_addr >> 32;

    /*
    34 12 3f f8    ldi    $r1,4660($r31)
    78 56 21 fc    ldih    $r1,22136($r1)
    34 12 5f f8    ldi    $r2,4660($r31)
    78 56 42 fc    ldih    $r2,22136($r2)
    02 09 44 48    sll    $r2,0x20,$r2
    41 07 41 40    bis    $r2,$r1,$r1
    */

    *walk++ = 0xf83f0000 | (low & 0xffff);
    if (low & 0x00008000) {
        // t0 will sign extend with 1, should clear them
        // maskll  $r1,0x2,$r1
        *walk++ = 0x48204c61;
    }
    *walk++ = 0xfc210000 | (low >> 16);
    *walk++ = 0xf85f0000 | (high & 0xffff);
    *walk++ = 0xfc420000 | (high >> 16);
    *walk++ = 0x48440902;
    *walk++ = 0x40410741;

    return (walk - head);
}

// @return: number of dwords
// 动态构造一段汇编指令把参数 target_addr 指向的64位地址值保存到 t12 寄存器;
// 返回动态构造的汇编指令的总字节数
static int store_address_in_t12(uintptr_t target_addr, uint32_t* walk)
{
    uint32_t* head = walk;
    uint32_t low = target_addr;
    uint32_t high = target_addr >> 32;

    /*
    34 12 7f fb    ldi    $r27,4660($r31)
    78 56 7b ff    ldih    $r27,22136($r27)
    34 12 3f f8    ldi    $r1,4660($r31)
    78 56 21 fc    ldih    $r1,22136($r1)
    01 09 24 48    sll    $r1,0x20,$r1
    5b 07 3b 40    bis    $r1,$r27,$r27
    */

    *walk++ = 0xfb7f0000 | (low & 0xffff);
    ++walk; // skip  maskll  $r27,0x2,$r27
    *walk++ = 0xff7b0000 | (low >> 16);
    *walk++ = 0xf83f0000 | (high & 0xffff);
    *walk++ = 0xfc210000 | (high >> 16);
    *walk++ = 0x48240901;
    *walk++ = 0x403b075b;

    return (walk - head);
}

// 修复 syscall_wrapper 函数里面动态计算 syscall_hook 地址的一段汇编指令
// 原因是拦截库的加载地址每次启动的时候都是变化的.
static void fix_syscall_wrapper(uintptr_t target_addr)
{
    uintptr_t head = (uintptr_t)syscall_wrapper;
    int pagesize = sysconf(_SC_PAGESIZE);
    char* base = (char*)(head & ~(pagesize -1));
    int err = syscall_no_intercept(SYS_mprotect, base,
            pagesize*2, PROT_WRITE|PROT_EXEC|PROT_READ);
    if (!err) {
        uint32_t* walk = (uint32_t*)head;
        for (int i = 0; ; ++i) {
            // 搜索动态计算 syscall_hook 地址的位置:
            // ldi $27,4660($31)
            if (walk[i] == 0xfb7f1234) {
                walk += i;
                break;
            }
            assert(i < 4096);
        }

        // 更新 syscall_hook 地址计算汇编代码.
        store_address_in_t12(target_addr, walk);
    }

    syscall_no_intercept(SYS_mprotect, base, pagesize*2, PROT_EXEC|PROT_READ);
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
    // f8 ff 5e af    stl    ra,-8(sp)
    *walk++ = 0xaf5efff8;

    // store address of current PC in t3 used in dump_syscall function
    // for backtrace call stack
    walk += store_address_in_t3(ret_addr - 4, walk);

    walk += store_address_in_t0((uintptr_t)syscall_wrapper, walk);

    // TODO: how to update gp before call syscall_wrapper?
    // like: ldgp $gp, 0($27)
    // NOTE: here we can't overwrite t12/pv
    // 调用 syscall_wrapper 函数，准备 USER_REGS 以调用C实现的 syscall_hook 函数
    // 来记录系统调用. call 指令值修改 ra 寄存器,不修改栈.
    // call ra, (t0)
    *walk++ = 0x07410000;

    // 从栈 $sp - 8 处恢复 $ra 寄存器值
    // f8 ff 5e 8f  ldl ra,-8(sp)
    *walk++ = 0x8f5efff8;

    // 返回到 syscall 指令的下一个指令.
    // br $31, ret_addr;jmp range is 8MB.
    *walk = ((ret_addr - (uintptr_t)walk - 4 ) >> 2) & 0x01fffff | 0x13e00000;

    return (uint8_t*)walk + 4 - (uint8_t*)stub;
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
    const uintptr_t max_disp = 4*1024*1024;
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
        // 替换 syscall 指令为一个br指令, 跳到syscall拦截跳板
        // br $31, trampoline ;jmp range is 8MB.
        *(uint32_t*)pc = (((uintptr_t)trampoline - pc - 4 ) >> 2) & 0x01fffff | 0x13e00000;
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

static bool g_hook_vdso = false;

// 判断当前 syscall 指令是否可以安全拦截,　目前不能安全拦截的是 clone 和 vfork系统调用
static bool is_can_safe_hook(unsigned char* code)
{
    // skip hook SYS_clone, SYS_vfork, SYS_rt_sigreturn
    // 11982c:   38 01 1f f8     ldi    $r0,312 //clone
    // d6b5c:    42 00 1f f8     ldi    $r0,66  //vfork
    /* clock_gettime:
    12c5d4:   a4 01 3f f9     ldi $r9,420
    12c5d8:   00 00 5e af     stl ra,0(sp)
    12c5dc:   40 07 e9 43     mov $r9,$r0
    12c5e0:   83 00 00 02     sys_call    0x83
    */
    /* gettimeofday:
    c0950:   67 01 1f f8     ldi $r0,359
    c0954:   83 00 00 02     sys_call    0x83
     * */
    if (!g_hook_vdso) {
        if (*(unsigned int*)(code - 8) == 0xf93f01a4 ||
            *(unsigned int*)code == 0xf81f0167) {
            // hook clock_gettime only if enabled vdso switch
            return false;
        }
    }
    return *(unsigned int*)code != 0xf81f0138 &&
           *(unsigned int*)code != 0xf81f0042;
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
        if (*(uint32_t*)code == 0x02000083 /*sys_call 0x83*/) {
            if (is_can_safe_hook(code - 4)) {
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
                    debug_dump("failed to hack:%p(%ld) -> %d\n",
                            code, code - desc->text_start, len);
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

    int fd = syscall_no_intercept(SYS_open, path, O_RDONLY);

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
    int fd = syscall_no_intercept(SYS_open, "/proc/self/maps", O_RDONLY);
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
    int fd = syscall_no_intercept(SYS_open, "/proc/self/maps", O_RDONLY);
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

    fix_syscall_wrapper((uintptr_t)syscall_hook);

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
