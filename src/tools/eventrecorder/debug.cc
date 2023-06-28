// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <assert.h>
#include <fnmatch.h>
#include <limits.h>
#include <linux/net.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/user.h>

#if !defined(__sw_64) && !defined(__aarch64__)
#include <capstone/capstone.h>
#endif

#if defined(__i386__) || defined(__x86_64__)
#include <libunwind-ptrace.h>
#endif

#include <string>
#include <memory>

#include "cpu.h"
#include "debug.h"
#include "elf_helper.h"

using namespace std;

int write_mem(int pid, uintptr_t addr, const void* buf, int buf_size)
{
    int nwritten = 0;
    // ptrace operates on the word size of the host, so we really do want
    // to use sizes of host types here.
    uintptr_t word_size = sizeof(long);

    errno = 0;

    // Only write aligned words. This ensures we can always write the last
    // byte before an unmapped region.
    while (nwritten < buf_size) {
        uintptr_t start = addr + nwritten;
        uintptr_t start_word = start & ~(word_size - 1);
        uintptr_t end_word = start_word + word_size;
        uintptr_t length = min(end_word - start, uintptr_t(buf_size - nwritten));

        long v;
        if (length < word_size) {
            v = ptrace(PTRACE_PEEKDATA, pid, start_word, nullptr);
            if (errno) {
                break;
            }
        }

        memcpy(reinterpret_cast<uint8_t*>(&v) + (start - start_word),
             static_cast<const uint8_t*>(buf) + nwritten, length);
        ptrace(PTRACE_POKEDATA, pid, start_word, reinterpret_cast<void*>(v));
        nwritten += length;
    }

    return nwritten;
}

int read_mem(int tid, uintptr_t addr, void* buf, int buf_size)
{
    int nread = 0;
    // ptrace operates on the word size of the host, so we really do want
    // to use sizes of host types here.
    uintptr_t word_size = sizeof(long);
    // Only read aligned words. This ensures we can always read the last
    // byte before an unmapped region.
    uintptr_t start_word = addr & ~(word_size - 1);
    uintptr_t end_word = (addr + buf_size) & ~(word_size - 1);

    errno = 0;
    if (start_word < addr) {
        // the first unaligned part
        long v = ptrace(PTRACE_PEEKDATA, tid, start_word, nullptr);
        long offset = (addr - start_word);
        start_word += word_size;
        nread = start_word - addr;
        if (nread > buf_size) nread = buf_size;
        memcpy(static_cast<uint8_t*>(buf),
               reinterpret_cast<uint8_t*>(&v) + offset, nread);
    }

    while (start_word < end_word) {
        long v = ptrace(PTRACE_PEEKDATA, tid, start_word, nullptr);
        if (errno) {
            break;
        }
        start_word += word_size;
        memcpy(static_cast<uint8_t*>(buf) + nread,
               reinterpret_cast<uint8_t*>(&v), word_size);
        nread += word_size;
    }

    if (nread < buf_size) {
        // the last unaligned part
        long v = ptrace(PTRACE_PEEKDATA, tid, start_word, nullptr);
        memcpy(static_cast<uint8_t*>(buf) + nread,
               reinterpret_cast<uint8_t*>(&v), buf_size - nread);
        nread = buf_size;
    }

    return nread;
}

