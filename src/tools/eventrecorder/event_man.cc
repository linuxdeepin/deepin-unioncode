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

#include <assert.h>
#include <elf.h>
#include <link.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <algorithm>
#include <type_traits>

#include "event_man.h"
#include "easylogging++.h"
#include "trace_reader.h"
#include "trace_writer.h"
#include "md_types.h"
#include "config.h"

using namespace std;

INITIALIZE_EASYLOGGINGPP

#ifdef __mips__
#define PAGE_SIZE_SUB_ONE (4*1024-1)
#elif defined(__sw_64)
#define PAGE_SIZE_SUB_ONE (4*1024-1)
#else
#define PAGE_SIZE_SUB_ONE (4095)
#endif

#define Ehdr   ElfW(Ehdr)
#define Phdr   ElfW(Phdr)
#define Shdr   ElfW(Shdr)
#define Nhdr   ElfW(Nhdr)

static void rolloutHandler(const char* filename, std::size_t size)
{
    static unsigned int log_idx = 0;
    (void)size;

#ifdef _DEBUG
    // SHOULD NOT LOG ANYTHING HERE BECAUSE LOG FILE IS CLOSED!
    std::cout << "************** Rolling out [" << filename
       << "] because it reached [" << size << " bytes]" << std::endl;
#endif

    // BACK IT UP
    std::stringstream ss;
    ss << "mv " << filename << " " << filename << "-backup." << ++log_idx;
    (void)system(ss.str().c_str());
}


static void init_log(DumpConfig& cfg, const char* filename)
{
    el::Configurations defaultConf;

    defaultConf.setToDefault();

    if (cfg.log_debug) {
        defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "true");
    }
    else {
        defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    }

    /*To hide %user,%host,%func,%file,%line*/
    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format, "%level %msg");
    defaultConf.set(el::Level::Warning, el::ConfigurationType::Format, "%level %msg");
    defaultConf.set(el::Level::Error, el::ConfigurationType::Format, "%level %msg");
    defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%level %msg");

    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput,
            cfg.log_to_stdout?"true":"false");
    defaultConf.setGlobally(el::ConfigurationType::ToFile,
            cfg.log_to_file?"true":"false");
    if (cfg.log_to_file) {
        string logfilename = cfg.dump_dir + filename;
        defaultConf.setGlobally(el::ConfigurationType::Filename,
                logfilename.data());
    }

    if (cfg.log_flush_threshold >= 0) {
        auto && oss = std::ostringstream();
        oss << cfg.log_flush_threshold;

        auto log_flush_threshold = oss.str();
        defaultConf.setGlobally(el::ConfigurationType::LogFlushThreshold,
                log_flush_threshold.data());
    }

    if (cfg.log_file_max_size > 0) {
        auto && oss = std::ostringstream();
        oss << cfg.log_file_max_size;

        auto log_file_max_size = oss.str();

        defaultConf.setGlobally(el::ConfigurationType::MaxLogFileSize,
                log_file_max_size.data());

        el::Helpers::installPreRollOutCallback(rolloutHandler);
        //el::Helpers::uninstallPreRollOutCallback();
    }
    el::Loggers::reconfigureLogger("default", defaultConf);
    el::Loggers::setDefaultConfigurations(defaultConf, true);
}

class Timeline{
public:
    Timeline(void) { current = 0;};
    ~Timeline(void) {};
    int open_trace(const char* maps_file, const char* context_file);
    int generate_coredump(int index, const char* core_file, bool verbose);

    uint32_t current;
    char mode[5];
    vector<EventEntry> event_table;
    vector<MapsEntry> maps_table;
    TraceReader ctx_reader;
    TraceReader maps_reader;
};

///////////////////////////////////////////////////////////////////////
// coredump functions

class Options {
public:
    Options() : verbose(true), out_fd(0), total(0) {
    }
    ~Options(){
        if (out_fd != STDOUT_FILENO) {
            close(out_fd);
        }
        LOG(INFO) << "coredump file size " << total;
    }

    bool verbose;
    int out_fd;
    int total;
};

// Write all of the given buffer, handling short writes and EINTR. Return true
// ff successful.
static bool write_file(Options& option, const void* idata, size_t length)
{
    const uint8_t* data = (const uint8_t*) idata;

    size_t done = 0;
    while (done < length) {
        ssize_t r = 0;
        do {
            r = write(option.out_fd, data + done, length - done);
        } while (r == -1 && errno == EINTR);

        if (r < 1) {
            LOG(ERROR) << "failed to write coredump file, current=" << option.total;
            return false;
        }
        done += r;
    }

    option.total += length;
    return true;
}

/* Dynamically determines the byte sex of the system. Returns non-zero
 * for big-endian machines.
 */
static inline int sex() {
  int probe = 1;
  return !*(char *)&probe;
}

typedef struct elf_timeval {    /* Time value with microsecond resolution    */
  long tv_sec;                  /* Seconds                                   */
  long tv_usec;                 /* Microseconds                              */
} elf_timeval;

typedef struct _elf_siginfo {   /* Information about signal (unused)         */
  int32_t si_signo;             /* Signal number                             */
  int32_t si_code;              /* Extra code                                */
  int32_t si_errno;             /* Errno                                     */
} _elf_siginfo;


#if defined(__aarch64__)
#elif defined (__mips64)
// see linux kernel source: arch/mips/include/uapi/asm/ptrace.h
typedef struct tag_user_regs_struct {
    gregset_t regs;
    uint64_t lo;
    uint64_t hi;
    uint64_t pc;
    uint64_t bad;
    uint64_t sr;
    uint64_t cause;
    uint64_t unkn[7];
}user_regs_struct;
#elif defined(__sw_64)
// see linux kernel source: arch/sw_64/include/uapi/asm/ptrace.h
typedef struct tag_user_regs_struct {
    unsigned long v0;
    unsigned long t0;
    unsigned long t1;
    unsigned long t2;
    unsigned long t3;
    unsigned long t4;
    unsigned long t5;
    unsigned long t6;
    unsigned long t7;
    unsigned long s0;
    unsigned long s1;
    unsigned long s2;
    unsigned long s3;
    unsigned long s4;
    unsigned long s5;
    unsigned long fp;
    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
    unsigned long a4;
    unsigned long a5;
    unsigned long t8;
    unsigned long t9;
    unsigned long t10;
    unsigned long t11;
    unsigned long ra;
    unsigned long t12;
    unsigned long at;
    unsigned long gp;
    unsigned long sp;
    unsigned long pc;
    unsigned long unkn;
}user_regs_struct;
#else
#endif

typedef struct prstatus {       /* Information about thread; includes CPU reg*/
  _elf_siginfo   pr_info;       /* Info associated with signal               */
  uint16_t       pr_cursig;     /* Current signal                            */
  unsigned long  pr_sigpend;    /* Set of pending signals                    */
  unsigned long  pr_sighold;    /* Set of held signals                       */
  pid_t          pr_pid;        /* Process ID                                */
  pid_t          pr_ppid;       /* Parent's process ID                       */
  pid_t          pr_pgrp;       /* Group ID                                  */
  pid_t          pr_sid;        /* Session ID                                */
  elf_timeval    pr_utime;      /* User time                                 */
  elf_timeval    pr_stime;      /* System time                               */
  elf_timeval    pr_cutime;     /* Cumulative user time                      */
  elf_timeval    pr_cstime;     /* Cumulative system time                    */
  user_regs_struct pr_reg;      /* CPU registers                             */
  uint32_t       pr_fpvalid;    /* True if math co-processor being used      */
} prstatus;