int gdb_disass(int pid, void* base, int len)
{
#if defined(__sw_64) || defined(__aarch64__)
    return -1;
#else
    csh handle;
    cs_insn* insn = NULL;
    uintptr_t address = (uintptr_t)base;
    string codestr('0', len);
    uint8_t* code = (uint8_t*)codestr.data();
    size_t code_size = len;

    int got = read_mem(pid, address, code, code_size);
    if (got < (int)code_size) {
        return -1;
    }

#if defined(__x86_64__)
    if (cs_open(CS_ARCH_X86, cs_mode(CS_MODE_64), &handle) != CS_ERR_OK) {
#elif defined(__mips64)
    if (cs_open(CS_ARCH_MIPS, cs_mode(CS_MODE_MIPS64|CS_MODE_LITTLE_ENDIAN),
                &handle) != CS_ERR_OK) {
#else
    if (1) {
#endif
        return -2;
    }

    // allocate memory cache for 1 instruction, to be used by cs_disasm_iter later.
    insn = cs_malloc(handle);

    /*Some mips struction on loonson not support by libcapstone
      use this statement maybe better decode*/
    cs_option(handle, CS_OPT_SKIPDATA, CS_OPT_ON);

    // disassemble one instruction a time & store the result into @insn variable above
    while(cs_disasm_iter(handle, (const uint8_t**)&code, &code_size, &address, insn))
    {
        // analyze disassembled instruction in @insn variable ...
        // NOTE: @code, @code_size & @address variables are all updated
        // to point to the next instruction after each iteration.

        printf("0x%" PRIx64 ":\t%s\t%s\n",
                insn->address, insn->mnemonic, insn->op_str);

        // print implicit registers used by this instruction
        cs_detail* detail = insn->detail;
        if (!detail) continue;

        if (detail->regs_read_count > 0) {
            printf("\tImplicit registers read: ");
            for (int n = 0; n < detail->regs_read_count; n++) {
                printf("%s ", cs_reg_name(handle, detail->regs_read[n]));
            }
            printf("\n");
        }

        // print implicit registers modified by this instruction
        if (detail->regs_write_count > 0) {
            printf("\tImplicit registers modified: ");
            for (int n = 0; n < detail->regs_write_count; n++) {
                printf("%s ", cs_reg_name(handle, detail->regs_write[n]));
            }
            printf("\n");
        }

        // print the groups this instruction belong to
        if (detail->groups_count > 0) {
            printf("\tThis instruction belongs to groups: ");
            for (int n = 0; n < detail->groups_count; n++) {
                printf("%s ", cs_group_name(handle, detail->groups[n]));
            }
            printf("\n");
        }
    }

    // release the cache memory when done
    cs_free(insn, 1);
    cs_close(&handle);
    return 0;
#endif
}

int gdb_pause(pid_t pid)
{
    int ret = ptrace(PTRACE_INTERRUPT, pid, nullptr, nullptr);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

// used by gdb console to debug dead-lock in recording
int gdb_cont(pid_t pid)
{
    int ret = ptrace(PTRACE_CONT, pid, nullptr, nullptr);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int gdb_step(int pid)
{
    int ret = ptrace(PTRACE_SINGLESTEP, pid, nullptr, nullptr);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int gdb_bt(int tid)
{
#if defined(__i386__) || defined(__x86_64__)
#define MAX_STACK_FRAME 64

    unw_addr_space_t unwind_as;
    struct UPT_info* ui = nullptr;
    int n = 0, ret;
    unw_cursor_t cursor;
    unw_proc_info_t pi;
    unw_word_t ip, sp, start_ip = 0, off;
    char buf[512];

    unwind_as = unw_create_addr_space (&_UPT_accessors, 0);
    if (!unwind_as) {
        printf("libunwind.unw_create_addr_space() failed\n");
        return -1;
    }

    ui = reinterpret_cast<struct UPT_info*>(_UPT_create(tid));
    ret = unw_init_remote (&cursor, unwind_as, ui);
    if (ret < 0) {
        printf("unw_init_remote() failed: pause first then try again, ret=%d\n", ret);
        _UPT_destroy (ui);
        return -2;
    }

    do {
        if ((ret = unw_get_reg (&cursor, UNW_REG_IP, &ip)) < 0 ||
            (ret = unw_get_reg (&cursor, UNW_REG_SP, &sp)) < 0) {
            printf("unw_get_reg failed: ret=%d\n", ret);
            break;
        }

        if (n == 0) start_ip = ip;

        buf[0] = '\0';
        unw_get_proc_name (&cursor, buf, sizeof (buf), &off);

        if ((ret = unw_get_proc_info (&cursor, &pi)) < 0) {
            printf("unw_get_proc_info(%lx)failed: ret=%d\n", ip, ret);
            break;
        }
        printf("\t[%d \t]ip=%-16lx\tsp=%-16lx\t%s+%lx\n", n, ip, sp, buf, off);

        ret = unw_step (&cursor);
        if (ret < 0) {
            unw_get_reg (&cursor, UNW_REG_IP, &ip);
            printf("FAILURE: unw_step() returned %d, for ip=%lx\n", ret, ip);
            break;
        }

        if (++n > MAX_STACK_FRAME) {
            /* guard against bad unwind info in old libraries... */
            printf("too deeply nested---assuming bogus unwind (start ip=%lx)\n", start_ip);
            break;
        }
    }
    while (ret > 0);

    _UPT_destroy (ui);
    if (unwind_as) {
        unw_destroy_addr_space (unwind_as);
        unwind_as = nullptr;
    }
#elif defined(__mips__) || defined(__sw_64) || defined(__aarch64__)
    USER_REGS regs;
    printf("\ndump register:\n");
    gdb_reg(tid, &regs);
    printf("\ndump memory of pc:\n");
    gdb_mem(tid, (void*)regs.pc, 256, 1);
    printf("\ndump memory of sp:\n");
    gdb_mem(tid, (void*)regs.sp, 256, 1);
#else
    printf("not implement in your arch\n");
#endif

    return 0;
}

static unsigned int copy_print_char (char* out, const unsigned char* buf, unsigned int count)
{
    unsigned int i = 0;
    while (i<count)
    {
        if (buf[i] < 0x20 || buf[i] > 0x7f)
        {
            out[i] = '.';
        }
        else
        {
            out[i] = buf[i];
        }
        ++i;
    }
    out[i++] = '\n';

    return i;
}

static void print_bytes(const void* buffer, int total)
{
    char* str = NULL;
    int i = 0;
    int j = 0;
    unsigned char* buf = (unsigned char*)buffer;
    int count = total;
    unique_ptr<char[]> outbuf = make_unique<char[]>(count*4 + (count+15)/16*2 + 48);

    str = outbuf.get();

    while ( i<count )
    {
        j += sprintf(str+j, "%02x ", buf[i] );

        ++i;
        if ( !( i&15 ) )
        {
            str[j++] = ' ';
            j += copy_print_char (str+j, buf+i-16, 16);
        }
    }

    if ( count&15 )
    {
        i = count&15; //left
        memset (str+j, 0x20, (16-i)*3 + 1 );
        j += (16-i)*3 + 1;
        j += copy_print_char (str+j, buf+count-i, i);
    }
    str[j] = 0;

    printf("%s", str);
}

static char hex2char(unsigned char hex)
{
    if (hex<0xa) return hex+'0';
    else return hex-0xa+'a';
}

static void hex2str2(unsigned char* hex, int size, char* str)
{
    int i = 0;
    char* walk = str;
    for (;i<size;)
    {
        walk[0] = hex2char(hex[2*i+1]>>4);
        walk[1] = hex2char(hex[2*i+1]&0xf);
        walk[2] = hex2char(hex[2*i]>>4);
        walk[3] = hex2char(hex[2*i]&0xf);
        walk[4] = 0x20;
        ++i;
        walk += 5;
        if (!(i & 7)) {
            *walk++ = '\n';
        }
    }

    *walk = 0;
}

static void hex2str4(unsigned char* hex, int size, char* str)
{
    int i = 0;
    char* walk = str;
    for (;i<size;)
    {
        walk[0] = hex2char(hex[4*i+3]>>4);
        walk[1] = hex2char(hex[4*i+3]&0xf);
        walk[2] = hex2char(hex[4*i+2]>>4);
        walk[3] = hex2char(hex[4*i+2]&0xf);
        walk[4] = hex2char(hex[4*i+1]>>4);
        walk[5] = hex2char(hex[4*i+1]&0xf);
        walk[6] = hex2char(hex[4*i]>>4);
        walk[7] = hex2char(hex[4*i]&0xf);
        walk[8] = 0x20;
        ++i;
        walk += 9;
        if (!(i&3)) {
            *walk++ = '\n';
        }
    }

    *walk = 0;
}

static void hex2str8(unsigned char* hex, int size, char* str)
{
    int i = 0;
    char* walk = str;
    for (;i<size;)
    {
        walk[0 ] = hex2char(hex[8*i+7]>>4);
        walk[1 ] = hex2char(hex[8*i+7]&0xf);
        walk[2 ] = hex2char(hex[8*i+6]>>4);
        walk[3 ] = hex2char(hex[8*i+6]&0xf);
        walk[4 ] = hex2char(hex[8*i+5]>>4);
        walk[5 ] = hex2char(hex[8*i+5]&0xf);
        walk[6 ] = hex2char(hex[8*i+4]>>4);
        walk[7 ] = hex2char(hex[8*i+4]&0xf);
        walk[8 ] = hex2char(hex[8*i+3]>>4);
        walk[9 ] = hex2char(hex[8*i+3]&0xf);
        walk[10] = hex2char(hex[8*i+2]>>4);
        walk[11] = hex2char(hex[8*i+2]&0xf);
        walk[12] = hex2char(hex[8*i+1]>>4);
        walk[13] = hex2char(hex[8*i+1]&0xf);
        walk[14] = hex2char(hex[8*i]>>4);
        walk[15] = hex2char(hex[8*i]&0xf);
        walk[16] = 0x20;
        ++i;
        walk += 17;
        if (!(i&1)) {
            *walk++ = '\n';
        }
    }

    *walk = 0;
}

static void print_short(const void* buffer, int total)
{
    int count = total/sizeof(short);
    unique_ptr<char[]> outbuf = make_unique<char[]>(count*5 + count/8 + 32);
    hex2str2((unsigned char*)buffer, count, outbuf.get());
    printf("%s", outbuf.get());
}

static void print_word(const void* buffer, int total)
{
    int count = total/sizeof(int);
    unique_ptr<char[]> outbuf = make_unique<char[]>(count*9 + count/4 + 32);
    hex2str4((unsigned char*)buffer, count, outbuf.get());
    printf("%s", outbuf.get());
}

static void print_dword(const void* buffer, int total)
{
    int count = total/sizeof(long);
    unique_ptr<char[]> outbuf = make_unique<char[]>(count*17 + count/2 + 32);
    hex2str8((unsigned char*)buffer, count, outbuf.get());
    printf("%s", outbuf.get());
}

int gdb_mem(int pid, void* base, int len, int group)
{
    uintptr_t address = (uintptr_t)base;
    unique_ptr<char[]> buffer = make_unique<char[]>(len);
    void* code = buffer.get();
    int got = read_mem(pid, address, code, len);
    if (got < 1) {
        printf("failed to read memory from:%lx,%d\n", address, len);
        return -1;
    }

    len = got;
    switch(group) {
    case 1:
        print_bytes(code, len);
        break;
    case 2:
        print_short(code, len);
        break;
    case 4:
        print_word(code, len);
        break;
    case 8:
        print_dword(code, len);
        break;
    default:
        break;
    }

    return len;
}

int gdb_reg(int tid, USER_REGS* out)
{
    USER_REGS regs;

#if defined(__aarch64__)
	static struct iovec io = {
		.iov_base = &regs,
		.iov_len = sizeof(regs)
	};
    if (ptrace(PTRACE_GETREGSET, tid, NT_PRSTATUS, &io) < 0) {
#else
    if (ptrace(PTRACE_GETREGS, tid, nullptr, &regs) < 0) {
#endif
        return -errno;
    }

#if defined(__i386__)
#error unspport get_cpu_context
#elif defined(__x86_64__)
    printf("\trip=%-16llx\n", regs.rip);
    printf("\trbp=%-16llx\n", regs.rbp);
    printf("\trsp=%-16llx\n", regs.rsp);

    printf("\trax=%-16llx\n", regs.rax);
    printf("\trbx=%-16llx\n", regs.rbx);

    printf("\trdi=%-16llx\n", regs.rdi);
    printf("\trsi=%-16llx\n", regs.rsi);
    printf("\trdx=%-16llx\n", regs.rdx);
    printf("\trcx=%-16llx\n", regs.rcx);
    printf("\tr8=%-16llx\n", regs.r8);
    printf("\tr9=%-16llx\n", regs.r9);

    printf("\tr10=%-16llx\n", regs.r10);
    printf("\tr11=%-16llx\n", regs.r11);
    printf("\tr12=%-16llx\n", regs.r12);
    printf("\tr13=%-16llx\n", regs.r13);
    printf("\tr14=%-16llx\n", regs.r14);
    printf("\tr15=%-16llx\n", regs.r15);
#elif defined(__mips__) || defined(__mips64)
    printf("\tat=%-16lx\n", regs.at);
    printf("\tv0=%-16lx\n", regs.v0);
    printf("\tv1=%-16lx\n", regs.v1);
    printf("\ta0=%-16lx\n", regs.a0);
    printf("\ta1=%-16lx\n", regs.a1);
    printf("\ta2=%-16lx\n", regs.a2);
    printf("\ta3=%-16lx\n", regs.a3);

#if _MIPS_SIM != _ABIO32
    printf("\ta4=%-16lx\n", regs.a4);
    printf("\ta5=%-16lx\n", regs.a5);
    printf("\ta6=%-16lx\n", regs.a6);
    printf("\ta7=%-16lx\n", regs.a7);
    printf("\tt0=%-16lx\n", regs.t0);
    printf("\tt1=%-16lx\n", regs.t1);
    printf("\tt2=%-16lx\n", regs.t2);
    printf("\tt3=%-16lx\n", regs.t3);
#else /* if _MIPS_SIM == _ABIO32 */
    printf("\tt0=%-16lx\n", regs.t0);
    printf("\tt1=%-16lx\n", regs.t1);
    printf("\tt2=%-16lx\n", regs.t2);
    printf("\tt3=%-16lx\n", regs.t3);
    printf("\tt4=%-16lx\n", regs.t4);
    printf("\tt5=%-16lx\n", regs.t5);
    printf("\tt6=%-16lx\n", regs.t6);
    printf("\tt7=%-16lx\n", regs.t7);
#endif /* _MIPS_SIM == _ABIO32 */
    printf("\ts0=%-16lx\n", regs.s0);
    printf("\ts1=%-16lx\n", regs.s1);
    printf("\ts2=%-16lx\n", regs.s2);
    printf("\ts3=%-16lx\n", regs.s3);
    printf("\ts4=%-16lx\n", regs.s4);
    printf("\ts5=%-16lx\n", regs.s5);
    printf("\ts6=%-16lx\n", regs.s6);
    printf("\ts7=%-16lx\n", regs.s7);
    printf("\tt8=%-16lx\n", regs.t8);
    printf("\tt9=%-16lx\n", regs.t9);
    printf("\tk0=%-16lx\n", regs.k0);
    printf("\tk1=%-16lx\n", regs.k1);
    printf("\tgp=%-16lx\n", regs.gp);
    printf("\tsp=%-16lx\n", regs.sp);
    printf("\tfp=%-16lx\n", regs.fp);
    printf("\tra=%-16lx\n", regs.ra);

    /* Saved special registers. */
    printf("\tlo=%-16lx\n", regs.lo);
    printf("\thi=%-16lx\n", regs.hi);
    printf("\tpc=%-16lx\n", regs.pc);
    printf("\tbadvaddr=%-16lx\n", regs.cp0_badvaddr);
    printf("\tstatus=%-16lx\n", regs.cp0_status);
    printf("\tcause=%-16lx\n", regs.cp0_cause);
#elif defined(__sw_64)
    printf("\tgp=%-16lx\n", regs.gp);
    printf("\tsp=%-16lx\n", regs.sp);
    printf("\tfp=%-16lx\n", regs.s6);
    printf("\tra=%-16lx\n", regs.ra);
    printf("\tpc=%-16lx\n", regs.pc);
    printf("\tt12=%-16lx\n", regs.t12);

    printf("\tv0=%-16lx\n", regs.v0);
    printf("\ta0=%-16lx\n", regs.a0);
    printf("\ta1=%-16lx\n", regs.a1);
    printf("\ta2=%-16lx\n", regs.a2);
    printf("\ta3=%-16lx\n", regs.a3);
    printf("\ta4=%-16lx\n", regs.a4);
    printf("\ta5=%-16lx\n", regs.a5);
#elif defined(__aarch64__)
    printf("\tsp=%-16lx\n", regs.sp);
    printf("\tpc=%-16lx\n", regs.pc);

    printf("\tx0=%-16lx\n", regs.x0);
    printf("\tx1=%-16lx\n", regs.x1);
    printf("\tx2=%-16lx\n", regs.x2);
    printf("\tx3=%-16lx\n", regs.x3);
    printf("\tx4=%-16lx\n", regs.x4);
    printf("\tx5=%-16lx\n", regs.x5);
#else
#error unspport get_cpu_context
#endif

    if (out) {
        memcpy(out, &regs, sizeof(regs));
    }

    return 0;
}

#if defined(__i386__) || defined(__x86_64__)
#define BREAKPOINT_INSTRUCTION 0xcc
#define BREAKPOINT_SIZE 1
#elif defined(__mips__)
#define BREAKPOINT_INSTRUCTION 0x0000000d
#define BREAKPOINT_SIZE 4
#elif defined(__sw_64)
#define BREAKPOINT_INSTRUCTION 0x00000080
#define BREAKPOINT_SIZE 4
#elif defined(__aarch64__)
#define BREAKPOINT_INSTRUCTION 0xd4200000
#define BREAKPOINT_SIZE 4
#else
#error need implement in new arch
#endif

int gdb_break(int tid, uintptr_t addr, uint32_t* value)
{
    uint32_t ret;
    uint32_t cc = BREAKPOINT_INSTRUCTION;
    if (read_mem(tid, addr, &ret, BREAKPOINT_SIZE) != BREAKPOINT_SIZE) {
        return -1;
    }
    if (write_mem(tid, addr, &cc, BREAKPOINT_SIZE) != BREAKPOINT_SIZE) {
        return -2;
    }

    if (value) *value = ret;

    return 0;
}

int gdb_delete(int tid, uintptr_t addr, uint32_t old_value)
{
    return write_mem(tid, addr, &old_value, BREAKPOINT_SIZE);
}

// gdb_print(pid, base, "/usr/lib/x86_64-linux-gnu/ld-2.24.so", addr, "struct link_map");
int gdb_print(int pid, uintptr_t module_base, const char* filename,
        uintptr_t var_address, const char* type_name)
{
    (void)pid;
    (void)module_base;
    (void)filename;
    (void)var_address;
    (void)type_name;
    SymbolFile helper(module_base, filename, true);
    /*
     http://www.dwarfstd.org/doc/Debugging%20using%20DWARF-2012.pdf

    .debug_abbrev Abbreviations used in the .debug_info section
    .debug_aranges A mapping between memory address and compilation
    .debug_frame Call Frame Information
    .debug_info The core DWARF data containing DWARF Information Entries (DIEs)
    .debug_line Line Number Program
    .debug_loc Location descriptions
    .debug_macinfo Macro descriptions
    .debug_pubnames A lookup table for global objects and functions
    .debug_pubtypes A lookup table for global types
    .debug_ranges Address ranges referenced by DIEs
    .debug_str String table used by .debug_info
    .debug_types Type descriptions

    for (const auto& cu : helper.m_dwarf.compilation_units()) {
        for (const auto& die : cu.root()) {
            if (die.has(dwarf::DW_AT::name) && at_name(die) == type_name) {
            }
        }
    }
    */

    return 0;
}