typedef struct prpsinfo {       /* Information about process                 */
  unsigned char  pr_state;      /* Numeric process state                     */
  char           pr_sname;      /* Char for pr_state                         */
  unsigned char  pr_zomb;       /* Zombie                                    */
  signed char    pr_nice;       /* Nice val                                  */
  unsigned long  pr_flag;       /* Flags                                     */
#if defined(__x86_64__) || defined(__mips__) || defined(__aarch64__)
  uint32_t       pr_uid;        /* User ID                                   */
  uint32_t       pr_gid;        /* Group ID                                  */
#else
  uint16_t       pr_uid;        /* User ID                                   */
  uint16_t       pr_gid;        /* Group ID                                  */
#endif
  pid_t          pr_pid;        /* Process ID                                */
  pid_t          pr_ppid;       /* Parent's process ID                       */
  pid_t          pr_pgrp;       /* Group ID                                  */
  pid_t          pr_sid;        /* Session ID                                */
  char           pr_fname[16];  /* Filename of executable                    */
  char           pr_psargs[80]; /* Initial part of arg list                  */
} prpsinfo;

// We parse the minidump file and keep the parsed information in this structure
struct DumpedProcess {
  DumpedProcess() : crashing_tid(-1) {
    memset(&prps, 0, sizeof(prps));
    prps.pr_sname = 'R';
    memset(&debug, 0, sizeof(debug));
  }

  struct Mapping {
    Mapping() : permissions(0xFFFFFFFF),
        start_address(0),
        end_address(0),
        offset(0) {
    }

    uint32_t permissions;
    uintptr_t start_address, end_address, offset;
    // The name we write out to the core.
    string filename;
    string data;
  };
  std::map<uintptr_t, Mapping> mappings;

  pid_t crashing_tid;
  int fatal_signal;

  class Heap{
  public:
    Heap(uintptr_t address, string&& astr):
        addr(address), data(std::move(astr)){
            LOG(DEBUG) << "Heap move constructor:" << HEX(addr);
    }
    Heap(uintptr_t address, string& astr):
        addr(address), data(astr){
            LOG(DEBUG) << "Heap constructor:" << HEX(addr);
    }
    uintptr_t addr;
    string data;
  };
  std::vector<Heap> heaps;

  struct Thread {
    pid_t tid;

    USER_REGS regs;
    USER_FPREGS fpregs;

#if defined(__i386__)
    user_fpxregs_struct fpxregs;
#endif

    uintptr_t stack_addr;
    string stack;
  };
  std::vector<Thread> threads;


  uintptr_t at_phdr;
  string auxv;
  string vdso;
  uintptr_t vdso_addr;

  prpsinfo prps;

  // The GUID/filename from MD_MODULE_LIST_STREAM entries.
  // We gather them for merging later on into the list of maps.
  struct Signature {
    string filename;
    char guid[48];
  };
  std::map<uintptr_t, Signature> signatures;

  string dynamic_data;
  MDRawDebug debug;
  std::vector<MDRawLinkMap> link_map;
};

static uint16_t ParseThreadList(const Options& options,
        DumpedProcess* process, TraceReader* reader)
{
    EventHead head;

    static_assert(sizeof(head.cur_time) +
            sizeof(head.reason) +
            sizeof(head.thread_num) +
            sizeof(head.current_tid) +
            sizeof(head.extra_size) == sizeof(head),
            "struct EventHead aligned error!");
    reader->ReadBlock(&head, sizeof(head));

    if (head.extra_size > 0) {
        // delay load them in get_event_extra_info
        reader->Seek(reader->Current() + head.extra_size);
    }

    if (options.verbose) {
        LOG(INFO) << "EMD_THREAD_LIST_STREAM:\nEvent(" << get_event_name(head.reason)
            << "), threads=" << head.thread_num;
    }

    for (int i = 0; i<head.thread_num; ++i) {
        DumpedProcess::Thread thread;

        uint32_t tid = 0;
        reader->ReadValue<uint32_t>(&tid);
        thread.tid = tid & 0xffff;
        reader->ReadBlock(&thread.regs, sizeof(thread.regs));
        reader->ReadBlock(&thread.fpregs, sizeof(thread.fpregs));

        string tls; reader->ReadBlock(tls); //skip tls data

        reader->ReadValue<uintptr_t>(&thread.stack_addr);
        reader->ReadBlock(thread.stack);

        process->threads.push_back(thread);
        if (options.verbose) {
            LOG(INFO) << "thread " << thread.tid << " stack range"
                << ":" << HEX(thread.stack_addr)
                << "," << HEX(thread.stack_addr + thread.stack.size());
        }
    }

    uint32_t heap_count = 0;
    reader->ReadValue<uint32_t>(&heap_count);
    for (uint32_t i = 0; i<heap_count; ++i) {
        uintptr_t addr = 0;
        reader->ReadValue<uintptr_t>(&addr);
        string data; reader->ReadBlock(data);
        if (!data.empty()) {
            if (options.verbose) {
                LOG(INFO) << "heap segment " << i
                    << ":" << HEX(addr)
                    << "," << HEX(addr + data.size());
            }
            process->heaps.push_back(DumpedProcess::Heap(addr, std::move(data)));
        }
    }

    if (head.reason >= DUMP_REASON_signal && head.reason < DUMP_REASON_dbus) {
        process->crashing_tid = head.current_tid;
        process->fatal_signal = head.reason - DUMP_REASON_signal;
    }
    return head.reason;
}

static bool ParseSystemInfo(TraceReader* reader, bool verbose, char* mode)
{
    string blk; reader->ReadBlock(blk);
    MemoryRange range(blk);

    const MDRawSystemInfo* sysinfo = range.GetData<MDRawSystemInfo>(0);
    if (!sysinfo) {
        LOG(ERROR) << "Failed to access EMD_SYSTEM_INFO_STREAM";
        return false;
    }
#if defined(__i386__)
    if (sysinfo->processor_architecture != MD_CPU_ARCHITECTURE_X86) {
        LOG(ERROR) << "This version of emd only supports x86 (32bit)";
        return false;
    }
#elif defined(__x86_64__)
    if (sysinfo->processor_architecture != MD_CPU_ARCHITECTURE_AMD64) {
        LOG(ERROR) << "This version of emd only supports x86 (64bit).";
        return false;
    }
#elif defined(__arm__)
    if (sysinfo->processor_architecture != MD_CPU_ARCHITECTURE_ARM) {
        LOG(ERROR) << "This version of emd only supports ARM (32bit).";
        return false;
    }
#elif defined(__aarch64__)
    if (sysinfo->processor_architecture != MD_CPU_ARCHITECTURE_ARM64) {
        LOG(ERROR) << "This version of emd only supports ARM (64bit).";
        return false;
    }
#elif defined(__mips__)
# if _MIPS_SIM == _ABIO32
    if (sysinfo->processor_architecture != MD_CPU_ARCHITECTURE_MIPS) {
        LOG(ERROR) << "This version of emd only supports mips o32 (32bit).";
        return false;
    }
# elif _MIPS_SIM == _ABI64
    if (sysinfo->processor_architecture != MD_CPU_ARCHITECTURE_MIPS64) {
        LOG(ERROR) << "This version of emd only supports mips n64 (64bit).";
        return false;
    }
# else
#  error "This mips ABI is currently not supported (n32)"
# endif
#elif defined(__sw_64)
    if (sysinfo->processor_architecture != MD_CPU_ARCHITECTURE_SW64) {
        LOG(ERROR) << "This version of emd only supports sw n64 (64bit).";
        return false;
    }
#else
#error "This code has not been ported to your platform yet"
#endif

    memcpy(mode, sysinfo->mode, 4);
    mode[4] = 0;

    if (verbose) {
        const char* arch = sysinfo->processor_architecture == MD_CPU_ARCHITECTURE_X86
                ? "i386"
                : sysinfo->processor_architecture == MD_CPU_ARCHITECTURE_AMD64
                ? "x86-64"
                : sysinfo->processor_architecture == MD_CPU_ARCHITECTURE_ARM
                ? "ARM"
                : sysinfo->processor_architecture == MD_CPU_ARCHITECTURE_ARM64
                ? "ARM64"
                : sysinfo->processor_architecture == MD_CPU_ARCHITECTURE_MIPS
                ? "MIPS"
                : sysinfo->processor_architecture == MD_CPU_ARCHITECTURE_MIPS64
                ? "MIPS64"
                : sysinfo->processor_architecture == MD_CPU_ARCHITECTURE_SW64
                ? "SW64"
                : "???";
        LOG(INFO) << "EMD_SYSTEM_INFO_STREAM:\nArchitecture: \n" << arch
            << "\nRecord Mode: " << mode
            << "\nKernel:\n" << sysinfo->uname
            << "\nDistribution:\n" << sysinfo->lsb_release;;
    }

    return true;
}

static void ParseMaps(const Options& options, DumpedProcess* process,
     const MemoryRange& range)
{
    if (options.verbose) {
        LOG(INFO) << "EMD_LINUX_MAPS:\n"
            << string((char *)range.data(), range.length())
            << "------------------------------------------------------------";
    }
    for (const uint8_t* ptr = range.data();
         ptr < range.data() + range.length();) {
      const uint8_t* eol = (uint8_t*)memchr(ptr, '\n',
                                         range.data() + range.length() - ptr);
      string line((const char*)ptr,
                  eol ? eol - ptr : range.data() + range.length() - ptr);
      ptr = eol ? eol + 1 : range.data() + range.length();
      unsigned long long start, stop, offset;
      char* permissions = nullptr;
      char* filename = nullptr;
      sscanf(line.c_str(), "%llx-%llx %m[-rwxp] %llx %*[:0-9a-f] %*d %ms",
             &start, &stop, &permissions, &offset, &filename);
      if (filename && *filename == '/') {
        DumpedProcess::Mapping mapping;
        mapping.permissions = 0;
        if (strchr(permissions, 'r')) {
          mapping.permissions |= PF_R;
        }
        if (strchr(permissions, 'w')) {
          mapping.permissions |= PF_W;
        }
        if (strchr(permissions, 'x')) {
          mapping.permissions |= PF_X;
        }
        mapping.start_address = start;
        mapping.end_address = stop;
        mapping.offset = offset;
        if (filename) {
          mapping.filename = filename;
        }
        process->mappings[mapping.start_address] = mapping;
      }
      free(permissions);
      free(filename);
    }
}

static void ParseAuxVector(const Options& options,
        DumpedProcess* process, TraceReader* reader)
{
    string blk; reader->ReadBlock(blk);
    MemoryRange range(blk);

    // Some versions of Chrome erroneously used the MD_LINUX_AUXV stream value
    // when dumping /proc/$x/maps
    if (range.length() > 17) {
        // The AUXV vector contains binary data, whereas the maps always begin
        // with an 8+ digit hex address followed by a hyphen and another 8+ digit
        // address.
        char addresses[18];
        memcpy(addresses, range.data(), 17);
        addresses[17] = '\000';
        if (strspn(addresses, "0123456789abcdef-") == 17) {
            ParseMaps(options, process, range);
            return;
        }
    }

    process->auxv = blk;
    const elf_aux_entry* aux_entry = (const elf_aux_entry*)blk.data();
    for (uint32_t i=0; i < blk.size()/sizeof(elf_aux_entry); ++i) {
        if (aux_entry->a_type == AT_PHDR) {
            process->at_phdr = aux_entry->a_un.a_val;
        }
        ++aux_entry;
    }

    if (options.verbose) {
        LOG(DEBUG) << "EMD_LINUX_AUXV:\nFound auxv, bytes=" <<  blk.size()
            << ", AT_PHDR=" << HEX(process->at_phdr);
    }
}

static void ParseCmdLine(const Options& options,
        DumpedProcess* process, TraceReader* reader)
{
    string blk; reader->ReadBlock(blk);
    MemoryRange range(blk);

    // The command line is supposed to use NUL bytes to separate arguments.
    // As Chrome rewrites its own command line and (incorrectly) substitutes
    // spaces, this is often not the case in our minidump files.
    const char* cmdline = (const char*) range.data();
    unsigned i = 0;
    for (; i < range.length() && cmdline[i] && cmdline[i] != ' '; ++i) { }

    if (options.verbose) {
        LOG(INFO) << "EMD_LINUX_CMD_LINE:\nargv[0]=" << string(cmdline, i);
        for (unsigned j = ++i, argc = 1; j < range.length(); ++j) {
            if (!cmdline[j] || cmdline[j] == ' ') {
                LOG(INFO) << "\nargv["<< argc << "]=" << string(cmdline + i, j - i);
                ++argc;
                i = j + 1;
            }
        }
    }

    const char *binary_name = cmdline;
    for (size_t i = 0; i < range.length(); ++i) {
        if (cmdline[i] == '/') {
          binary_name = cmdline + i + 1;
        } else if (cmdline[i] == 0 || cmdline[i] == ' ') {
          static const size_t fname_len = sizeof(process->prps.pr_fname) - 1;
          static const size_t args_len = sizeof(process->prps.pr_psargs) - 1;
          memset(process->prps.pr_fname, 0, fname_len + 1);
          memset(process->prps.pr_psargs, 0, args_len + 1);
          unsigned len = cmdline + i - binary_name;
          memcpy(process->prps.pr_fname, binary_name,
                   len > fname_len ? fname_len : len);

          len = range.length() > args_len ? args_len : range.length();
          memcpy(process->prps.pr_psargs, cmdline, len);
          for (unsigned j = 0; j < len; ++j) {
            if (process->prps.pr_psargs[j] == 0)
              process->prps.pr_psargs[j] = ' ';
          }
          break;
        }
    }
}

static void ParseDSODebugInfo(const Options& options,
        DumpedProcess* process, TraceReader* reader)
{
    int size = 0;
    MDRawDebug debug;
    reader->ReadValue<int>(&size);
    if (0 == size) {
        return;
    }
    if (sizeof(debug) != size) {
        LOG(WARNING) << "Invalid EMD_LINUX_DSO_DEBUG";
        return;
    }
    reader->ReadBlock(&debug, sizeof(debug));

    if (options.verbose) {
        LOG(INFO) << "EMD_LINUX_DSO_DEBUG:\nVersion: " << debug.version
                   << "\nNumber of DSOs: " << debug.dso_count
                   << "\nBrk handler: " << HEX(debug.brk)
                   << "\nLoader address: " << HEX(debug.ldbase)
#if defined(__mips64)
                   << "\nrld_map: " << HEX(debug.rld_map)
#endif
                   << "\n_DYNAMIC address: " << HEX(debug.dynamic);
    }

    process->debug = debug;

    if (debug.dso_count) {
        unique_ptr<MDRawLinkMap[]> buffer = make_unique<MDRawLinkMap[]>(debug.dso_count);
        MDRawLinkMap* link_map = buffer.get();

        reader->ReadBlock(buffer.get(), debug.dso_count*sizeof(MDRawLinkMap));

        for (unsigned int i = 0; i < debug.dso_count; ++i) {
            // NOTE: MD_LINUX_DSO_DEBUG info is gotten from DT_DEBUG field in the dynamic
            // section.
            if (options.verbose) {
                LOG(DEBUG) << i
                    << ", " << HEX(link_map->addr)
                    << ", " << HEX(link_map->ld)
                    << ", " << link_map->name;
            }
            process->link_map.push_back(*link_map);
            ++link_map;
        }
    }

    string blk; reader->ReadBlock(blk); // load dynamic data
    if (blk.length() > 0) {
        process->dynamic_data.assign(blk.data(), blk.length());
    }
}

static void ParseExceptionStream(DumpedProcess* process, TraceReader* reader)
{
    string blk; reader->ReadBlock(blk);
    MemoryRange range(blk);

    const MDRawExceptionStream* exp = range.GetData<MDRawExceptionStream>(0);
    process->crashing_tid = exp->thread_id;
    process->fatal_signal = (int) exp->exception_record.exception_code;
}

static void ParseModuleList(const Options& options, DumpedProcess* process,
                  TraceReader* reader)
{
    if (options.verbose) {
        LOG(INFO) << "EMD_MODULE_LIST_STREAM:";
    }

    uint32_t num_mappings = 0;
    if (!reader->ReadValue(&num_mappings))
        return;

    unique_ptr<MDRawModule[]> buffer = make_unique<MDRawModule[]>(num_mappings);
    reader->ReadBlock(buffer.get(), num_mappings*sizeof(MDRawModule));

    for (unsigned i = 0; i < num_mappings; ++i) {
        DumpedProcess::Mapping mapping;
        MDRawModule* rawmodule = buffer.get() + i;
        mapping.start_address = rawmodule->base_of_image;
        mapping.end_address = rawmodule->size_of_image + rawmodule->base_of_image;

        if (process->mappings.find(mapping.start_address) ==
            process->mappings.end()) {
            // We prefer data from MD_LINUX_MAPS over MD_MODULE_LIST_STREAM, as
            // the former is a strict superset of the latter.
            process->mappings[mapping.start_address] = mapping;
        }

        DumpedProcess::Signature signature;
        signature.filename = rawmodule->file_path;

        if (options.verbose) {
            for (int j = 0; j<kMDGUIDSize; ++j){
                sprintf(&signature.guid[j*2], "%02x", rawmodule->guid[j]);
            }
            signature.guid[kMDGUIDSize*2] = 0;
            LOG(INFO) << HEX(rawmodule->base_of_image)
                << "-" << HEX(rawmodule->base_of_image + rawmodule->size_of_image)
                << ", " << signature.guid
                << ", " << rawmodule->file_path;
        }

        process->signatures[rawmodule->base_of_image] = signature;
    }
}

#if defined(__sw_64)
void copy_regs(user_regs_struct* dst, const USER_REGS* src)
{
    // NOTE: the layout of dst and src are different!
    dst->v0 = src->v0;
    dst->t0 = src->t0;
    dst->t1 = src->t1;
    dst->t2 = src->t2;
    dst->t3 = src->t3;
    dst->t4 = src->t4;
    dst->t5 = src->t5;
    dst->t6 = src->t6;
    dst->t7 = src->t7;

    dst->s0 = src->s0;
    dst->s1 = src->s1;
    dst->s2 = src->s2;
    dst->s3 = src->s3;
    dst->s4 = src->s4;
    dst->s5 = src->s5;
    dst->fp = src->s6;
    dst->a0 = src->a0;
    dst->a1 = src->a1;
    dst->a2 = src->a2;
    dst->a3 = src->a3;
    dst->a4 = src->a4;
    dst->a5 = src->a5;

    dst->t8  = src->t8;
    dst->t9  = src->t9;
    dst->t10 = src->t10;
    dst->t11 = src->t11;
    dst->ra  = src->ra;
    dst->t12 = src->t12;
    dst->at  = src->r28;

    dst->gp = src->gp;
    dst->sp = src->sp;
    dst->pc = src->pc;
}
#endif

static bool WriteThread(struct Options& options,
        const DumpedProcess::Thread& thread, int fatal_signal)
{
    struct prstatus pr;
    memset(&pr, 0, sizeof(pr));

    pr.pr_info.si_signo = fatal_signal;
    pr.pr_cursig = fatal_signal;
    pr.pr_pid = thread.tid;

#if defined(__sw_64)
    copy_regs(&pr.pr_reg, &thread.regs);
#else
    memcpy(&pr.pr_reg, &thread.regs, sizeof(thread.regs));
#endif

    Nhdr nhdr;
    memset(&nhdr, 0, sizeof(nhdr));
    nhdr.n_namesz = 5;
    nhdr.n_descsz = sizeof(struct prstatus);
    nhdr.n_type = NT_PRSTATUS;
    if (!write_file(options, &nhdr, sizeof(nhdr)) ||
        !write_file(options, "CORE\0\0\0\0", 8) ||
        !write_file(options, &pr, sizeof(struct prstatus))) {
      return false;
    }

    nhdr.n_descsz = sizeof(USER_FPREGS);
    nhdr.n_type = NT_FPREGSET;
    if (!write_file(options, &nhdr, sizeof(nhdr)) ||
        !write_file(options, "CORE\0\0\0\0", 8) ||
        !write_file(options, &thread.fpregs, sizeof(USER_FPREGS))) {
      return false;
    }

#if defined(__i386__)
    nhdr.n_descsz = sizeof(user_fpxregs_struct);
    nhdr.n_type = NT_PRXFPREG;
    if (!write_file(options, &nhdr, sizeof(nhdr)) ||
        !write_file(options, "LINUX\0\0\0", 8) ||
        !write_file(options, &thread.fpxregs, sizeof(user_fpxregs_struct))) {
      return false;
    }
#endif

    return true;
}

// gdb's svr4_exec_displacement will failed if we not exclude AT_PHDR region.
// Then the call stack backtrace is corrupt!
static void ExcludePhdrRegion(DumpedProcess::Mapping& mapping, uintptr_t phdr_addr)
{
    if (mapping.start_address < phdr_addr && phdr_addr < mapping.end_address) {
        LOG(WARNING) << "exclude phdr region in " << HEX(mapping.start_address)
            << "-"  << HEX(mapping.end_address);
        mapping.end_address = phdr_addr;
        mapping.data.resize(mapping.end_address - mapping.start_address);
    }
}

static void AddDataToMapping(DumpedProcess* process,
        const string& data, uintptr_t addr)
{
    for (std::map<uintptr_t, DumpedProcess::Mapping>::iterator
       iter = process->mappings.begin();
       iter != process->mappings.end();
       ++iter) {
        if (addr >= iter->second.start_address &&
            addr < iter->second.end_address) {
            uintptr_t base = (addr & ~PAGE_SIZE_SUB_ONE);
            DumpedProcess::Mapping mapping = iter->second;

            if (base != iter->second.start_address) {
                // If there are memory pages in the mapping prior to where the
                // data starts, truncate the existing mapping so that it ends with
                // the page immediately preceding the data region.
                iter->second.end_address = base;
                if (!mapping.filename.empty()) {
                    // "mapping" is a copy of "iter->second". We are splitting the
                    // existing mapping into two separate ones when we write the data
                    // to the core file. The first one does not have any associated
                    // data in the core file, the second one is backed by data that is
                    // included with the core file.
                    // If this mapping wasn't supposed to be anonymous, then we also
                    // have to update the file offset upon splitting the mapping.
                    mapping.offset += iter->second.end_address -
                      iter->second.start_address;
                }
            }
            // Create a new mapping that contains the data contents. We often
            // limit the amount of data that is actually written to the core
            // file. But it is OK if the mapping itself extends past the end of
            // the data.
            mapping.start_address = base;
            mapping.data.assign(addr & PAGE_SIZE_SUB_ONE, 0).append(data);
            mapping.data.append(-mapping.data.size() & PAGE_SIZE_SUB_ONE, 0);
            ExcludePhdrRegion(mapping, process->at_phdr);
            process->mappings[mapping.start_address] = mapping;
            if (base + mapping.data.size() < mapping.end_address) {
                LOG(WARNING) << "data mapping is not enough:" << HEX(mapping.start_address)
                    << "," << mapping.data.size() << "/" << (mapping.end_address - base);
            }
            return;
        }
    }
    // Didn't find a suitable existing mapping for the data. Create a new one.
    DumpedProcess::Mapping mapping;
    mapping.permissions = PF_R | PF_W;
    mapping.start_address = addr & ~PAGE_SIZE_SUB_ONE;
    mapping.end_address = (addr + data.size() + PAGE_SIZE_SUB_ONE) & ~PAGE_SIZE_SUB_ONE;
    mapping.data.assign(addr & PAGE_SIZE_SUB_ONE, 0).append(data);
    mapping.data.append(-mapping.data.size() & PAGE_SIZE_SUB_ONE, 0);
    ExcludePhdrRegion(mapping, process->at_phdr);
    process->mappings[mapping.start_address] = mapping;
}

static string BaseName(const string& path) {
  char* path_tmp = strdup(path.c_str());
  assert(path_tmp);
  string result(basename(path_tmp));
  free(path_tmp);
  return result;
}

#if 0
static void Add_rodata(DumpedProcess* process, const char* filename, uintptr_t base)
{
    // skip kMappedFileUnsafePrefix and kDeletedSuffix file
    if ( strstr(filename, kMappedFileUnsafePrefix) ||
         strstr(filename, kDeletedSuffix)) {
        return ;
    }

    MemoryMappedFile mapped_file(filename, 0);
    if (!mapped_file.data() ||
        mapped_file.size() < SELFMAG) {
        return;
    }
    if (!IsValidElf(mapped_file.data())) {
        return;
    }

    void* data = nullptr;
    size_t data_size = 0;
    if (!FindElfSection(mapped_file.data(), ".rodata", SHT_PROGBITS,
                      (const void**)&data, &data_size)) {
        return;
    }

    string heap((char*)data, data_size);
    uintptr_t addr = base + int((char*)data - (char*)mapped_file.data());
    AddDataToMapping(process, heap, addr);
    LOG(DEBUG) << ".rodata at " << HEX(addr)
        << ", size=" << data_size <<", for " << filename;
}
#endif

static void AugmentMappings(const Options& options, DumpedProcess* process)
{
    // For each thread, find the memory mapping that matches the thread's stack.
    // Then adjust the mapping to include the stack dump.
    for (unsigned i = 0; i < process->threads.size(); ++i) {
        const DumpedProcess::Thread& thread = process->threads[i];
        AddDataToMapping(process, thread.stack, thread.stack_addr);
    }

    for (auto& heap: process->heaps) {
        AddDataToMapping(process, heap.data, heap.addr);
    }

    // NOTE: vdso is not implemented in SunWay!
    if (process->vdso.size() > 1024) {
        AddDataToMapping(process, process->vdso, process->vdso_addr);
    }

#if 0
    // load .rodata section of each module
    for (auto& m: process->signatures) {
        Add_rodata(process, m.second.filename.data(), m.first);
    }
#endif

    // Create a new link map with information about DSOs. We move this map to
    // the beginning of the address space, as this area should always be
    // available.
    static const uintptr_t start_addr = 4096;
    string data;
    struct r_debug debug;
    debug.r_version = process->debug.version;
    debug.r_brk = (ElfW(Addr))process->debug.brk;
    debug.r_state = r_debug::RT_CONSISTENT;
    debug.r_ldbase = (ElfW(Addr))process->debug.ldbase;
    debug.r_map = process->debug.dso_count > 0 ?
      (struct link_map*)(start_addr + sizeof(debug)) : 0;
    data.append((char*)&debug, sizeof(debug));

    struct link_map* prev = 0;
    for (std::vector<MDRawLinkMap>::iterator iter = process->link_map.begin();
         iter != process->link_map.end();
         ++iter) {
        struct link_map link_map;
        link_map.l_addr = (ElfW(Addr))iter->addr;
        link_map.l_name = (char*)(start_addr + data.size() + sizeof(link_map));
        link_map.l_ld = (ElfW(Dyn)*)iter->ld;
        link_map.l_prev = prev;
        prev = (struct link_map*)(start_addr + data.size());
        string filename = iter->name;

        // Look up signature for this filename. If available, change filename
        // to point to GUID, instead.
        std::map<uintptr_t, DumpedProcess::Signature>::const_iterator sig =
        process->signatures.find((uintptr_t)iter->addr);
        if (sig != process->signatures.end()) {
            // At this point, we have:
            // old_filename: The path as found via SONAME (e.g. /lib/libpthread.so.0).
            // sig_filename: The path on disk (e.g. /lib/libpthread-2.19.so).
            string sig_filename = sig->second.filename;
            string old_filename = filename.empty() ? sig_filename : filename;
            string new_filename;

            // First set up the leading path.  We assume dirname always ends with a
            // trailing slash (as needed), so we won't be appending one manually.
            if (1) {
                string dirname = old_filename;
                size_t slash = dirname.find_last_of('/');
                if (slash != string::npos) {
                    new_filename = dirname.substr(0, slash + 1);
                }
            }

            // Decide whether we use the filename or the SONAME (where the SONAME tends
            // to be a symlink to the actual file).
            new_filename += BaseName(old_filename);

            if (filename != new_filename) {
                if (options.verbose) {
                    LOG(DEBUG) << HEX(link_map.l_addr)
                        << " rewriting mapping " << filename.c_str()
                        << " to " << new_filename.c_str();
                }
                filename = new_filename;
            }
        }

        if (std::distance(iter, process->link_map.end()) == 1) {
            link_map.l_next = 0;
        } else {
            link_map.l_next = (struct link_map*)(start_addr + data.size() +
                                                 sizeof(link_map) +
                                                 ((filename.size() + 8) & ~7));
        }
        data.append((char*)&link_map, sizeof(link_map));
        data.append(filename);
        data.append(8 - (filename.size() & 7), 0);
    }
    AddDataToMapping(process, data, start_addr);

    // Map the page containing the _DYNAMIC array
    if (!process->dynamic_data.empty()) {
        // Make _DYNAMIC DT_DEBUG entry point to our link map
        for (int i = 0;; ++i) {
            ElfW(Dyn) dyn;
            if ((i+1)*sizeof(dyn) > process->dynamic_data.length()) {
                if (options.verbose) {
                    LOG(WARNING) << "DT_DEBUG entry not found";
                }
                return;
            }
            memcpy(&dyn, process->dynamic_data.c_str() + i*sizeof(dyn),
               sizeof(dyn));

#if defined(__mips__)
            if (dyn.d_tag == DT_MIPS_RLD_MAP) {
                // NOTE: fix DT_MIPS_RLD_MAP
                // or call stack back trace will miss call frame!
                string ptr;
                ptr.assign((char*)&start_addr, sizeof(start_addr));
                AddDataToMapping(process, ptr, dyn.d_un.d_ptr);
                break;
            }
            else if (dyn.d_tag == DT_MIPS_RLD_MAP_REL) {
                // NOTE: set value of .rld_map to start_addr,
                // or call stack back trace will miss call frame!
                string ptr;
                ptr.assign((char*)&start_addr, sizeof(start_addr));
                AddDataToMapping(process, ptr, process->debug.rld_map);
                break;
#else
            if (dyn.d_tag == DT_DEBUG) {
                process->dynamic_data.replace(i*sizeof(dyn) +
                                         offsetof(ElfW(Dyn), d_un.d_ptr),
                                         sizeof(start_addr),
                                         (char*)&start_addr, sizeof(start_addr));
                break;
#endif
            } else if (dyn.d_tag == DT_NULL) {
                if (options.verbose) {
                    LOG(WARNING) << "DT_DEBUG entry not found 2";
                }
                return;
            }
        }
        AddDataToMapping(process, process->dynamic_data,
                       (uintptr_t)process->debug.dynamic);
    }
}

int Timeline::generate_coredump(int index, const char* core_file, bool verbose)
{
    struct Options options;
    DumpedProcess process;
    TraceReader* cf = &ctx_reader;
    TraceReader* mf = &maps_reader;

    options.verbose = verbose;

    if (index >= 0 && index < (int)event_table.size()) {
        current = index;
    }
    else {
        return -1;
    }

#if 0
    if (!strcmp(mode, "fast")) {
        if (event_table[current].type >= DUMP_REASON_dbus &&
            event_table[current].type < DUMP_REASON_ptrace) {
            LOG(INFO) << "No stack data for x11 and dbus with fast mode!";
            return -1;
        }
    }
#endif

    if (core_file == nullptr || !strcmp(core_file, "-")) {
        options.out_fd = STDOUT_FILENO;
    } else {
        options.out_fd = open(core_file, O_WRONLY|O_CREAT|O_TRUNC, 0664);
        if (options.out_fd == -1) {
            return -2;
        }
    }

    cf->Seek(event_table.at(current).offset);

    uint16_t event = ParseThreadList(options, &process, cf);
    if (!strcmp(mode, "fast") &&
        (event >= DUMP_REASON_signal && event < DUMP_REASON_dbus)) {
        // FIX pc,ra,sp , so backtrace can work right?

        for (auto& i:process.threads) {
            if (i.tid != process.crashing_tid) {
                continue;
            }

#if defined(__mips64) || defined(__sw_64)
            int pos = i.regs.s5 - i.stack_addr;
            if (pos >= 0 && pos < i.stack.size()) {
                memcpy(&i.regs, i.stack.data() + pos, sizeof(i.regs));
                i.regs.sp += 512; //see syscall_wrapper in mips64/sunway64
                i.regs.ra = i.regs.t2; //see syscall_wrapper in mips64/sunway64
                i.regs.pc = i.regs.t3; // see build_trampoline in mips64/sunway64
            }
#elif defined(__aarch64__)
            int pos = i.regs.x28 - i.stack_addr;
            if (pos >= 0 && pos < i.stack.size()) {
                memcpy(&i.regs, i.stack.data() + pos, sizeof(i.regs));
                i.regs.sp += 512; //see syscall_wrapper in arm64
                i.regs.ra = i.regs.x11; //see syscall_wrapper in arm64
                i.regs.pc = i.regs.x10; // see build_trampoline in arm64
            }
#endif
        }
    }

    MapsEntry target;
    target.offset = 0;
    target.time = event_table.at(current).time;
    auto upper = upper_bound(maps_table.begin(),
            maps_table.end(), target,
            [](auto& a, auto& b){ return (a.time < b.time); });
    if (upper == maps_table.end()) upper = maps_table.end() - 1;

    // These parse order can't be changed,
    // see TraceSession::dump_maps, dump_auxv, dump_proc_file!
    mf->Seek(maps_table.at(0).offset);
    ParseAuxVector(options, &process, mf);

    ParseCmdLine(options, &process, mf);

    // load environ
    string env; mf->ReadBlock(env);
    if (options.verbose) {
        const char* walk = env.data();
        LOG(INFO) << "EMD_LINUX_ENVIRON:";
        for (int i = 0; *walk > 0; ++i) {
            if (memcmp(walk, "LS_COLORS=", 8)) {
                LOG(INFO) << i << ":" << walk;
            }
            else {
                LOG(INFO) << i << ":LS_COLORS=...";
            }
            walk += strlen(walk) + 1;
        }
    }

    // Load vdso
    mf->ReadBlock(process.vdso);
    // NOTE: vdso is not implemented in SunWay!
    if (process.vdso.size() > 1024) {
        int position = process.vdso.size() - sizeof(uintptr_t);
        assert(0 == (position & 1023));
        memcpy(&process.vdso_addr,
                process.vdso.data() + position, sizeof(uintptr_t));
        process.vdso.erase(position);
    }

    if (upper > maps_table.begin()) {
        mf->Seek(upper->offset);
    }
    string blk; mf->ReadBlock(blk);
    MemoryRange maps(blk);
    ParseMaps(options, &process, maps);

    ParseModuleList(options, &process, mf);

    ParseDSODebugInfo(options, &process, mf);
    if (process.debug.dso_count < 1) {
        return -3;
    }

    AugmentMappings(options, &process);

    // Write the ELF header. The file will look like:
    //   ELF header
    //   Phdr for the PT_NOTE
    //   Phdr for each of the thread stacks
    //   PT_NOTE
    //   each of the thread stacks
    Ehdr ehdr;
    memset(&ehdr, 0, sizeof(Ehdr));
    ehdr.e_ident[0] = ELFMAG0;
    ehdr.e_ident[1] = ELFMAG1;
    ehdr.e_ident[2] = ELFMAG2;
    ehdr.e_ident[3] = ELFMAG3;
    ehdr.e_ident[4] = ELF_CLASS;
    ehdr.e_ident[5] = sex() ? ELFDATA2MSB : ELFDATA2LSB;
    ehdr.e_ident[6] = EV_CURRENT;
    ehdr.e_type     = ET_CORE;
    ehdr.e_machine  = ELF_ARCH;
    ehdr.e_version  = EV_CURRENT;
    ehdr.e_phoff    = sizeof(Ehdr);
    ehdr.e_ehsize   = sizeof(Ehdr);
    ehdr.e_phentsize= sizeof(Phdr);
    ehdr.e_phnum    = 1 +           // PT_NOTE
                  process.mappings.size();  // memory mappings
    ehdr.e_shentsize= sizeof(Shdr);
    if (!write_file(options, &ehdr, sizeof(Ehdr))) {
        return -4;
    }

#if defined(GENERATE_NT_FILE)
    long filemapinfo[5];
    int filenamelen = strlen("/home/deepin/chliu/event_recorder/debug/testclone") + 1;
#endif

    size_t offset = sizeof(Ehdr) + ehdr.e_phnum * sizeof(Phdr);
    size_t filesz = sizeof(Nhdr) + 8 + sizeof(prpsinfo) +
                sizeof(Nhdr) + 8 + process.auxv.length() +
#if defined(GENERATE_NT_FILE)
                sizeof(Nhdr) + 8 + sizeof(filemapinfo) + (filenamelen+15)/16*16 +
#endif
                process.threads.size() * (
                  (sizeof(Nhdr) + 8 + sizeof(prstatus))
                   + sizeof(Nhdr) + 8 + sizeof(USER_FPREGS)
#if defined(__i386__)
                   + sizeof(Nhdr) + 8 + sizeof(user_fpxregs_struct)
#endif
                    );

    Phdr phdr;
    memset(&phdr, 0, sizeof(Phdr));
    phdr.p_type = PT_NOTE;
    phdr.p_offset = offset;
    phdr.p_filesz = filesz;
    if (!write_file(options, &phdr, sizeof(phdr))) {
        return -5;
    }

    phdr.p_type = PT_LOAD;
    phdr.p_align = 4096;
    size_t note_align = phdr.p_align - ((offset+filesz) % phdr.p_align);
    if (note_align == phdr.p_align) {
        note_align = 0;
    }
    offset += note_align;

    for (std::map<uintptr_t, DumpedProcess::Mapping>::const_iterator iter =
         process.mappings.begin();
        iter != process.mappings.end(); ++iter) {
        const DumpedProcess::Mapping& mapping = iter->second;
        if (mapping.permissions == 0xFFFFFFFF) {
            // This is a map that we found in MD_MODULE_LIST_STREAM (as opposed to
            // MD_LINUX_MAPS). It lacks some of the information that we would like
            // to include.
            phdr.p_flags = PF_R;
        } else {
            phdr.p_flags = mapping.permissions;
        }
        phdr.p_vaddr = mapping.start_address;
        phdr.p_memsz = mapping.end_address - mapping.start_address;
        if (mapping.data.size()) {
            offset += filesz;
            filesz = mapping.data.size();
            phdr.p_filesz = mapping.data.size();
            phdr.p_offset = offset;
        } else {
            phdr.p_filesz = 0;
            phdr.p_offset = 0;
        }
        if (!write_file(options, &phdr, sizeof(phdr))) {
            return -6;
        }
    }

    Nhdr nhdr;
    memset(&nhdr, 0, sizeof(nhdr));
    nhdr.n_namesz = 5;
    nhdr.n_descsz = sizeof(prpsinfo);
    nhdr.n_type = NT_PRPSINFO;
    if (!write_file(options, &nhdr, sizeof(nhdr)) ||
        !write_file(options, "CORE\0\0\0\0", 8) ||
        !write_file(options, &process.prps, sizeof(prpsinfo))) {
        return -7;
    }

    nhdr.n_descsz = process.auxv.length();
    nhdr.n_type = NT_AUXV;
    if (!write_file(options, &nhdr, sizeof(nhdr)) ||
        !write_file(options, "CORE\0\0\0\0", 8) ||
        !write_file(options, process.auxv.data(), process.auxv.length())) {
        return -8;
    }

#if defined(GENERATE_NT_FILE)
    nhdr.n_descsz = sizeof(filemapinfo) + (filenamelen+15)/16*16;
    nhdr.n_type = NT_FILE;
    if (!write_file(options, &nhdr, sizeof(nhdr)) ||
        !write_file(options, "CORE\0\0\0\0", 8)) {
    }
    // write file map count
    // write file map entry(long start, long end, long offset)
    filemapinfo[0] = 1;
    filemapinfo[1] = 1;
    filemapinfo[2] = 0xaaaaab8000LL;
    filemapinfo[3] = 0xaaaaabc000LL;
    filemapinfo[4] = 0;
    write_file(options, filemapinfo, sizeof(filemapinfo));
    // write file name array
    write_file(options,
            "/home/deepin/chliu/event_recorder/debug/testclone",
            filenamelen);
    int align = 16 - (filenamelen & 15);
    if (align > 0) {
        memset(filemapinfo, 0, align);
        write_file(options, filemapinfo, align);
    }
#endif

    for (unsigned i = 0; i < process.threads.size(); ++i) {
        if (process.threads[i].tid == process.crashing_tid) {
            WriteThread(options, process.threads[i], process.fatal_signal);
            break;
        }
    }

    for (unsigned i = 0; i < process.threads.size(); ++i) {
        if (process.threads[i].tid != process.crashing_tid)
            WriteThread(options, process.threads[i], 0);
    }

    if (note_align) {
        unique_ptr<char[]> scratch = make_unique<char[]>(note_align);
        memset(scratch.get(), 0, note_align);
        if (!write_file(options, scratch.get(), note_align)) {
            return -9;
        }
    }

    for (std::map<uintptr_t, DumpedProcess::Mapping>::const_iterator iter =
           process.mappings.begin();
        iter != process.mappings.end(); ++iter) {
        const DumpedProcess::Mapping& mapping = iter->second;
        if (mapping.data.size()) {
            if (options.verbose) {
                LOG(INFO) << "write data mapping:" << HEX(mapping.start_address)
                    << ", " << mapping.data.size();
            }
            if (!write_file(options, mapping.data.c_str(), mapping.data.size())) {
                return -10;
            }
        }
    }

    if (options.verbose) {
        LOG(DEBUG) << "Create coredump file:" << core_file;
    }

    return 0;
}

int Timeline::open_trace(const char* maps_file, const char* context_file)
{
    if (!maps_reader.Open(maps_file, true)) {
        return -1;
    }

    if (!ctx_reader.Open(context_file, true)) {
        return -2;
    }

    // Always check the system info first, as that allows us to tell whether
    // this is a minidump file that is compatible with our converter.
    bool ok = ParseSystemInfo(&ctx_reader, true, mode);
    if (!ok) {
        LOG(ERROR) << "Invalid system info head in context file";
        return -3;
    }
    ok = ParseSystemInfo(&maps_reader, true, mode);
    if (!ok) {
        LOG(ERROR) << "Invalid system info head in maps file";
        return -4;
    }

    maps_reader.BuildMapsTable(&maps_table);
    if (maps_table.size() < 1) {
        LOG(ERROR) << "Invalid maps file.";
        return -5;
    }
    LOG(DEBUG) << "maps count:" << maps_table.size();

    ctx_reader.BuildEventTable(&event_table);
    if (event_table.size() < 1) {
        LOG(ERROR) << "Invalid context file.";
        return -6;
    }
    if (!strcmp(mode, "fast")) {
        sort(event_table.begin(), event_table.end(),
                [](auto&a, auto& b){return a.time < b.time;});
    }
    LOG(DEBUG) << "event count:" << event_table.size();

    current = 0;

    return (int)(event_table.size());
}

int create_timeline(const char* maps_file, const char* context_file, void** pp_timeline)
{
    static char g_init = 0;

    if (0 == g_init) {
        DumpConfig cfg;
        load_config(cfg);
        init_log(cfg, "emdv.log");
    }
    ++g_init;

    Timeline* timeline = new Timeline();
    int ret = timeline->open_trace(maps_file, context_file);

    if (ret > 0) *pp_timeline = timeline;

    return ret;
}

int destroy_timeline(void* timeline)
{
    Timeline* t = (reinterpret_cast<Timeline*>(timeline));
    if (t) {
        delete t;
    }

    return 0;
}

int dump_raw_event(const char* context_file)
{
    char mode[5];
    TraceReader ctx_reader;
    vector<EventEntry> event_table;

    if (!ctx_reader.Open(context_file, true)) {
        return -1;
    }

    // Always check the system info first, as that allows us to tell whether
    // this is a minidump file that is compatible with our converter.
    bool ok = ParseSystemInfo(&ctx_reader, true, mode);
    if (!ok) {
        LOG(ERROR) << "Invalid system info head in context file";
        return -2;
    }

    ctx_reader.BuildEventTable(&event_table);
    if (event_table.size() < 1) {
        LOG(ERROR) << "Invalid context file.";
        return -3;
    }

    for (auto& e : event_table) {
        printf("%s\n", get_event_name(e.type));
    }

    return 0;
}

const EventEntry* get_event_pointer(void* timeline)
{
    Timeline* t = (reinterpret_cast<Timeline*>(timeline));
    if (t) {
        return &t->event_table[0];
    }

    return nullptr;
}

int get_event(void* timeline, int index, EventEntry* out)
{
    Timeline* t = (reinterpret_cast<Timeline*>(timeline));
    if (t) {
        const EventEntry& entry = t->event_table.at(index);
        out->time = entry.time;
        out->duration = entry.duration;
        out->type = entry.type;
        out->thread_num = entry.thread_num;
        out->tid = entry.tid;
        out->extra_size = entry.extra_size;
        out->offset = entry.offset;
        out->syscall_result = entry.syscall_result;
    }

    return 0;
}


#if defined(__x86_64__)
#include "./x86_64/syscall_param.h"
#elif defined(__mips64)
#include "./mips64/syscall_param.h"
#elif defined(__sw_64)
#include "./sw64/syscall_param.h"
#elif defined(__aarch64__)
#include "./aarch64/syscall_param.h"
#else
#error need define new arch implement
#endif

static int get_syscall_ptr_args(USER_REGS* regs,
        unsigned char flags, unsigned char args_no, uintptr_t* out)
{
    uintptr_t args[6];
    int count = 0;
    // parameter is void*, size indicate in next parameter
    bool size_indicate_by_next = flags & 0x80;

#if defined(__x86_64__)
    // The kernel interface uses: %rdi, %rsi, %rdx, %r10, %r8 and %r9."
    args[0] = regs->rdi;
    args[1] = regs->rsi;
    args[2] = regs->rdx;
    args[3] = regs->r10;
    args[4] = regs->r8;
    args[5] = regs->r9;
#elif defined(__mips__) || defined(__mips64)
    //FIXME: The mips/o32 system call convention passes arguments 5 through 8 on the user
    //stack.
    args[0] = regs->a0;
    args[1] = regs->a1;
    args[2] = regs->a2;
    args[3] = regs->a3;
    args[4] = regs->a4;
    args[5] = regs->a5;
#elif defined(__sw_64)
    args[0] = regs->a0;
    args[1] = regs->a1;
    args[2] = regs->a2;
    args[3] = regs->a3;
    args[4] = regs->a4;
    args[5] = regs->a5;
#elif defined(__aarch64__)
    args[0] = regs->x0;
    args[1] = regs->x1;
    args[2] = regs->x2;
    args[3] = regs->x3;
    args[4] = regs->x4;
    args[5] = regs->x5;
#else
#error Not implment
#endif

    for (unsigned char i = 0; i < args_no; ++i) {
        if (flags & 1) {
            uintptr_t addr = args[i];
            if (addr > 0) {
                if (size_indicate_by_next) {
                    if (args[i+1] > 0) {
                        out[count++] = addr;
                    }
                    break;
                }
                else {
                    out[count++] = addr; // Null terminate string
                }
            }
            else {
                out[count++] = 1; //indicate null ptr
            }
        }
        flags >>= 1;
    }

    return count;
}

static char* load_memory(string& stack, uintptr_t stack_start,
       uintptr_t* str_ptr, int* count, char* walk, char* walk_end)
{
    int done = 0;
    int todo = *count;
    uintptr_t stack_end = stack_start + stack.size();

    if (todo >= 6) return walk; // not init

    for (int i = 0; i < 6; ++i) {
        if (0 == str_ptr[i]) continue; // has done

        if (1 == str_ptr[i]) {
            strcpy(walk, "null");
            walk += 4;
            *walk++ = '\n';
            *walk = 0;
            ++done;
            str_ptr[i] = 0; // has loaded.
            continue;
        }

        if (stack_start <= str_ptr[i] && stack_end > str_ptr[i]) {
            const char* ptr = stack.data() + str_ptr[i] - stack_start;
            const char* ptr_end = stack.data() + stack.size();
            while (*ptr > 0 &&
                    *ptr != '\n' &&  // limited one line
                    ptr < ptr_end &&
                    walk < walk_end) {
                *walk++ = *ptr++;
            }
            *walk++ = '\n';
            *walk = 0;
            ++done;
            str_ptr[i] = 0; // has loaded.
        }
    }

    *count = todo - done;

    return walk;
}

int get_event_extra_info(void* timeline, int index, char* buf, int buf_size)
{
    Timeline* t = (reinterpret_cast<Timeline*>(timeline));
    if (!t) {
        return 0;
    }

    const EventEntry& entry = t->event_table.at(index);
    if (buf && buf_size > entry.extra_size) {
        t->ctx_reader.Seek(entry.offset + sizeof(EventHead));
        t->ctx_reader.ReadBlock(buf, entry.extra_size);
        buf[entry.extra_size] = 0;
    }

    if (entry.type >= DUMP_REASON_signal) {
        return entry.extra_size;
    }

    // syscall evnet, load context of char* parameter
    int syscall_no = entry.type - __NR_Linux;
    int flags = syscall_param_flags[2*syscall_no];
    if (!flags) {
        return 0;
    }

    // load register
    t->ctx_reader.Seek(entry.offset + sizeof(EventHead));
    if (entry.extra_size > 0) {
        // delay load them in get_event_extra_info
        t->ctx_reader.Seek(t->ctx_reader.Current() + entry.extra_size);
    }

    USER_REGS regs;
    USER_FPREGS fpregs;
    string stack;
    uintptr_t stack_start;
    uintptr_t str_ptr[6];
    int str_ptr_count = 6;
    char* walk = buf;
    char* walk_end = buf + buf_size - 2;

    for (int i = 0; i < entry.thread_num && str_ptr_count > 0; ++i) {
        uint32_t tid = 0;
        t->ctx_reader.ReadValue<uint32_t>(&tid);
        t->ctx_reader.ReadBlock(&regs, sizeof(regs));
        t->ctx_reader.ReadBlock(&fpregs, sizeof(fpregs));

        string tls; t->ctx_reader.ReadBlock(tls); //skip tls data

        t->ctx_reader.ReadValue<uintptr_t>(&stack_start);
        t->ctx_reader.ReadBlock(stack);

        if ((tid & 0xffff) == entry.tid) {
            memset(str_ptr, 0, sizeof(str_ptr));
            str_ptr_count = get_syscall_ptr_args(&regs, flags,
                 syscall_param_flags[2*syscall_no + 1], &str_ptr[0]);
        }

        walk = load_memory(stack, stack_start,
                str_ptr, &str_ptr_count, walk, walk_end);
    }

    // search heap block
    uint32_t heap_count = 0;
    t->ctx_reader.ReadValue<uint32_t>(&heap_count);
    for (uint32_t i = 0; i < heap_count && str_ptr_count > 0; ++i) {
        t->ctx_reader.ReadValue<uintptr_t>(&stack_start);
        t->ctx_reader.ReadBlock(stack);
        if (!stack.empty()) {
            walk = load_memory(stack, stack_start,
                    str_ptr, &str_ptr_count, walk, walk_end);
        }
    }

    return (walk - buf);
}

int generate_coredump(void* timeline, int index, const char* corefile, int verbose)
{
    Timeline* t = (reinterpret_cast<Timeline*>(timeline));
    if (t) {
        return t->generate_coredump(index, corefile, verbose);
    }

    return 0;
}

static void __attribute__((constructor)) init_process(void) {
}
