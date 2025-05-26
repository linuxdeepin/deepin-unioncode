// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <assert.h>
#include <math.h>
#include <fnmatch.h>
#include <limits.h>
#include <linux/net.h>
#include <linux/seccomp.h>
#include <linux/if_tun.h>
#include <linux/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#if defined(__aarch64__)
#include "./aarch64/syscall.h"
#else
#include <sys/syscall.h>
#endif

#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/un.h>

#include "cpu.h"
#include "debug.h"
#include "session.h"
#include "utils.h"
#include "event_man.h"
#include "WaitStatus.h"
#include "easylogging++.h"
#include "syscall_name.h"
#include "syscall_filter.h"

using namespace std;

static int g_nonce = 0;

///////////////////////////////////////////////////////////////////////
// syscall filter utility
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

const struct_sysent g_sysent0[] = {
#if defined(__x86_64__)
#include "./x86_64/syscallent.h"
#elif defined(__mips64)
#include "./mips64/syscallent-n64.h"
#elif defined(__sw_64)
#include "./sw64/syscallent.h"
#elif defined(__aarch64__)
#include "./aarch64/syscallent.h"
#else
#error need define new arch implement
#endif
};
enum SyscallCount{
    nsyscalls0 = ARRAY_SIZE(g_sysent0)
};

const unsigned int g_nsyscall_vec[SUPPORTED_PERSONALITIES] = {
    nsyscalls0
};

const struct_sysent *const g_sysent_vec[SUPPORTED_PERSONALITIES] = {
    g_sysent0,
};

static struct number_set g_trace_set[SUPPORTED_PERSONALITIES];

static void number_setbit(const unsigned int i, number_slot_t *const vec)
{
    vec[i / BITS_PER_SLOT] |= (number_slot_t) 1 << (i % BITS_PER_SLOT);
}

inline bool number_isset(const unsigned int i, const number_slot_t *const vec)
{
    return vec[i / BITS_PER_SLOT] & ((number_slot_t) 1 << (i % BITS_PER_SLOT));
}

static void reallocate_number_set(struct number_set *const set,
        const unsigned int new_nslots)
{
    if (new_nslots <= set->nslots)
        return;
    set->vec = (number_slot_t*)realloc(set->vec, new_nslots*sizeof(*set->vec));
    memset(set->vec + set->nslots, 0,
           sizeof(*set->vec) * (new_nslots - set->nslots));
    set->nslots = new_nslots;
}

static void add_number_to_set(const unsigned int number,
        struct number_set *const set)
{
    reallocate_number_set(set, number / BITS_PER_SLOT + 1);
    number_setbit(number, set->vec);
}

static bool is_number_in_set(const unsigned int number, struct number_set* set)
{
    if (0 == set[0].nslots) return set[0].not_flag;
    unsigned int no = number - __NR_Linux;
    return (((no / BITS_PER_SLOT < set[0].nslots) &&
              number_isset(no, set[0].vec)) ^ set[0].not_flag);
}

static bool qualify_syscall_number(const char *s, struct number_set *set)
{
    int n = atoi(s);
    if (n < 0)
        return false;

    unsigned int p;
    bool done = false;

    for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
        if ((unsigned) n >= g_nsyscall_vec[p]) {
            continue;
        }
        add_number_to_set(n, &set[p]);
        done = true;
    }

    return done;
}

static unsigned int lookup_class(const char *s)
{
    static const struct {
        const char *name;
        unsigned int value;
    } syscall_class[] = {
        { "desc",    TRACE_DESC},
        { "file",    TRACE_FILE},
        { "memory",  TRACE_MEMORY},
        { "process", TRACE_PROCESS},
        { "signal",  TRACE_SIGNAL},
        { "ipc",     TRACE_IPC},
        { "network", TRACE_NETWORK},
    };

    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(syscall_class); ++i) {
        if (strcmp(s, syscall_class[i].name) == 0) {
            return syscall_class[i].value;
        }
    }

    return 0;
}

static bool qualify_syscall_class(const char *s, struct number_set *set)
{
    const unsigned int n = lookup_class(s);
    if (!n)
        return false;

    unsigned int p;
    for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
        unsigned int i;

        for (i = 0; i < g_nsyscall_vec[p]; ++i) {
            if (!g_sysent_vec[p][i].sys_name
                || (g_sysent_vec[p][i].sys_flags & n) != n) {
                continue;
            }
            add_number_to_set(i, &set[p]);
        }
    }

    return true;
}

static bool qualify_syscall_name(const char *s, struct number_set *set)
{
    unsigned int p;
    bool found = false;

    for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
        unsigned int i;

        for (i = 0; i < g_nsyscall_vec[p]; ++i) {
            if (!g_sysent_vec[p][i].sys_name
                || strcmp(s, g_sysent_vec[p][i].sys_name)) {
                continue;
            }
            add_number_to_set(i, &set[p]);
            found = true;
        }
    }

    return found;
}

static bool qualify_syscall(const char *token, struct number_set *set)
{
    if (*token >= '0' && *token <= '9')
        return qualify_syscall_number(token, set);
    return qualify_syscall_class(token, set) ||
        qualify_syscall_name(token, set);
}

/*
 * Add syscall numbers to SETs for each supported personality
 * according to STR specification.
 */
static void qualify_syscall_tokens(const char *const str,
        struct number_set *const set, const char *const name, bool reset)
{
    /* Clear all sets. */
    unsigned int p;
    if (reset) {
        for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
            if (set[p].nslots)
                memset(set[p].vec, 0,
                       sizeof(*set[p].vec) * set[p].nslots);
            set[p].not_flag = false;
        }
    }

    /*
     * Each leading ! character means inversion
     * of the remaining specification.
     */
    const char *s = str;
handle_inversion:
    while (*s == '!') {
        for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
            set[p].not_flag = !set[p].not_flag;
        }
        ++s;
    }

    if (strcmp(s, "none") == 0) {
        /*
         * No syscall numbers are added to sets.
         * Subsequent is_number_in_set invocations
         * will return set[p]->not_flag.
         */
        return;
    } else if (strcmp(s, "all") == 0) {
        s = "!none";
        goto handle_inversion;
    }

    /*
     * Split the string into comma separated tokens.
     * For each token, call qualify_syscall that will take care
     * if adding appropriate syscall numbers to sets.
     * The absence of tokens or a negative return code
     * from qualify_syscall is a fatal error.
     */
    char *copy = strdup(s);
    char *saveptr = NULL;
    const char *token;
    bool done = false;

    for (token = strtok_r(copy, ",", &saveptr); token;
         token = strtok_r(NULL, ",", &saveptr)) {
        done = qualify_syscall(token, set);
        if (!done) {
            LOG(ERROR) << "invalid " <<  name << ":" << token;
        }
    }

    free(copy);

    if (!done) {
        LOG(ERROR) << "invalid " <<  name << ":" << token;
    }
}

void set_syscall_filter(const char* filter, bool reset)
{
    qualify_syscall_tokens(filter, g_trace_set, "system call", reset);
}

static void link_exec_file(string& parent_dir, int pid)
{
    char exe_path[500];
    char linkname[500];
    snprintf(linkname, sizeof(linkname), "/proc/%d/exe", pid);
    int len = readlink(linkname, exe_path, sizeof(exe_path));
    if (len < 0) {
            LOG(ERROR) << "failed to readlink for tracee " << pid
                << ", errno=" << errno;
            return;
    }
    exe_path[len] = 0;

    snprintf(linkname, sizeof(linkname), "%s%s%d",
            parent_dir.data(), EXEC_FILE_NAME, pid);
    FILE* pf = fopen(linkname, "wb");
    if (pf) {
        fprintf(pf, "%s", exe_path);
        fclose(pf);
    }
}

///////////////////////////////////////////////////////////////////////
//ptrace utility

#define CASE(_id)   \
  case _id:         \
    return #_id;

const char* ptrace_event_name(int event) {
  switch (event) {
    CASE(PTRACE_EVENT_FORK);
    CASE(PTRACE_EVENT_VFORK);
    CASE(PTRACE_EVENT_CLONE);
    CASE(PTRACE_EVENT_EXEC);
    CASE(PTRACE_EVENT_VFORK_DONE);
    CASE(PTRACE_EVENT_EXIT);
    /* XXX Ubuntu 12.04 defines a "PTRACE_EVENT_STOP", but that
     * has the same value as the newer EVENT_SECCOMP, so we'll
     * ignore STOP. */
    CASE(PTRACE_EVENT_SECCOMP_OBSOLETE);
    CASE(PTRACE_EVENT_SECCOMP);
    CASE(PTRACE_EVENT_STOP);
    /* Special-case this so we don't need to sprintf in this common case.
     * This case is common because we often pass ptrace_event_name(event) to
     * assertions when event is 0.
     */
    case 0:
      return "PTRACE_EVENT(0)";
    default: {
      static char buf[100];
      sprintf(buf, "PTRACE_EVENT(%d)", event);
      return (buf);
    }
  }
}

static int ReadProcess(void* dest, TraceStream* stream, const void* src, int length)
{
    if (src == nullptr) return 0;

    int got = stream->read((uintptr_t)src, dest, length);
    if (got > 0) {
        return got;
    }

    return read_mem(stream->get_pid(), (uintptr_t)src, dest, length);
}

bool inline my_ptrace(const char* reason,
        enum __ptrace_request request, pid_t pid, void *addr, void *data)
{
    int ret = ptrace(request, pid, addr, data);
    if (ret < 0) {
        LOG(ERROR) << ptrace_cmd_name(request)
            << " failed for tracee " << pid
            << ", reason=" << reason
            << ", errno=" << errno;

        return false;
    }

#ifdef _DEBUG
    LOG(DEBUG) << ptrace_cmd_name(request) << " ok for tracee " << pid
        << ", reason=" << reason;
#endif

    return true;
}

inline long get_syscall_no(USER_REGS* regs)
{
#if defined(__x86_64__)
    return regs->orig_rax;
#elif defined(__mips__) || defined(__mips64)
    return regs->v0;
#elif defined(__sw_64)
    return regs->v0;
#elif defined(__aarch64__)
    return regs->x8;
#else
#error Not implment
#endif
}

inline long get_syscall_result(USER_REGS* regs)
{
#if defined(__x86_64__)
    return regs->rax;
#elif defined(__mips__) || defined(__mips64)
    // $a3 set to 0/1 for success/error
    return regs->v0;
#elif defined(__sw_64)
    // $a3 set to 0/1 for success/error
    return regs->v0;
#elif defined(__aarch64__)
    return regs->x0;
#else
#error Not implment
#endif
}

static void get_syscall_args(USER_REGS* regs, uintptr_t* args)
{
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
}

///////////////////////////////////////////////////////////////////////////////
// auxv and elf helper function

#include "elf_parse.h"

bool is_elf(const char* filename)
{
    MemoryMappedFile mapped_file(filename, O_RDONLY);
    if (!mapped_file.data() ||
        mapped_file.size() < SELFMAG) {
        return false;
    }
    if (!IsValidElf(mapped_file.data())) {
        return false;
    }

    int cls = ElfClass(mapped_file.data());
    if (cls == ELFCLASS32) {
        return false;
    }
    else if (cls == ELFCLASS64) {
        return true;
    }

    return false;
}

static bool GetElfSectionRanges(const char* elf_mapped_base,
        const char** section_names, int* section_types, int* ranges, int count)
{
    const char* addr = NULL;
    size_t size = 0;
    for (int i = 0; i<count; ++i) {
        if (!FindElfSection(elf_mapped_base, section_names[i], section_types[i],
                         (const void**)&addr, &size)) {
            return false;
        }
        ranges[2*i+0] = (int)(addr - elf_mapped_base);
        ranges[2*i+1] = ranges[2*i] + (int)size;
    }

    return true;
}

// Attempt to locate a .note.gnu.build-id section in an ELF binary
// and copy it into |identifier|.
static bool FindElfBuildIDNote(const void* elf_mapped_base,
                               uint8_t* identifier)
{
  // lld normally creates 2 PT_NOTEs, gold normally creates 1.
  vector<ElfSegment> segs;
  if (FindElfSegments(elf_mapped_base, PT_NOTE, &segs)) {
    for (ElfSegment& seg : segs) {
      if (ElfClassBuildIDNoteIdentifier(seg.start, seg.size, identifier)) {
        return true;
      }
    }
  }

  void* note_section;
  size_t note_size;
  if (FindElfSection(elf_mapped_base, ".note.gnu.build-id", SHT_NOTE,
                     (const void**)&note_section, &note_size)) {
    return ElfClassBuildIDNoteIdentifier(note_section, note_size, identifier);
  }

  return false;
}

// Attempt to locate the .text section of an ELF binary and generate
// a simple hash by XORing the first page worth of bytes into |identifier|.
static bool HashElfTextSection(const void* elf_mapped_base,
                               uint8_t* identifier)
{
  void* text_section;
  size_t text_size;
  if (!FindElfSection(elf_mapped_base, ".text", SHT_PROGBITS,
                      (const void**)&text_section, &text_size) ||
      text_size == 0) {
    return false;
  }

  // Only provide |kMDGUIDSize| bytes to keep identifiers produced by this
  // function backwards-compatible.
  memset(identifier, 0, kMDGUIDSize);
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(text_section);
  const uint8_t* ptr_end = ptr + std::min(text_size, static_cast<size_t>(4096));
  while (ptr < ptr_end) {
    for (unsigned i = 0; i < kMDGUIDSize; i++)
      identifier[i] ^= ptr[i];
    ptr += kMDGUIDSize;
  }
  return true;
}

bool ElfFileIdentifierFromMappedFile(const void* base, uint8_t* identifier)
{
    // Look for a build id note first.
    if (FindElfBuildIDNote(base, identifier))
        return true;

    // Fall back on hashing the first page of the text section.
    return HashElfTextSection(base, identifier);
}

static void get_system_info(MDRawSystemInfo* info)
{
    memset(info, 0, sizeof(*info));

#if defined(__i386__)
    info->processor_architecture = MD_CPU_ARCHITECTURE_X86;
#elif defined(__x86_64__)
    info->processor_architecture = MD_CPU_ARCHITECTURE_AMD64;
#elif defined(__arm__)
    info->processor_architecture = MD_CPU_ARCHITECTURE_ARM;
#elif defined(__aarch64__)
    info->processor_architecture = MD_CPU_ARCHITECTURE_ARM64;
#elif defined(__mips__)
# if _MIPS_SIM == _ABIO32
    info->processor_architecture = MD_CPU_ARCHITECTURE_MIPS;
# elif _MIPS_SIM == _ABI64
    info->processor_architecture = MD_CPU_ARCHITECTURE_MIPS64;
# else
#  error "This mips ABI is currently not supported (n32)"
# endif
#elif defined(__sw_64)
    info->processor_architecture = MD_CPU_ARCHITECTURE_SW64;
#else
#error "This code has not been ported to your platform yet"
#endif

    // dump deepin version and Linux kernel version
    MemoryMappedFile lsb_release("/etc/lsb-release", 0);
    uint32_t length = lsb_release.size();
    memcpy(info->lsb_release, lsb_release.data(),
            (length > sizeof(info->lsb_release)) ?
            sizeof(info->lsb_release) : length);

    system("uname -a > /tmp/.uname.nux");
    MemoryMappedFile uname("/tmp/.uname.nux", 0);
    length = uname.size();
    memcpy(info->uname, uname.data(),
            (length > sizeof(info->lsb_release)) ?
            sizeof(info->lsb_release) : length);
}

bool EnumerateMappings(vector< shared_ptr<MappingInfo> >& mappings,
        vector<elf_aux_val_t>& auxv, int pid, VmSegment* stack, VmSegment* heap)
{
  char maps_path[NAME_MAX];
  snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

  // linux_gate_loc is the beginning of the kernel's mapping of
  // linux-gate.so in the process.  It doesn't actually show up in the
  // maps list as a filename, but it can be found using the AT_SYSINFO_EHDR
  // aux vector entry, which gives the information necessary to special
  // case its entry when creating the list of mappings.
  // See http://www.trilithium.com/johan/2005/08/linux-gate/ for more
  // information.
  const void* linux_gate_loc =
      reinterpret_cast<void *>(auxv[AT_SYSINFO_EHDR]);
  // Although the initial executable is usually the first mapping, it's not
  // guaranteed (see http://crosbug.com/25355); therefore, try to use the
  // actual entry point to find the mapping.
  const void* entry_point_loc = reinterpret_cast<void *>(auxv[AT_ENTRY]);

  const int fd = open(maps_path, O_RDONLY, 0);
  if (fd < 0)
    return false;

  unique_ptr<LineReader> line_reader = make_unique<LineReader>(fd);

  const char* line = nullptr;
  unsigned int line_len = 0;
  while (line_reader->GetNextLine(&line, &line_len)) {
    int offset;
    uintptr_t start_addr, end_addr;

    char* i1 = nullptr;
    start_addr = strtoul(line, &i1, 16);
    if (*i1 == '-') {
      char* i2 = nullptr;
      end_addr = strtoul(i1+1, &i2, 16);
      if (*i2 == ' ') {
        // bool write = (*(i2 + 2) == 'w');
        bool exec = (*(i2 + 3) == 'x');
        char* i3 = nullptr;
        offset = strtol(i2 + 6, &i3, 16/* skip ' rwxp ' */);
        if (*i3 == ' ') {
          const char* name = nullptr;
          // Only copy name if the name is a valid path name, or if
          // it's the VDSO image.
          if ((name = strchr(i3, '/')) == nullptr){
             if (linux_gate_loc &&
              reinterpret_cast<void*>(start_addr) == linux_gate_loc) {
                name = kLinuxGateLibraryName;
                offset = 0;
            }
            else if (strstr(i3, "[heap]") != nullptr) {
                heap->start = start_addr;
                heap->end = end_addr;
            }
            else if (strstr(i3, "[stack]") != nullptr) {
                stack->start = start_addr;
                stack->end = end_addr;
            }
          }
          // Merge adjacent mappings into one module, assuming they're a single
          // library mapped by the dynamic linker. Do this only if their name
          // matches and either they have the same +x protection flag, or if the
          // previous mapping is not executable and the new one is, to handle
          // lld's output (see crbug.com/716484).
          if (name && !mappings.empty()) {
            MappingInfo* module = mappings.back().get();
            if ((start_addr == module->start_addr + module->size) &&
                (strlen(name) == strlen(module->name)) &&
                (strncmp(name, module->name, strlen(name)) == 0) &&
                (offset > module->offset) &&
                ((exec == module->exec) || (!module->exec && exec))) {
              module->size = end_addr - module->start_addr;
              module->exec |= exec;
              line_reader->PopLine(line_len);
              continue;
            }
          }
          shared_ptr<MappingInfo> module = make_shared<MappingInfo>();
          mappings.push_back(module);
          memset(module.get(), 0, sizeof(MappingInfo));
          module->start_addr = start_addr;
          module->size = end_addr - start_addr;
          module->offset = offset;
          module->exec = exec;
          if (name != nullptr) {
            const unsigned int l = strlen(name);
            assert(l < sizeof(module->name));
            memcpy(module->name, name, l);
          }
        }
      }
    }
    line_reader->PopLine(line_len);
  }

  if (entry_point_loc) {
    for (size_t i = 0; i < mappings.size(); ++i) {
      auto module = mappings[i];

      // If this module contains the entry-point, and it's not already the first
      // one, then we need to make it be first.  This is because the minidump
      // format assumes the first module is the one that corresponds to the main
      // executable (as codified in
      // processor/minidump.cc:MinidumpModuleList::GetMainModule()).
      if ((entry_point_loc >= reinterpret_cast<void*>(module->start_addr)) &&
          (entry_point_loc <
           reinterpret_cast<void*>(module->start_addr + module->size))) {
        for (size_t j = i; j > 0; j--) {
          mappings[j] = mappings[j - 1];
        }
        mappings[0] = module;
        break;
      }
    }
  }

  close(fd);

  return !mappings.empty();
}

static bool ShouldIncludeMapping(const MappingInfo& mapping)
{
    // TODO: should exlucde /etc/ld.so.cache
    if (mapping.name[0] == 0 ||  // only want modules with filenames.
        // Only want to include one mapping per shared lib.
        // Avoid filtering executable mappings.
        (mapping.offset != 0 && !mapping.exec) ||
        mapping.size < 4096) {  // too small to get a signature for.
      return false;
    }

    return true;
}

inline void GetMappingEffectivePath(const MappingInfo& mapping,
                             char* file_path, size_t file_path_size)
{
    strncpy(file_path, mapping.name, file_path_size);

    // If an executable is mapped from a non-zero offset, this is likely because
    // the executable was loaded directly from inside an archive file (e.g., an
    // apk on Android). We try to find the name of the shared object (SONAME) by
    // looking in the file for ELF sections.
#if 0
    bool mapped_from_archive = false;
    if (mapping.exec && mapping.offset != 0) {
        LOG(DEBUG) << "Maybe load from an archive file, offset="
            << mapping.offset << ", file=" << file_path;
    }
#endif
}

/**
 * Returns the name of the first dynamic library that |exe_file| depends on
 * that starts with |prefix|, or an empty string if there isn't one or
 * anything fails.
 */
string find_needed_library_starting_with(const string& exe_file,
        const string& prefix)
{
    // load elf file!
    MemoryMappedFile mapped_file(exe_file.c_str(), O_RDONLY);
    if (!mapped_file.data() ||
        mapped_file.size() < SELFMAG) {
        return string();
    }
    if (!IsValidElf(mapped_file.data())) {
        return string();
    }

    const char* name = nullptr;
    int cls = ElfClass(mapped_file.data());
    if (cls == ELFCLASS32) {
        name = FindLibClassStartWith<ElfClass32>(mapped_file.data(),
                mapped_file.size(), prefix.data());
    }
    else if (cls == ELFCLASS64) {
        name = FindLibClassStartWith<ElfClass64>(mapped_file.data(),
                mapped_file.size(), prefix.data());
    }
    if (name) return string(name);
    return string();
}

typedef struct tagElfIdInfo{
    string guid;
    int rw_offset; // first rw_offset
    int addrs[4]; //.data and .bss
}ElfIdInfo;
static map<string, ElfIdInfo> g_guid_maps;

// Fill the MDRawModule |mod| with information about the provided
// |mapping|. If |identifier| is non-nullptr, use it instead of calculating
// a file ID from the mapping.
bool FillRawModule(const MappingInfo& mapping,
    ElfSymbolFiles& modules, TraceStream* stream, MDRawModule* mod)
{
    memset(mod, 0xcc, sizeof(MDRawModule));

    mod->base_of_image = mapping.start_addr;
    mod->size_of_image = mapping.size;
    GetMappingEffectivePath(mapping, mod->file_path, sizeof(mod->file_path));

    if (mapping.exec && modules.find(mapping.name) == modules.end()) {
        // not cached in current process, but maybe has cached in other process.
        // FIXME: some module has more than one execute segment,e.g.:
        /*
        7ffff7bbc000-7ffff7bd4000 r-xp 00000000 08:02 1449013 /usr/lib/x86_64-linux-gnu/libpthread-2.24.so
        7ffff7bd4000-7ffff7dd3000 ---p 00018000 08:02 1449013 /usr/lib/x86_64-linux-gnu/libpthread-2.24.so
        7ffff7dd3000-7ffff7dd9000 r-xp 00217000 08:02 1449013 /usr/lib/x86_64-linux-gnu/libpthread-2.24.so
        */
        std::shared_ptr<SymbolFile> helper = make_shared<SymbolFile>(
                mapping.start_addr, mapping.name);
        modules.insert(pair<string, shared_ptr<SymbolFile>>(
                    string(&mapping.name[0]), helper));
    }

    auto it = g_guid_maps.find(mapping.name);
    if (it != g_guid_maps.end()) {
        memcpy(mod->guid, it->second.guid.data(), kMDGUIDSize);
    }
    else {
        ElfIdInfo idinfo;
        // Special-case linux-gate because it's not a real file.
        // Look for a build id note first.
        // Fall back on hashing the first page of the text section.
        if (strcmp(mapping.name, kLinuxGateLibraryName) == 0) {
            // load linux_gate context.
            unique_ptr<char []> linux_gate = make_unique<char []>(mapping.size);
            ReadProcess(linux_gate.get(), stream,
                          reinterpret_cast<const void*>(mapping.start_addr),
                          mapping.size);
            ElfFileIdentifierFromMappedFile(linux_gate.get(), mod->guid);
        }
        // skip kMappedFileUnsafePrefix and kDeletedSuffix file
        else if ( strstr(mapping.name, kMappedFileUnsafePrefix) ||
                  strstr(mapping.name, kDeletedSuffix)) {
            return false;
        }
        else {
            // load the whole elf file!
            MemoryMappedFile mapped_file(mapping.name, 0);
            if (!mapped_file.data() ||
                mapped_file.size() < SELFMAG) {
                return false;
            }
            if (!IsValidElf(mapped_file.data())) {
                return false;
            }
            ElfFileIdentifierFromMappedFile(mapped_file.data(), mod->guid);

            int section_types[] = {SHT_PROGBITS, SHT_NOBITS};
            const char* section_names[] = {".data", ".bss"};
            memset(&idinfo.addrs[0], 0, sizeof(idinfo.addrs));
            GetElfSectionRanges((const char*)mapped_file.data(),
                    section_names, section_types, &idinfo.addrs[0],
                    sizeof(section_types)/sizeof(int));

            vector<ElfSegment> segs;
            if (FindElfSegments(mapped_file.data(), PT_LOAD, &segs)) {
                idinfo.rw_offset = (int)((const char*)segs[1].start - (char*)mapped_file.data());
            }
            else {
                idinfo.rw_offset = 0;
            }
        }

        idinfo.guid = string((char*)(mod->guid), kMDGUIDSize);
        g_guid_maps.insert(pair<string, ElfIdInfo>(string(&mapping.name[0]), idinfo));
    }

    return true;
}

// Write information about the mappings in effect. Because we are using the
// minidump format, the information about the mappings is pretty limited.
// Because of this, we also include the full, unparsed, /proc/$x/maps file in
// another stream in the file.
bool WriteMappings(ZstdWriter& file,
    const vector< shared_ptr<MappingInfo> >& mappings,
    ElfSymbolFiles& modules, TraceStream* stream)
{
    const unsigned int num_mappings = mappings.size();
    unsigned int num_output_mappings = 0;

    for (unsigned int i = 0; i < mappings.size(); ++i) {
      const MappingInfo& mapping = *mappings[i];
      if (ShouldIncludeMapping(mapping))
        num_output_mappings++;
    }

#ifdef _DEBUG2
    LOG(DEBUG) << "module count:" << num_output_mappings;
#endif

    file.write(&num_output_mappings, sizeof(num_output_mappings));

    for (unsigned int i = 0; i < num_mappings; ++i) {
      const MappingInfo& mapping = *mappings[i];
      if (!ShouldIncludeMapping(mapping))
        continue;

      MDRawModule mod;
      FillRawModule(mapping, modules, stream, &mod);
      file.write(&mod, sizeof(mod));
#ifdef _DEBUG2
      LOG(DEBUG) << "write module:" << mod.file_path
          << " at " << HEX(mod.base_of_image)
          << ", size=" << HEX(mod.size_of_image);
#endif
    }

    return true;
}

bool WriteDSODebugStream(ZstdWriter& file, vector<elf_aux_val_t>& auxv, TraceStream* stream)
{
    vector<MDRawLinkMap> dso_list;
    MDRawDebug debug;
    char* base = nullptr;
    int phnum = auxv[AT_PHNUM];
    int size = 0;
    ElfW(Phdr)* phdr = reinterpret_cast<ElfW(Phdr) *>(auxv[AT_PHDR]);
    if (!phnum || !phdr) {
      file.write(&size, sizeof(size));
      return false;
    }

    // Assume the program base is at the beginning of the same page as the PHDR
    base = reinterpret_cast<char *>(reinterpret_cast<uintptr_t>(phdr) & ~0xfff);

    // Search for the program PT_DYNAMIC segment
    ElfW(Addr) dyn_addr = 0;
    for (; phnum >= 0; phnum--, phdr++) {
      ElfW(Phdr) ph;
      if (sizeof(ph) != ReadProcess(&ph, stream, phdr, sizeof(ph))) {
        file.write(&size, sizeof(size));
        return false;
      }

      // Adjust base address with the virtual address of the PT_LOAD segment
      // corresponding to offset 0
      if (ph.p_type == PT_LOAD && ph.p_offset == 0) {
        base -= ph.p_vaddr;
      }
      if (ph.p_type == PT_DYNAMIC) {
        dyn_addr = ph.p_vaddr;
      }
    }
    if (!dyn_addr) {
      file.write(&size, sizeof(size));
      return false;
    }

    ElfW(Dyn) *dynamic = reinterpret_cast<ElfW(Dyn) *>(dyn_addr + base);

    // The dynamic linker makes information available that helps gdb find all
    // DSOs loaded into the program. If this information is indeed available,
    // dump it to a MD_LINUX_DSO_DEBUG stream.
    // see readelf -d for more information
    uint32_t dynamic_length = 0;
    struct r_debug* r_debug_addr = nullptr;

    for (int i = 0; ; ++i) {
      ElfW(Dyn) dyn;
      dynamic_length += sizeof(dyn);
      if (sizeof(dyn) != ReadProcess(&dyn, stream, dynamic + i, sizeof(dyn))) {
        file.write(&size, sizeof(size));
        return false;
      }

      // in gdb: see `info address _r_debug`
#ifdef __mips__
      if (dyn.d_tag == DT_MIPS_RLD_MAP) {
        r_debug_addr = reinterpret_cast<struct r_debug*>(dyn.d_un.d_ptr);
        ReadProcess(&r_debug_addr, stream, r_debug_addr, sizeof(r_debug_addr));
        continue;
      }
      else if (dyn.d_tag == DT_MIPS_RLD_MAP_REL) {
        /* NOTE:
           start from deepin 15.5 SP3, DT_MIPS_RLD_MAP has been changed to  DT_MIPS_RLD_MAP_REL
           And add a new .rld_map section.
           An alternative description of the classic MIPS RLD_MAP that is usable
           in a PIE as it stores a relative offset from the address of the tag
           rather than an absolute address.
        */
        char* rld_map = (char*)(dynamic + i) + dyn.d_un.d_ptr;
        ReadProcess(&r_debug_addr, stream, rld_map, sizeof(r_debug_addr));
        debug.rld_map = (uint64_t)rld_map;
        continue;
#else
      if (dyn.d_tag == DT_DEBUG) {
        r_debug_addr = reinterpret_cast<struct r_debug*>(dyn.d_un.d_ptr);
        continue;
#endif
      } else if (dyn.d_tag == DT_NULL) {
        break;
      }
    }

    // The "r_map" field of that r_debug struct contains a linked list of all
    // loaded DSOs.
    // Our list of DSOs potentially is different from the ones in the crashing
    // process. So, we have to be careful to never dereference pointers
    // directly. Instead, we use ReadProcess() everywhere.
    // See <link.h> for a more detailed discussion of the how the dynamic
    // loader communicates with debuggers.

    // Count the number of loaded DSOs
    struct r_debug debug_entry;
    if (sizeof(debug_entry) != ReadProcess(&debug_entry,
                stream, r_debug_addr, sizeof(debug_entry))) {
      file.write(&size, sizeof(size));
      return false;
    }

    LOG(DEBUG) << "load r_debug at: " << r_debug_addr
        << ", r_version=" << debug_entry.r_version
        << ", r_map=" << debug_entry.r_map
        << ", r_brk=" << HEX(debug_entry.r_brk)
        << ", r_state=" << HEX(debug_entry.r_state)
        << ", loader base :" << HEX(debug_entry.r_ldbase)
        << ", _DYNAMIC address :" << dynamic << "," << dynamic_length;

    for (struct link_map* ptr = debug_entry.r_map; ptr; ) {
      struct link_map map;
      if (sizeof(map) != ReadProcess(&map, stream, ptr, sizeof(map))) {
        file.write(&size, sizeof(size));
        return false;
      }

      MDRawLinkMap entry;
      if (map.l_name) {
        ReadProcess(entry.name, stream, map.l_name, sizeof(entry.name) - 1);
      }
      entry.name[sizeof(entry.name) -1] = 0;
      entry.addr = map.l_addr;
      entry.ld = reinterpret_cast<uintptr_t>(map.l_ld);
      dso_list.push_back(entry);

#ifdef _DEBUG2
      LOG(DEBUG) << "\tdso:" << HEX(map.l_addr) << ", " << entry.name;
#endif

      ptr = map.l_next;
    }

    // Write MD_LINUX_DSO_DEBUG record
    debug.version = debug_entry.r_version;
    debug.dso_count = dso_list.size();
    debug.brk = debug_entry.r_brk;
    debug.ldbase = debug_entry.r_ldbase;
    debug.dynamic = reinterpret_cast<uintptr_t>(dynamic);

    size = sizeof(debug);
    file.write(&size, sizeof(size));
    file.write(&debug, sizeof(debug));

    for (uint32_t i = 0; i<debug.dso_count; ++i) {
        file.write(&dso_list[i], sizeof(MDRawLinkMap));
    }

    vector<char> dso_data;
    // The passed-in size to the constructor (above) is only a hint.
    // Must call .resize() to do actual initialization of the elements.
    dso_data.resize(dynamic_length);
    ReadProcess(&dso_data[0], stream, dynamic, dynamic_length);
    file.write(&dynamic_length, sizeof(dynamic_length));
    file.write(&dso_data[0], dynamic_length);

    return debug.dso_count > 0;
}

///////////////////////////////////////////////////////////////////////////////

static void copy_variables(vector<Variable>& src,
        vector<TraceProcess::VariableEx>& dest)
{
    for (auto& v : src) {
        TraceProcess::VariableEx v2;
        v2.address = 0;
        v2.address_ptr = 0;
        v2.max_size = v.max_size;
        v2.is_pointer = v.is_pointer;
        v2.is_loaded = false;
        v2.is_pointer_loaded = false;
        v2.sym_name = v.sym_name;
        dest.push_back(v2);
    }
}

static void align_4bytes (char* buf, int& len)
{
    int unaligned = (len&3);
    if (unaligned) {
        memset(buf + len, 0, 4 - unaligned);
        len += 4-unaligned;
    }
}

void TraceProcess::process_magic_syscall(pid_t tid,
        int syscall_no, uintptr_t* syscall_args)
{
    int len = 0;
    char detail[EVENT_EXTRA_INFO_SIZE];

    switch (syscall_no) {
        case SYS_dump_x11: {
            detail[0] = 0;
            if (syscall_args[1] && syscall_args[2] < sizeof(detail)-4) {
                len = ReadProcess(&detail[0], &m_ctx_stream,
                        reinterpret_cast<const void*>(syscall_args[1]), syscall_args[2]);
                align_4bytes(detail, len);
            }

            dump_event(DUMP_REASON_x11 + (char)(syscall_args[0]), tid, detail, len);
            LOG(DEBUG) << "X11-" << syscall_args[0] << ":" << &detail[0];
        }
        break;

        case SYS_dump_dbus: {
            detail[0] = 0;
            if (syscall_args[1] && syscall_args[2] < sizeof(detail)-4) {
                len = ReadProcess(&detail[0], &m_ctx_stream,
                        reinterpret_cast<const void*>(syscall_args[1]), syscall_args[2]);
                align_4bytes(detail, len);
            }

            dump_event(DUMP_REASON_dbus + (char)(syscall_args[0]), tid, detail, len);
            LOG(DEBUG) << "DBUS-" << syscall_args[0] << ":" << &detail[0];
        }
        break;
        default:
        break;
    }
}

// @return: true, trace syscall-exit-stop;
//          false, skip syscall-exit-stop;
bool TraceProcess::process_syscall_enter(pid_t tid,
        int syscall_no, uintptr_t* syscall_args)
{
    if (SYS_restart_syscall == syscall_no) return true;
    LOG(DEBUG) << "(SYSCALL " << syscall_name(syscall_no)
               << ") enter for tracee " << tid;

    ++m_counter.syscall_enter;

    if (syscall_no >= SYS_init_buffers) {
        process_magic_syscall(tid, syscall_no, syscall_args);
        return true;
    }

    switch (syscall_no) {
        case SYS_fork:
#if defined(__x86_64__)
        case SYS_vfork:
#endif
        case SYS_clone:
        return prepare_clone(tid, syscall_no, syscall_args) >= 0;
        default:
        if (syscall_no < __NR_Linux) {
            if (89 == syscall_no) {
                // on 3.10.0 #1 SMP PREEMPT Fri Dec 14 15:20:30 CST 2018 00022-g5b64ed3 mips64
                // SYS_dump_x11 and SYS_dump_dbus will got 89
                // and syscall_args[3] always set to 1, indicate syscall failed.
                syscall_no = (1 == syscall_args[4]) ? SYS_dump_x11 : SYS_dump_dbus;
                process_magic_syscall(tid, syscall_no, syscall_args);
            }
            else {
                LOG(WARNING) << "(SYSCALL " << syscall_name(syscall_no)
                    << " < __NR_Linux";
            }
            return false;
        }
        else if (is_number_in_set(syscall_no, &g_trace_set[0])) {
            dump_event(syscall_no, tid, syscall_args, 0);
        }
        return true;
    }
}

bool TraceProcess::process_syscall_exit(pid_t tid,
        int syscall_no, uintptr_t* syscall_args, long syscall_result)
{
    bool dump = true;
    if (SYS_restart_syscall == syscall_no) return false;
    ++m_counter.syscall_exit;
    LOG(DEBUG) << "(SYSCALL " << syscall_name(syscall_no)
               << ") exit for tracee " << tid
               << ", result=" << syscall_result
               << ", continue=" << m_cont_type;

    if (syscall_no >= SYS_init_buffers) {
        // ignore magic syscall
        ptrace(PTRACE_POKEUSER, tid, sizeof(long)*SYSCALL_RESULT_INDEX, 0);
        return false;
    }

    switch (syscall_no) {
    case SYS_clone:
        // FIXME: It seems no need call PTRACE_SYSCALL after SYS_clone?
        /*
        if (syscall_result > 0) {
            my_ptrace("SYS_clone_succeed", PTRACE_SYSCALL, syscall_result, 0, 0);
        }*/
        break;
    case SYS_execve:
        // see PTRACE_EVENT_EXEC
        dump = false;
        break;
    case SYS_brk:
        dump_maps(syscall_no, 0);
        break;
    case SYS_mmap:
        if (syscall_result >= 0) {
            // NOTE: here we can't ignore anonymous map, or will cause call stack
            // back-trace not work in core file, because some thread stack or parameter
            // buffer in anonymous map;
            dump_maps(syscall_no, syscall_args[2]);
        }
        break;
    case SYS_mremap:
    // case SYS_munmap:
        if (syscall_result >= 0) {
            dump_maps(syscall_no, 0);
        }
        break;
    case SYS_mprotect:
        break;
    default:
        if (syscall_no < __NR_Linux) {
            dump = false;
        }
        break;
    }

    if (dump && is_number_in_set(syscall_no, &g_trace_set[0])) {
        dump_event(DUMP_REASON_syscall_exit+syscall_no,
                tid, &syscall_result, sizeof(syscall_result));
    }

    return dump;
}

int TraceProcess::prepare_clone(pid_t tid, int syscall_no, uintptr_t* syscall_args)
{
    uintptr_t flags;
    int ptrace_event;
    int termination_signal = SIGCHLD;

    if (syscall_no == SYS_clone) {
        flags = syscall_args[0];
        if (flags & CLONE_UNTRACED) {
            assert(0 && "should remove CLONE_UNTRACED");
        }
        termination_signal = flags & 0xff;
        if (flags & CLONE_VFORK) {
            ptrace_event = PTRACE_EVENT_VFORK;
        } else if (termination_signal == SIGCHLD) {
            ptrace_event = PTRACE_EVENT_FORK;
        } else {
            ptrace_event = PTRACE_EVENT_CLONE;
        }
#if defined(__x86_64__)
    } else if (syscall_no == SYS_vfork) {
        ptrace_event = PTRACE_EVENT_VFORK;
        flags = CLONE_VM | CLONE_VFORK | SIGCHLD;
#endif
    } else {
        ptrace_event = PTRACE_EVENT_FORK;
        flags = SIGCHLD;
    }

    if (is_number_in_set(syscall_no, &g_trace_set[0])) {
        dump_event(syscall_no, tid, syscall_args, 0);
    }

    if (!my_ptrace("SYS_clone_execute", PTRACE_SYSCALL, tid, 0, 0)) {
        ++m_counter.ptrace_error;
        return -1;
    }

    WaitStatus status;
    int raw_status = 0;
    for (;;) {
        pid_t ret = waitpid(tid, &raw_status,  __WALL);
        if (tid == ret) {
            status = WaitStatus(raw_status);

            if (WaitStatus::SYSCALL_STOP == status.type()) {
                // clone failed
                long syscall_result = ptrace(PTRACE_PEEKUSER,
                    tid, sizeof(long)*SYSCALL_RESULT_INDEX, NULL);
                process_syscall_exit(tid, syscall_no, syscall_args, syscall_result);
                LOG(ERROR) << "clone failed for tracee " << tid;
                return -2;
            }

            //NOTE: PTRACE_EVENT_SECCOMP will arrived before PTRACE_EVENT_CLONE
            // (PTRACE_EVENT_VFORK, PTRACE_EVENT_FORK) if enable seccom.
            if (PTRACE_EVENT_SECCOMP != status.ptrace_event()) {
                break;
            }
        } else if (tid < 0) {
            int wait_errno = errno;
            if ((wait_errno == ECHILD) && m_syscall_state.empty()) {
                break;
            }
            continue;
        }

        assert(m_syscall_state.find(ret) != m_syscall_state.end());
        if (!process_status(raw_status, ret))
            return -3;
    }

    if (WaitStatus::PTRACE_EVENT != status.type()) {
        LOG(ERROR) << "Not found PTRACE_EVENT for tracee " << tid
            << ", status=" << HEX(raw_status)
            << ", type=" <<status.type();
        return -4;
    }

    if (ptrace_event != status.ptrace_event()) {
        LOG(WARNING) << "Got " << ptrace_event_name(status.ptrace_event())
            << " for " << tid
            << " but require " << ptrace_event_name(ptrace_event);
        assert(0);
    }

    // Ideally we'd just use t->get_ptrace_eventmsg_pid() here, but
    // kernels failed to translate that value from other pid namespaces to
    // our pid namespace until June 2014:
    // https://github.com/torvalds/linux/commit/4e52365f279564cef0ddd41db5237f0471381093
    pid_t new_tid = 0;
    my_ptrace("query_cloned_tid", PTRACE_GETEVENTMSG, tid, nullptr, &new_tid);
    if (new_tid > 0) {
        if (flags & CLONE_THREAD) {
            if (m_syscall_state.find(new_tid) == m_syscall_state.end()) {
                add_thread(new_tid);
                LOG(INFO) << new_tid << " newborn thread after clone >>>";
            }
        } else {
            if (nullptr == m_parent->get_process(new_tid)) {
                shared_ptr<TraceProcess> child = make_shared<TraceProcess>(this);
                m_parent->add_process(child, new_tid);
                LOG(INFO) << new_tid << " newborn process after clone >>>";
            }
        }
    }

    return 0;
}

void TraceProcess::dump_debugger_count(void)
{
    LOG(INFO) << "\tNumber of ptrace error:" << m_counter.ptrace_error;
    LOG(INFO) << "\tNumber of pread error:" << m_ctx_stream.error_count();
    LOG(INFO) << "\tNumber of seccomp stop:" << m_counter.seccomp_stop;
    LOG(INFO) << "\tNumber of syscall-enter stop:" << m_counter.syscall_enter;
    LOG(INFO) << "\tNumber of syscall-exit stop:" << m_counter.syscall_exit;
    LOG(INFO) << "\tNumber of syscall-exit miss:" << m_counter.syscall_exit_miss;
    LOG(INFO) << "\tNumber of maps changed:" << m_counter.maps_change;
    if (m_syscall_dumper) {
        LOG(INFO) << "\tNumber of flush buffer:" << m_counter.flush_buffer;
        LOG(INFO) << "\tNumber of syscall hooked:" << m_syscall_dumper->syscall_count;
        LOG(INFO) << "\tNumber of events dumped:"
            << m_syscall_dumper->syscall_count + m_counter.event_dumped;
        if (m_counter.hook_dumped > 1024*1024) {
            LOG(INFO) << "\tSyscall hook bytes:"
                << m_counter.hook_dumped/(1024.0*1024.0) << "MB";
        }
        else {
            LOG(INFO) << "\tSyscall hook bytes:"
                << m_counter.hook_dumped/(1024.0) << "KB";
        }
    }
    else {
        LOG(INFO) << "\tNumber of events dumped:" <<  m_counter.event_dumped;
    }

    if (m_counter.total_dumped > 1024*1024) {
        LOG(INFO) << "\tTotal dump bytes:"
            << m_counter.total_dumped/(1024.0*1024.0) << "MB";
    }
    else {
        LOG(INFO) << "\tTotal dump bytes:"
            << m_counter.total_dumped/(1024.0) << "KB";
    }
}

void TraceProcess::reset_debugger_count(void)
{
    m_counter.syscall_enter = 0;
    m_counter.syscall_exit = 0;
    m_counter.syscall_exit_miss = 0;
    m_counter.seccomp_stop = 0;
    m_counter.flush_buffer = 0;
    m_counter.maps_change = 0;
    m_counter.ptrace_error = 0;
    m_counter.event_dumped = 0;
    m_counter.hook_dumped = 0;
    m_counter.total_dumped = 0;
}

TraceProcess::TraceProcess(DumpConfig* config)
{
    m_parent = this;
    m_can_dump = false;
    m_exec_stop = false;
    m_syscall_dumper = nullptr;
    m_breakpoint.address = 0;

    reset_debugger_count();

    m_cfg = config;
    copy_variables(m_cfg->vars, m_global_vars);

    m_page_size = sysconf(_SC_PAGESIZE);

    m_send_socket = 0;

    LOG(INFO) << "create root process " << this;
}

TraceProcess::TraceProcess(TraceProcess* parent)
{
    m_parent = parent;
    m_can_dump = false;
    m_exec_stop = false;
    m_syscall_dumper = nullptr;
    m_breakpoint.address = 0;

    reset_debugger_count();

    m_cfg = parent->get_config();
    copy_variables(m_cfg->vars, m_global_vars);

    m_page_size = sysconf(_SC_PAGESIZE);

    m_send_socket = -1;
    m_recv_socket = -1;
    m_server_socket = -1;
    m_recv_fd = -1;

    LOG(INFO) << "create child process " << this;
}

TraceProcess::~TraceProcess(void)
{
    if (m_recv_socket > 0) {
        close(m_recv_socket);
    }
    if (m_send_socket > 0) {
        close(m_send_socket);
    }

    LOG(INFO) << "destroy process " << this << ", for tracee " << m_pid;
}

void TraceProcess::get_share_name(uintptr_t name)
{
    char path[256];
    int len = snprintf(path, sizeof(path)-1,
            SHARED_FILE_NAME, m_pid, m_nonce);

    write_mem(m_pid, name, &path[0], len + 1);
    LOG(INFO) << "pass shared file:" << path;
}

int TraceProcess::get_shared_buffers(char** ptr)
{
    if (m_syscall_dumper) {
        *ptr = BUFFER_HEAD(m_syscall_dumper);
        return m_syscall_dumper->current;
    }

    return 0;
}

void TraceProcess::init_shared_buffers(int size)
{
    int fd = -1;
    int flags = MAP_SHARED;
    char shared_file[256];
    snprintf(shared_file, sizeof(shared_file) - 1,
            SHARED_FILE_NAME, m_pid, m_nonce);
    fd = open(shared_file, O_RDWR | O_CLOEXEC, 0600);
    if (fd <= 0) {
        LOG(FATAL) << "Failed to open shmem " << shared_file
            << ", errno=" << errno
            << ", for tracee " << m_pid;
        return;
    }
    if (ftruncate(fd, size)) {
        close(fd);
        LOG(FATAL) << "Failed to resize shmem to " << size
            << ", errno=" << errno
            << ", for tracee " << m_pid;
        return;
    }

    void* map_addr = mmap(NULL, size,
            PROT_READ | PROT_WRITE, flags, fd, 0);
    if (MAP_FAILED == map_addr) {
        close(fd);
        LOG(FATAL) << "Failed to mmap shmem region, errno=" << errno
            << ", for tracee " << m_pid;
        return;
    }

    m_syscall_dumper = reinterpret_cast<MemoryDumper *>(map_addr);
    m_syscall_dumper->size = size - sizeof(MemoryDumper);
    m_syscall_dumper->current = 0;
    m_syscall_dumper->page_size = m_page_size;
    m_syscall_dumper->max_stack_size = m_cfg->max_stack_size;
    m_syscall_dumper->stack_begin = 0;
    m_syscall_dumper->stack_end = 0;
    m_syscall_dumper->max_param_size = m_cfg->max_param_size;

    // pass syscall filter set to syscall preload
    m_syscall_dumper->syscall.nslots = g_trace_set[0].nslots;
    m_syscall_dumper->syscall.not_flag = g_trace_set[0].not_flag;
    char* data = BUFFER_HEAD(m_syscall_dumper);
    if (g_trace_set[0].nslots > 0) {
        int size = sizeof(number_slot_t) * g_trace_set[0].nslots;
        memcpy(data, g_trace_set[0].vec, size);
        data += size;
    }

    // pass syscall parameters to syscall preload
    *(int*)data = sizeof(syscall_param_flags);
    memcpy(data + sizeof(int), syscall_param_flags, sizeof(syscall_param_flags));
    close(fd);

    LOG(INFO) << "init_shared_buffers create dumper=" << m_syscall_dumper
        << " for tracee " << m_pid;
}

void TraceProcess::post_exec_syscall(pid_t tid)
{
    (void)tid;
    if (m_cfg->mode != DRY_RUN) {
        m_can_dump = true;
        link_exec_file(m_cfg->dump_dir,  m_pid);
        dump_maps(SYS_execve, PROT_EXEC|PROT_WRITE);
        if (!m_cfg->break_function.empty()) {
            //wait the specified function resolved and called!
            m_can_dump = false;
        }
    }
}

struct ps_prochandle
{
    pid_t pid;
    TraceProcess* process;
    TraceStream* stream;
    vector<VmSegment>* segs;
};

typedef enum tag_ps_err_e
{
    PS_OK,        /* Generic "call succeeded". */
    PS_ERR,        /* Generic error. */
    PS_BADPID,    /* Bad process handle. */
    PS_BADLID,    /* Bad LWP identifier. */
    PS_BADADDR,    /* Bad address. */
    PS_NOSYM,    /* Could not find given symbol. */
    PS_NOFREGS    /* FPU register set not available for given LWP. */
} ps_err_e;

extern "C" {
#include <thread_db.h>
// these ps_xxx  NEED by libthread_db.so.1
ps_err_e ps_pdread(struct ps_prochandle *ph,
        psaddr_t addr, void *buf, size_t size)
{
    if (ReadProcess(buf, ph->stream, addr, size) != (int)size) {
        LOG(ERROR) << "ps_pdread failed to read " << size
            << " bytes from " << (addr);
        return PS_ERR;
    }

    /* whatever td_ta_thr_iter() reads, dump to core */
    VmSegment vm;
    vm.start = reinterpret_cast<uintptr_t>(addr);
    vm.end = reinterpret_cast<uintptr_t>(addr) + size;
    ph->segs->push_back(vm);

    return PS_OK;
}

ps_err_e ps_pdwrite(struct ps_prochandle *ph,
        psaddr_t addr, const void *buf, size_t size)
{
    (void)ph;
    (void)addr;
    (void)buf;
    (void)size;
    /* NOP */
    return PS_OK;
}

ps_err_e ps_lgetregs(struct ps_prochandle *ph,
        lwpid_t lwpid, prgregset_t prgregset)
{
    (void)ph;
    (void)lwpid;
    (void)prgregset;
    /* NOP */
    return PS_OK;
}

ps_err_e ps_lsetregs(struct ps_prochandle *ph,
        lwpid_t lwpid, const prgregset_t prgregset)
{
    (void)ph;
    (void)lwpid;
    (void)prgregset;
    /* NOP */
    return PS_OK;
}

ps_err_e ps_lgetfpregs(struct ps_prochandle *ph,
        lwpid_t lwpid, prfpregset_t *prfpregset)
{
    (void)ph;
    (void)lwpid;
    (void)prfpregset;
    /* NOP */
    return PS_OK;
}

ps_err_e ps_lsetfpregs(struct ps_prochandle *ph,
        lwpid_t lwpid, const prfpregset_t *prfpregset)
{
    (void)ph;
    (void)lwpid;
    (void)prfpregset;
    /* NOP */
    return PS_OK;
}

pid_t ps_getpid(struct ps_prochandle *ph)
{
    return ph->pid;
}

ps_err_e ps_pglobal_lookup(struct ps_prochandle *ph,
        const char *object_name, const char *sym_name, psaddr_t *sym_addr)
{
    unsigned long addr;

    if (!ph->process->get_sym_address(object_name, sym_name, &addr)) {
        LOG(DEBUG) << __FUNCTION__ << " failed for:" << object_name << ", " << sym_name;
        return PS_NOSYM;
    }
    LOG(DEBUG) << __FUNCTION__ << " ok for:" << object_name << ", " << sym_name;

    *sym_addr = (psaddr_t)addr;

    return PS_OK;
}
} /* end extern "C" */

static int find_pthreads_cb(const td_thrhandle_t *th, void *cb_data)
{
    (void)cb_data;
    /* Get thread info, in order to access (and dump) data that
       gdb/libthread_db needs.  */
    td_thrinfo_t thinfo;
    td_thr_get_info(th, &thinfo);

    return TD_OK;
}

#if defined(__x86_64__)
#define PTHREAD_LIB "/usr/lib/x86_64-linux-gnu/libpthread-2.24.so"
#elif defined(__mips64)
#define PTHREAD_LIB "/lib/mips64el-linux-gnuabi64/libpthread-2.23.so"
#elif defined(__sw_64)
#define PTHREAD_LIB "/lib/libpthread-2.23.so"
#elif defined(__aarch64__)
#define PTHREAD_LIB "/lib/libpthread-2.23.so"
#else
#error need define PTHREAD_LIB
#endif

int TraceProcess::dump_thread_list(pid_t tid)
{
    struct ps_prochandle ph;
    td_thragent_t *ta;
    td_err_e err;

    (void)tid;

    if (m_symbols.find(PTHREAD_LIB) == m_symbols.end()) {
        return 0;
    }

    ph.pid = m_pid;
    ph.process = this;
    ph.stream = &m_ctx_stream;
    ph.segs = &m_syscall_memblks;

    err = td_ta_new(&ph, &ta);
    if (err == TD_OK) {
        err = td_ta_thr_iter(ta, find_pthreads_cb, NULL,
                     TD_THR_ANY_STATE, TD_THR_LOWEST_PRIORITY,
                     TD_SIGNO_MASK, TD_THR_ANY_USER_FLAGS);

        td_ta_delete(ta);

        LOG(DEBUG) << "dump_thread_list count:" << ph.segs->size();
    }

    if (err == TD_NOLIBTHREAD) {
        LOG(DEBUG) << "target does not appear to be multi-threaded for tracee " << m_pid;
    } else if (err != TD_OK) {
        LOG(WARNING) << "FIXME: libthread_db not found, using fallback: " << err
            << " for tracee " << m_pid;
        // TODO: fallback to call get_pthread_list_fallback(di);
    }

    return 0;
}

int TraceProcess::dump_thread_status(pid_t tid)
{
    char maps_path[NAME_MAX];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/status", tid);

    const int fd = open(maps_path, O_RDONLY, 0);
    if (fd < 0)
        return false;

    unique_ptr<LineReader> line_reader = make_unique<LineReader>(fd);

    const char* line = nullptr;
    unsigned int line_len = 0;
    int i = 0;
    while ((i < 8) && line_reader->GetNextLine(&line, &line_len)) {
        LOG(DEBUG) << "\t" << line;
        ++i;
        line_reader->PopLine(line_len);
    }

    close(fd);

    return true;
}

int TraceProcess::dump_thread_context(ThreadContext* ctx)
{
#if 1
    (void)ctx;
#else
    struct user_desc desc;

    ctx->tls.clear();

    errno = 0;

#if defined(__x86_64__)
#define GDT_ENTRY_TLS_MIN        12
#define GDT_ENTRY_TLS_MAX        14
#else
#error need to define new GDT_ENTRY_TLS_MAX, GDT_ENTRY_TLS_MIN
#endif

    // TODO: read tls by ptrace does not work, the follow code just read user_desc.
    // We should dump the .tbss section.
    // [19] .tbss             NOBITS          0000000000201dc8 001dc8 000004 00 WAT  0   0  4
    for (int i = GDT_ENTRY_TLS_MIN; i <= GDT_ENTRY_TLS_MAX; ++i) {
        long ret = ptrace(
                static_cast<__ptrace_request>(PTRACE_GET_THREAD_AREA),
                tid, i, &desc);
        if (ret < 0) {
            LOG(DEBUG) << "Failed to get tls for tracee " << tid
                << ", nth=" << i
                << ", errno=" << errno;
            break;
        }
        ctx->tls.push_back(desc);
    }

    ctx->tls.push_back(desc);
#endif

    return 0;
}

/* parse the 28th field of /proc/#tid/stat or use PTRACE_PEEKUSER.
 * then, search current stack point in /proc/#pid/maps to locate stack segment
 */
int TraceProcess::dump_thread_stack(ThreadContext* ctx)
{
    VmSegment vm;
    uintptr_t stack_pointer;

#if defined(__i386__)
        stack_pointer = ctx->regs.esp;
#elif defined(__x86_64__)
        stack_pointer = ctx->regs.rsp;
#elif defined(__mips64) || defined(__sw_64) || defined(__aarch64__)
        stack_pointer = ctx->regs.sp;
#else
#error need to implement new method to read stack pointer
#endif

    vm.start = stack_pointer & (~(m_page_size-1));
    vm.end = vm.start + m_cfg->max_stack_size;

    // NOTE: some child thread's stack is in an anonymous region not in [stack] region,
    // need search stack_pointer in maps file to detect the real stack range
    for (auto& m:m_mappings) {
        MappingInfo* mp = m.get();
        uintptr_t end = mp->start_addr + mp->size;
        if (vm.start >= mp->start_addr &&
            vm.start < end) {
            if (vm.end > end) vm.end = end;
            break;
        }
    }

    ctx->stack = vm;

    return 0;
}

void TraceProcess::check_global_pointer_var_is_assigned(void)
{
    VmSegment seg;
    vector<VmSegment> new_rw_segs;

    for (auto& var: m_global_vars) {
        if (!var.address ||
            !var.is_pointer ||
            var.is_pointer_loaded) {
            continue;
        }

        if (sizeof(uintptr_t) == ReadProcess(&var.address_ptr,
                &m_ctx_stream,
                reinterpret_cast<const void*>(var.address),
                sizeof(uintptr_t)) && (var.address_ptr > 0)) {
            const uintptr_t mask = ~3;
            seg.start = var.address_ptr & mask;
            seg.end = (var.address_ptr + var.max_size + 4) & mask;
            assert(seg.start > 0xffff);
            new_rw_segs.push_back(seg);
            var.is_pointer_loaded = true;
        }
        else {
            var.address_ptr = 0;
        }
    }

    if (!new_rw_segs.empty()) {
        merge_heap(new_rw_segs);
    }
}

void TraceProcess::merge_heap(vector<VmSegment>& segs)
{
    VmSegment seg;

    segs.insert(segs.end(), m_heaps.begin(), m_heaps.end());
    sort(segs.begin(), segs.end(),
            [](auto& a, auto& b){return a.start < b.start;});

    seg.start = 0;
    seg.end = 0;

    m_heaps.clear();

    for (auto& heap: segs) {
        assert(heap.start > 0xffff);

        if (0 == seg.start) {
            // first
            seg.start = heap.start;
            seg.end = heap.end;
            continue;
        }

        if (seg.start <= heap.start && heap.start <= seg.end) {
            // overlap
            if (seg.end < heap.end) seg.end = heap.end;
        }
        else {
            // new range
            m_heaps.push_back(seg);
            seg.start = heap.start;
            seg.end = heap.end;
        }
    }

    m_heaps.push_back(seg);

    return;
}

/* maps.bin layout:
 * time(struct timespec), [data-size, data], ...
 * */
int TraceProcess::dump_maps(int reason, int map_prot)
{
    if (m_counter.total_dumped >= m_cfg->max_dump_bytes) {
        return 0;
    }

    if (m_auxv.empty()) {
        // NOTE: here we only dump auxv once!
        dump_auxv(m_pid);
    }

    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    m_maps_file.write(&current, sizeof(current));
    dump_proc_file(m_pid, "maps");

    // dump module list, especially linux-gate.so,
    VmSegment seg;
    VmSegment heap = {0,0};
    VmSegment stack = {0,0};
    vector<VmSegment> new_rw_segs;
    m_mappings.clear();
    EnumerateMappings(m_mappings, m_auxv, m_pid, &stack, &heap);
    if (m_syscall_dumper) {
        m_syscall_dumper->stack_begin = stack.start;
        m_syscall_dumper->stack_end = stack.end;
    }

    // NOE: [heap] created and adjust after every brk syscall;
    // And the first part of heap maybe used by libc,libpthread,...
    // so we dump around the center point.
    if (m_cfg->max_heap_size && heap.start > 0 && heap.end > heap.start) {
        uintptr_t mid = heap.start + (heap.end - heap.start)/2;
        seg.start = mid - m_cfg->max_heap_size/2;
        seg.end = mid + m_cfg->max_heap_size/2;
        if (seg.start < heap.start) seg.start = heap.start;
        if (seg.end > heap.end) seg.end = heap.end;
        new_rw_segs.push_back(seg);
    }

    WriteMappings(m_maps_file, m_mappings, m_symbols, &m_ctx_stream);

    // dump DSO Debug, should have PT_DYNAMIC, see readelf -d xxx-elf
    bool dso_found = WriteDSODebugStream(m_maps_file, m_auxv, &m_ctx_stream);
    LOG(DEBUG) << "dump_maps for tracee " << m_pid
        << ", dso_found=" << dso_found << ", reason=" << reason;

    if (map_prot & PROT_EXEC) {
        // search unresolved symbol
        const char* break_function = NULL;
        if ((0 == m_breakpoint.address) && !m_cfg->break_function.empty()) {
            break_function = m_cfg->break_function.data();
        }
        for (auto& kv : m_symbols) {
            SymbolFile* helper = kv.second.get();
            if (!helper->m_valid || helper->m_searched) continue;
            helper->m_searched = true;

            // search global variables address
            for (auto& var: m_global_vars) {
                if (!var.address &&
                    helper->get_sym_address(var.sym_name.data(),
                        &var.address, elf::stt::object)) {
                    if (var.is_pointer) {
                        if (sizeof(uintptr_t) != ReadProcess(&var.address_ptr,
                                &m_ctx_stream,
                                reinterpret_cast<const void*>(var.address),
                                sizeof(uintptr_t))) {
                            var.address_ptr = 0;
                        }
                    }
                    LOG(DEBUG) << "Resolved variable:" <<  var.sym_name.data()
                       << " in module " << kv.first.data()
                      << ", at " << HEX(var.address);
                }
            }

            // search break function address
            if (break_function && helper->get_sym_address(break_function,
                        &m_breakpoint.address, elf::stt::func)) {
                break_at_function(m_pid);
            }
        }
    }

    // check if there are some new global variables loaded
    const uintptr_t mask = ~3;
    for (auto& var: m_global_vars) {
        if (!var.address || var.is_loaded) {
            continue;
        }

        // TODO: remove a global variable if its module has been unloaded!
        // NOTE: we dump at least one memory page!
        seg.start = var.address & mask;
        seg.end = (var.address + var.max_size + 4) & mask;
        assert(seg.start > 0xffff);
        new_rw_segs.push_back(seg);
        var.is_loaded = true;

        if (var.is_pointer) {
            if (var.address_ptr > 0) {
                seg.start = var.address_ptr & mask;
                seg.end = (var.address_ptr + var.max_size + 4) & mask;
                assert(seg.start > 0xffff);
                new_rw_segs.push_back(seg);
                var.is_pointer_loaded = true;
            }
            else {
                continue; // the pointer is not assigned !
            }
        }
    }

    // check if new writable segments
    if (map_prot & PROT_WRITE) {
        std::list<string>::iterator n = m_cfg->modules.begin();
        while (n != m_cfg->modules.end()) {
            bool found = false;
            for (auto& m: m_mappings) {
                MappingInfo* mp = m.get();
                if (!mp->name[0] || !mp->offset) {
                    continue;
                }

                if (fnmatch(n->data(), mp->name, FNM_PATHNAME)) {
                    continue;
                }

                auto it = g_guid_maps.find(mp->name);
                if (it == g_guid_maps.end()) {
                    found = true;
                    break;
                }

                /*NOTE: libc is strange
7ffff7813000-7ffff79a8000 r-xp 00000000 08:02 1447866 /usr/lib/x86_64-linux-gnu/libc-2.24.so
7ffff79a8000-7ffff7ba7000 ---p 00195000 08:02 1447866 /usr/lib/x86_64-linux-gnu/libc-2.24.so
7ffff7ba7000-7ffff7bad000 rw-p 00194000 08:02 1447866 /usr/lib/x86_64-linux-gnu/libc-2.24.so
*/
                if (abs(mp->offset - it->second.rw_offset) > m_page_size) {
                    // the segment's offset should page size align
                    continue; // try next segment
                }
                //NOTE: here we just need dump .data and .bss
                for (int i = 0; i < 4; i += 2) {
                    seg.start = it->second.addrs[i] - mp->offset + mp->start_addr;
                    seg.end = it->second.addrs[i+1]- mp->offset + mp->start_addr;
                    if (!std::binary_search(m_heaps.begin(), m_heaps.end(), seg,
                            [](const VmSegment& a, const VmSegment& b){
                            return (a.start < b.start) || (a.end < b.end);})) {
                        new_rw_segs.push_back(seg);
                    }
                }

                found = true;
                break; // OK
            }
            if (!found)
                ++n;
            else
                n = m_cfg->modules.erase(n);
        }
    }

    if (!new_rw_segs.empty()) {
        merge_heap(new_rw_segs);
    }
    ++m_counter.maps_change;

    return 0;
}

/*status, stat, maps, smaps, cmdline, environ, ... etc*/
int TraceProcess::dump_proc_file(pid_t pid, const char* name)
{
    char path[NAME_MAX];
    snprintf(path, sizeof(path), "/proc/%d/%s", pid, name);

    MemoryMappedFile mapped_file(path, 0);
    int size = mapped_file.size();
    assert(size > 0);
    m_maps_file.write(&size, sizeof(size));
    m_maps_file.write(mapped_file.data(), size);
    LOG(DEBUG) << "dump " << path << "," << size;

    return 0;
}

int TraceProcess::dump_auxv(pid_t pid)
{
    uint32_t size = 0;
    char path[NAME_MAX];

#define AT_MAX AT_SYSINFO_EHDR

    struct timespec current;
    clock_gettime(CLOCK_REALTIME, &current);
    m_maps_file.write(&current, sizeof(current));

    m_auxv.resize(AT_MAX+1);
    for (auto& i : m_auxv) {
        i = 0;
    }
    snprintf(path, sizeof(path), "/proc/%d/auxv", pid);
    MemoryMappedFile mapped_file(path, 0);
    size = mapped_file.size();
    assert(size > 0);
    m_maps_file.write(&size, sizeof(size));
    m_maps_file.write(mapped_file.data(), size);
    LOG(DEBUG) << "dump auxv " << size;

    dump_proc_file(pid, "cmdline");
    dump_proc_file(pid, "environ");

    const elf_aux_entry* aux_entry = reinterpret_cast<const elf_aux_entry*>(mapped_file.data());
    for (uint32_t i=0; i < size/sizeof(elf_aux_entry); ++i) {
        if (aux_entry->a_type != AT_NULL) {
            if (aux_entry->a_type <= AT_MAX) {
                m_auxv[aux_entry->a_type] = aux_entry->a_un.a_val;
            }
        }
        ++aux_entry;
    }

    // dump vdso
    const void* linux_gate_loc = reinterpret_cast<const void*>(
            m_auxv[AT_SYSINFO_EHDR]);
    if (linux_gate_loc) {
        // FIXME: how to get size of vdso
#if defined(__mips__)
        size = m_page_size;
#else
        size = m_page_size * 2;
#endif
        unique_ptr<char[]> buf = make_unique<char[]>(size);
        int len = ReadProcess(buf.get(), &m_ctx_stream, linux_gate_loc, size);
        size += sizeof(uintptr_t);
        m_maps_file.write(&size, sizeof(size));
        m_maps_file.write(buf.get(), size - sizeof(uintptr_t));
        m_maps_file.write(&linux_gate_loc, sizeof(linux_gate_loc));
        LOG(DEBUG) << "dump vdso " << linux_gate_loc << ", " << len;
    }
    else {
        int size = 0;
        m_maps_file.write(&size, sizeof(size));
        // NOTE: vdso is not implemented in SunWay!
#if !defined(__sw_64)
        LOG(WARNING) << "failed to dump vdso " << linux_gate_loc;
#endif
    }

    return 0;
}

static bool is_in_stack(VmSegment* seg, vector<ThreadContext>& ctx, int count)
{
    for (int i = 0; i < count; ++i) {
        if (seg->start >= ctx[i].stack.start &&
            seg->end <= ctx[i].stack.end) {
            return true;
        }
    }

    return false;
}

int TraceProcess::dump_clone(pid_t cur_tid, pid_t newtid) //only used in fast mode
{
    EventHead head;
    uint32_t heap_num = 0;
    long syscall_result = newtid;
    ThreadContext context, *ctx = &context;
#if defined(__aarch64__)
	static struct iovec io;
#endif

    if (!m_can_dump) {
        return 0;
    }

    clock_gettime(CLOCK_REALTIME, &head.cur_time);
    head.reason = SYS_clone;
    head.thread_num = 1;
    head.current_tid = cur_tid;
    head.extra_size = sizeof(syscall_result);

    ctx->tid = cur_tid;
#if defined(__aarch64__)
	io.iov_base = &ctx->regs;
	io.iov_len = sizeof(ctx->regs);
    ptrace(PTRACE_GETREGSET, ctx->tid, NT_PRSTATUS, &io);

	io.iov_base = &ctx->fpregs;
	io.iov_len = sizeof(ctx->fpregs);
    ptrace(PTRACE_GETREGSET, ctx->tid, NT_FPREGSET, &io);
#else
    ptrace(PTRACE_GETREGS, ctx->tid, nullptr, &ctx->regs);
    ptrace(PTRACE_GETFPREGS, ctx->tid, nullptr, &ctx->fpregs);
#endif
    dump_thread_context(ctx);
    dump_thread_stack(ctx);
    m_ctx_stream2.write(&head, &syscall_result);
    m_ctx_stream2.write(ctx);
    m_ctx_stream2.write(&heap_num, sizeof(heap_num));

    return 0;
}

int TraceProcess::dump_event(int reason, pid_t cur_tid, void* extra_data, int extra_data_size)
{
    int ret = 0;
    uint8_t flags = 0;
    uint8_t args = 0;
    bool is_sig = false;
    uint32_t heap_num = 0;
    EventHead head;
    long begin = 0;
    uint32_t syscall_no = 0;

#if defined(__aarch64__)
	static struct iovec io;
#endif

    if (!m_can_dump || m_counter.total_dumped >= m_cfg->max_dump_bytes) {
        return 0;
    }

    check_global_pointer_var_is_assigned();

    clock_gettime(CLOCK_REALTIME, &head.cur_time);
    head.reason = reason;
    head.thread_num = 0;
    head.current_tid = cur_tid;
    head.extra_size = extra_data_size;

    begin = m_counter.total_dumped;
    if (reason < __NR_Linux) {
        m_counter.total_dumped += m_ctx_stream.write(&head, extra_data);

        //TODO: how to dump errno in target tracee with pread?
        goto dump_end;
    }
    ++m_counter.event_dumped;

    if (m_thread_ctx.size() < m_syscall_state.size()) {
        m_thread_ctx.resize(m_syscall_state.size());
    }

    is_sig = (reason >= DUMP_REASON_signal && reason < DUMP_REASON_dbus);
    for (auto& kv : m_syscall_state) {
        if (!is_sig) {
            if (m_cfg->current_thread_only) {
                if (kv.first != cur_tid)
                    continue;
            }
            // FIXME: skip sleeping thread which has dumped is reasonable?
            // The Stack Data is redundancy if not do that.
            else if (kv.second.state & SYSCALL_ENTER_MASK) {
                continue;
            }
        }

        ThreadContext* ctx = &m_thread_ctx[head.thread_num];
        ctx->tid = kv.first;
        ctx->interrupted = false;

#if defined(__aarch64__)
		io.iov_base = &ctx->regs;
		io.iov_len = sizeof(ctx->regs);
        ret = ptrace(PTRACE_GETREGSET, ctx->tid, NT_PRSTATUS, &io);
#else
        ret = ptrace(PTRACE_GETREGS, ctx->tid, nullptr, &ctx->regs);
#endif
        if (ret < 0) {
            if (errno == ESRCH) {
                // current thread is not at trace-STOP (running or sleeping)
                /*
                 * If the tracee is running or sleeping in kernel space and PTRACE_SYSCALL
                 * is in effect, the system call is interrupted and syscall-exit-stop is reported.
                 * (The interrupted system call is restarted when the tracee is restarted.)
                 *
                 * 根据以上 PTRACE_INTERRUPT 的说明, 此时如果使用 PTRACE_INTERRUPT 强行中断在
                 * running 或 sleeping 的线程:
                 * 1) 处于 sleeping 的线程会返回一个失败的系统调用，干扰分析，不应该记录这种线程.
                 * 2) 处于 running 的线程的执行过程不是立即被打断，而是有一个延时, 在其进入系统调用
                 *    或时间片结束的时候才会停下来,执行性能严重降低，会干扰多线程程序的重现条件.
                 * 如果问题出在 running 的线程上，运行一段时间后一般会触发一个崩溃事件或死锁事件;
                 * 从解决问题角度和记录性能上考虑，此时强行中断 running 的线程其实意义不大,除非要
                 * 防止 dump 全局变量和堆内存时候的数据访问竞争.
                 *
                 * 另外一个方法是利用 cpu affinity set API 限定目标进程在一个单核上运行.
                 */
                continue; //国产平台 PTRACE_INTERRUPT 性能低.
            }
            else {
                ++m_counter.ptrace_error;
                LOG(DEBUG) << "PTRACE_GETREGS failed for tracee " << ctx->tid
                    << ", ret=" << ret << ", errno=" << errno;
                continue;
            }
        }

        ++head.thread_num;
#if defined(__aarch64__)
		io.iov_base = &ctx->fpregs;
		io.iov_len = sizeof(ctx->fpregs);
        ptrace(PTRACE_GETREGSET, ctx->tid, NT_FPREGSET, &io);
#else
        ptrace(PTRACE_GETFPREGS, ctx->tid, nullptr, &ctx->fpregs);
#endif
        dump_thread_context(ctx);
        dump_thread_stack(ctx);
    }
    assert(head.thread_num > 0);

    m_counter.total_dumped += m_ctx_stream.write(&head, extra_data);

    for (int i = 0; i < head.thread_num; ++i) {
        m_counter.total_dumped += m_ctx_stream.write(&m_thread_ctx[i]);
    }

    if (reason >= DUMP_REASON_signal) {
        goto dump_heap;
    }

    // Auto dump if current syscall has pass memory pointer?
    syscall_no = reason - __NR_Linux;
    flags = syscall_param_flags[2*syscall_no];
    if ((flags != 0) && (m_cfg->max_param_size > 0)) {
        VmSegment seg;
        const uintptr_t mask = ~3;
        uintptr_t* syscall_args = (uintptr_t*)extra_data;

        // parameter is void*, size indicate in next parameter
        bool size_indicate_by_next = flags & 0x80;

        args = syscall_param_flags[2*syscall_no + 1];
        assert(m_syscall_memblks.empty());
        for (unsigned char i = 0; i < args; ++i) {
            if (flags & 1) {
                uintptr_t addr = syscall_args[i];
                if ((addr > 0) ) {
                    seg.start = addr & mask;
                    if (size_indicate_by_next) {
                        if (syscall_args[i+1] > 0) {
                            seg.end = (addr + syscall_args[i+1] + 3) & mask;
                            if (!is_in_stack(&seg, m_thread_ctx, head.thread_num))
                                m_syscall_memblks.push_back(seg);
                        }
                        break;
                    }
                    seg.end = (addr + m_cfg->max_param_size + 3) & mask;
                    if (!is_in_stack(&seg, m_thread_ctx, head.thread_num))
                        m_syscall_memblks.push_back(seg);
                }
            }
            flags >>= 1;
        }

        if (!m_syscall_memblks.empty()) {
            // memory pointer parameter only dump once,
            // so store heaps first, then restore.
            m_heaps_temp = m_heaps;
            merge_heap(m_syscall_memblks);
        }
    }

dump_heap:
    heap_num = m_heaps.size();
    // heap_count(4 byte)[, sizeof(long)+sizeof(int)+data, ...]
    m_counter.total_dumped += m_ctx_stream.write(&heap_num, sizeof(heap_num));
    for (auto& heap: m_heaps) {
        m_counter.total_dumped += m_ctx_stream.write(&heap);
    }

    if (!m_syscall_memblks.empty()) {
        m_syscall_memblks.clear();
        m_heaps_temp.swap(m_heaps);
    }

    // resume all interrupted threads
    for (int i = 0; i < head.thread_num; ++i) {
        if (m_thread_ctx[i].interrupted) {
            my_ptrace("dump_event", PTRACE_SYSCALL, m_thread_ctx[i].tid, 0, 0);
        }
    }

dump_end:
    LOG(DEBUG) << "dump_event reason:" << reason
        << " tid " << cur_tid
        << " threads " << head.thread_num << "/" << m_syscall_state.size()
        << " at [" << begin
        << ", " << m_counter.total_dumped << "]";

    return 0;
}

int TraceProcess::flush_shared_buffers(void)
{
    char* buf = nullptr;
    int size = get_shared_buffers(&buf);
    if (size > 0 ) {
        LOG(DEBUG) << "flush_shared_buffers offset=" << m_counter.total_dumped
            << ", size=" << size << " for tracee " << m_pid;
        m_syscall_dumper->current = 0;

        ++m_counter.flush_buffer;
        m_counter.hook_dumped += size;
        if (m_counter.total_dumped < m_cfg->max_dump_bytes) {
            m_counter.total_dumped += size;
            m_ctx_stream.write(buf, size);
        }
    }

    return size;
}

void TraceProcess::stop_record(void)
{
    m_can_dump = false;
    for (auto i = m_childs.begin(); i != m_childs.end(); ++i) {
        i->get()->stop_record();
    }

    m_end_time = time(NULL);
    LOG(INFO) << "close process " << this
        << " duration=" << (m_end_time - m_begin_time)
        <<"s for tracee " << m_pid;

    flush_shared_buffers();
    m_ctx_file.close();

    if (m_cfg->mode == FAST) {
        m_ctx_file2.close();
        // NOTE: the handle_connect thread maybe in dump_maps,
        // so simple sleep 1s to wait dump_maps finish.
        sleep(1);
    }

    m_maps_file.close();

    dump_debugger_count();
}

static void* handle_connect(void* param)
{
    reinterpret_cast<TraceProcess*>(param)->handle_connect();
    return nullptr;
}

int TraceProcess::handle_connect(void)
{
    char buf[256];

    while (m_maps_file.valid()) {
        int fd = accept(m_server_socket, NULL, NULL);
        if (fd < 0) {
            LOG(FATAL) << "failed to accept:" <<  strerror(errno)
                << ", for tracee " << m_pid;
            break;
        }

        // blocking recv
        int nbytes = read(fd, buf, sizeof(int) + sizeof(uintptr_t));
        if (-1 == nbytes) {
            LOG(FATAL) << "failed to read:" <<  strerror(errno)
                << ", for tracee " << m_pid;
            break;
        }

        // TODO: add sync protect to avoid conflict to the waitpid thread
        LOG(DEBUG) << "recvmsg:" <<  *(int*)&buf[0]
                << "," << HEX(*(uintptr_t*)&buf[4])
                << ", for tracee " << m_pid;

        switch (*(int*)&buf[0]) {
        case SYS_init_buffers:
        init_shared_buffers(*(int*)&buf[4]);
        break;

        case SYS_flush_buffers:
        // NOTE: SYS_flush_buffers maybe receive before SYS_init_buffers
        // for child create by fork before execve,e.g. firefox-esr 52.0
        flush_shared_buffers();
        break;

        case SYS_share_name:
        *(int*)&buf[0] = m_nonce;
        break;

        case SYS_enable_dump: {
            // Here we can dump dso link map now!
            m_can_dump = true;
            dump_maps(SYS_enable_dump, PROT_EXEC|PROT_WRITE);
            if (!m_cfg->break_function.empty()) {
                //wait the specified function resolved and called!
                m_can_dump = false;
            }
        }
        break;

        case SYS_update_maps:
        dump_maps(SYS_update_maps, *(int*)&buf[4]);
        break;

        default:
        break;
        }

        nbytes = write(fd, buf, sizeof(int));
        if (-1 == nbytes) {
            LOG(FATAL) << "failed to write:" <<  strerror(errno)
                << ", for tracee " << m_pid;
            break;
        }
        close(fd);
    }

    close(m_server_socket);
    m_server_socket = -1;

    snprintf(buf, sizeof(buf) - 1, SHARED_SOCKET_NAME, m_pid);
    unlink(buf);

    LOG(INFO) << "disconnect socket:" << buf  << ", for tracee " << m_pid;

    return 0;
}

int TraceProcess::setup_listener(void)
{
    struct sockaddr_un addr;
    int sc, rc = -1;

    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        LOG(ERROR) << "failed to create socket:" << strerror(errno)
            << ", for tracee " << m_pid;
        goto done;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path) - 1, SHARED_SOCKET_NAME, m_pid);

    sc = bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (sc == -1) {
        LOG(ERROR) << "failed to bind:" << strerror(errno)
            << ", for tracee " << m_pid;
        goto done;
    }

    sc = listen(sock_fd, 5);
    if (sc == -1) {
        LOG(ERROR) << "failed to listen:" << strerror(errno)
            << ", for tracee " << m_pid;
        goto done;
    }
    LOG(INFO) << "listen " << addr.sun_path << ", for tracee " << m_pid;

    rc = 0;

    m_server_socket = sock_fd;
    pthread_t thread;
    pthread_create(&thread, nullptr, ::handle_connect, this);

done:
    return rc;
}

int TraceProcess::setup_socket(int* pfd_number)
{
    int sockets[2];
    long ret = socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets);
    if (ret < 0) {
        LOG(FATAL) << "socketpair failed";
        if (pfd_number) *pfd_number = 0;

        return -1;
    }
    m_recv_socket = sockets[0];

    // Find a usable FD number to dup to in the child. RESERVED_SOCKET_FD
    // might already be used by an outer rr.
    int fd_number = RESERVED_SOCKET_FD;
    // We assume no other thread is mucking with this part of the fd address space.
    while (true) {
        ret = fcntl(fd_number, F_GETFD);
        if (ret < 0) {
            if (errno != EBADF) {
                LOG(FATAL) << "Error checking fd";
            }
            break;
        }
        ++fd_number;
    }
    if (pfd_number) *pfd_number = fd_number;

    LOG(INFO) << "socketpair " << sockets[0]
        << ", " << sockets[1] << ", " << fd_number;

    return sockets[1];
}

static void mark_crash(const char* dir, int pid, int sig)
{
    char path[256];
    int len = snprintf(path, sizeof(path), "%s/crash.txt", dir);
    FILE* pf = fopen(path, "wb");
    if (pf) {
        fprintf(pf, "%d,%d\n", pid, sig);
        fclose(pf);
    }
    else {
        path[len-1] = 0;
        LOG(ERROR) << "Failed to mark crash:" << len << "," << path;
    }
}

bool TraceProcess::process_signal(pid_t tid)
{
    bool exit_record = true;

    switch (m_sig) {
        case SIGHUP:
        case SIGINT:
        case SIGKILL:
        case SIGPIPE:
        case SIGALRM:
        case SIGTERM:
        case SIGPOLL:
        case SIGPROF:
        {
            // term signal
            // increase max dump bytes first, or dump_maps or dump_event will ignore
            m_cfg->max_dump_bytes += 4*1024*1024*1024LL;
            if (!m_can_dump) {
                m_can_dump = true;
                link_exec_file(m_cfg->dump_dir,  m_pid);
                dump_maps(-m_sig, PROT_EXEC|PROT_WRITE);
            }
            m_cfg->current_thread_only = false;
            dump_event(DUMP_REASON_signal+m_sig, tid, nullptr, 0);
        }
        break;

        case SIGQUIT:
        case SIGILL:
        case SIGABRT:
        case SIGFPE:
        case SIGSEGV:
        case SIGBUS:
        {
            // core signal
            // increase max dump bytes first, or dump_maps or dump_event will ignore
            m_cfg->max_dump_bytes += 4*1024*1024*1024LL;
            if (!m_can_dump) {
                m_can_dump = true;
                link_exec_file(m_cfg->dump_dir,  m_pid);
                dump_maps(-m_sig, PROT_EXEC|PROT_WRITE);
            }
            m_cfg->current_thread_only = false;
            dump_event(DUMP_REASON_signal+m_sig, tid, nullptr, 0);
        }
        break;

        case SIGCHLD:
        case SIGCONT:
        case SIGSTOP:
        case SIGTSTP:
        case SIGTTIN:
        case SIGTTOU:
        case SIGSYS:
        case SIGTRAP:
        if (std::find(m_cfg->sigs.begin(), m_cfg->sigs.end(), m_sig) !=
            m_cfg->sigs.end()) {
            if (!m_can_dump) {
                m_can_dump = true;
                link_exec_file(m_cfg->dump_dir,  m_pid);
                dump_maps(-m_sig, PROT_EXEC|PROT_WRITE);
            }
            dump_event(DUMP_REASON_signal+m_sig, tid, nullptr, 0);
        }
        else {
            m_sig = 0; // ignore
            exit_record = false;
        }
        break;

        case SIGVTALRM: //firefox-esr 52.0 got SIGVTALRM
        exit_record = false;
        break;

        default:
        // other signal, forward it to the kernel!
        exit_record = false;
        break;
    }

    if (exit_record) {
#ifdef _DEBUG
        gdb_bt(tid);
#endif

        mark_crash(m_cfg->dump_dir.data(), m_pid, m_sig);
    }

    return exit_record;
}

bool TraceProcess::process_status(int raw_status, pid_t tid)
{
    USER_REGS regs;
    WaitStatus status = WaitStatus(raw_status);

    switch (status.type()) {
        case WaitStatus::EXIT:
        {
            remove_thread(tid);
            LOG(INFO) << "(EXIT-" << status.exit_code() << ") for tracee " << tid;

            goto has_restart;
        }
        break;
        case WaitStatus::FATAL_SIGNAL:
        {
            LOG(INFO) << "(FATAL-" << signal_name(status.fatal_sig())
                      << ") for tracee " << tid;
            siginfo_t pending_siginfo;
            ptrace(PTRACE_GETSIGINFO, tid, 0, &pending_siginfo);
            m_sig = pending_siginfo.si_signo;

            dump_event(DUMP_REASON_signal+m_sig, tid, nullptr, 0);
            goto fatal_error;
        }
        break;
        case WaitStatus::SIGNAL_STOP:
        {
            siginfo_t pending_siginfo;
            LOG(INFO) << "(SIGNAL-STOP-" << signal_name(status.stop_sig())
                       << ") for tracee " << tid;
            if (status.stop_sig() == SIGTRAP && remove_break_function(tid)) {
                goto has_restart;
            }

            ptrace(PTRACE_GETSIGINFO, tid, 0, &pending_siginfo);
            m_sig = pending_siginfo.si_signo;
            if (process_signal(tid)) {
                goto fatal_error;
            }
        }
        break;
        case WaitStatus::GROUP_STOP:
        {
            int stop = status.group_stop();
            LOG(DEBUG) << "(GROUP-STOP-" << signal_name(stop) << ") for tracee " << tid;

            /* If the PTRACE_O_TRACEEXEC option is not in effect, all successful
               calls to execve(2) by the traced process will cause it to be sent a
               SIGTRAP signal, giving the parent a chance to gain control before the
               new program begins execution.
               */
            if (stop == SIGTRAP) {
                if (!my_ptrace("skip_SIGTRAP", PTRACE_SYSCALL, tid, 0, 0))
                    goto fatal_error;
                goto has_restart;
            }

            /*
             * This ends ptrace-stop, but does *not* end group-stop.
             * This makes stopping signals work properly on straced process
             * (that is, process really stops. It used to continue to run).
             */
            if (!my_ptrace("skip_GROUPSTOP", PTRACE_LISTEN, tid, 0, 0))
                goto fatal_error;
            goto has_restart;
        }
        break;
        case WaitStatus::SYSCALL_STOP:
        {
            /**
             * Syscall events track syscalls through entry into the kernel,
             * processing in the kernel, and exit from the kernel.
             *
             * This also models interrupted syscalls.  During recording, only
             * descheduled buffered syscalls /push/ syscall interruptions; all
             * others are detected at exit time and transformed into syscall
             * interruptions from the original, normal syscalls.
             *
             * Normal system calls (interrupted or not) record two events: ENTERING_SYSCALL
             * and EXITING_SYSCALL. If the process exits before the syscall exit (because
             * this is an exit/exit_group syscall or the process gets SIGKILL), there's no
             * syscall exit event.
             *
             * When PTRACE_SYSCALL is used, there will be three events:
             * ENTERING_SYSCALL_PTRACE to run the process until it gets into the kernel,
             * then ENTERING_SYSCALL and EXITING_SYSCALL. We need three events to handle
             * PTRACE_SYSCALL with clone/fork/vfork and execve. The tracee must run to
             * the ENTERING_SYSCALL_PTRACE state, allow a context switch so the ptracer
             * can modify tracee registers, then perform ENTERING_SYSCALL (which actually
             * creates the new task or does the exec), allow a context switch so the
             * ptracer can modify the new task or post-exec state in a PTRACE_EVENT_EXEC/
             * CLONE/FORK/VFORK, then perform EXITING_SYSCALL to get into the correct
             * post-syscall state.
             *
             * When PTRACE_SYSEMU is used, there will only be one event: an
             * ENTERING_SYSCALL_PTRACE.
             */
            /*TODO: HOW to handle interrupted syscall was restart?*/
#if defined(__aarch64__)
			static struct iovec io = {
				.iov_base = &regs,
				.iov_len = sizeof(regs)
			};
            int ok = my_ptrace("syscall_stop", PTRACE_GETREGSET, tid, (void*)NT_PRSTATUS, &io);
#else
            int ok = my_ptrace("syscall_stop", PTRACE_GETREGS, tid, nullptr, &regs);
#endif
            if (!ok) {
                goto do_restart;
            }

            auto state = get_thread_state(tid);
#if defined(__mips64) || defined(__mips__) || defined(__sw_64) || defined(__aarch64__)
            if (state->second.state >= 0) {
                int syscall_no = get_syscall_no(&regs);
                get_syscall_args(&regs, state->second.args);
                if (process_syscall_enter(tid, syscall_no, state->second.args)) {
                    state->second.state = syscall_no | SYSCALL_ENTER_MASK;
                }
                else {
                    state->second.state = 0; //skip syscall exit-stop
                }
            }
            else {
                // ON MIPS: regs.v0 always return 0 when syscall exit-stop
                // FIXME: regs.a3=1 indicate failed of current syscall
                process_syscall_exit(tid, state->second.state & SYSCALL_NO_MASK,
                        state->second.args, get_syscall_result(&regs));
                state->second.state = 0;
            }
#else
            int syscall_no = get_syscall_no(&regs);
            if (syscall_no < 0) {
                LOG(WARNING) << "syscall-number is negative for tracee " << tid;
                // negative syscall are treated as skip this call
                // for some syscall: SYS_rt_sigreturn, SYS_exit_group never return.
                state->second.state = 0; //skip syscall exit-stop
                goto do_restart;
            }

            if ((state->second.state < 0) &&
                (syscall_no != (state->second.state & SYSCALL_NO_MASK))) {
                long previous = (state->second.state & SYSCALL_NO_MASK);
                LOG(WARNING) << "syscall-exit missing for tracee " << tid
                    << ", current=" << syscall_name(syscall_no)
                    << ", previous=" << syscall_name(previous);
                state->second.state = 0;
                ++m_counter.syscall_exit_miss;

                // fake a EINTR result
                if (is_number_in_set(previous, &g_trace_set[0])) {
                    long syscall_result = -EINTR;
                    dump_event(DUMP_REASON_syscall_exit+previous,
                            tid, &syscall_result, sizeof(syscall_result));
                }
            }

            if (state->second.state >= 0) {
                get_syscall_args(&regs, state->second.args);
                if (process_syscall_enter(tid, syscall_no, state->second.args)) {
                    state->second.state = syscall_no | SYSCALL_ENTER_MASK;
                }
                else {
                    state->second.state = 0; //skip syscall exit-stop
                }
            }
            else {
                process_syscall_exit(tid, syscall_no, state->second.args,
                        get_syscall_result(&regs));
                state->second.state = 0;
            }
#endif
        }
        break;
        case WaitStatus::PTRACE_EVENT:
        {
            int event = status.ptrace_event();
            LOG(DEBUG) << "(" << ptrace_event_name(event) << ") for tracee " << tid;

            switch (event) {
                case PTRACE_EVENT_EXEC:
                {
                    /*
                     * Under Linux, execve changes pid to thread leader's pid,
                     * and we see this changed pid on EVENT_EXEC and later,
                     * execve sysexit. Leader "disappears" without exit
                     * notification. Let user know that, drop leader's tcb,
                     * and fix up pid in execve thread's tcb.
                     * Effectively, execve thread's tcb replaces leader's tcb.
                     *
                     * BTW, leader is 'stuck undead' (doesn't report WIFEXITED
                     * on exit syscall) in multithreaded programs exactly
                     * in order to handle this case.
                     *
                     * PTRACE_GETEVENTMSG returns old pid starting from Linux 3.0.
                     * On 2.6 and earlier, it can return garbage.
                     */
                    int old_tid = 0;
                    if (!my_ptrace("PTRACE_EVENT_EXEC",
                                PTRACE_GETEVENTMSG, tid, NULL, &old_tid)) {
                        ++m_counter.ptrace_error;
                    }
                    else if (old_tid <=0 || old_tid == tid) {
                    }
                    else {
                        remove_thread(old_tid);
                        add_thread(tid);
                        LOG(INFO) << "pid has changed from " << old_tid << " to " << tid;
                    }

                    // maybe can receive PTRACE_EVENT_EXEC but no receive exec syscall exit
                    m_exec_stop = true;
                    post_exec_syscall(tid);
                }
                break;
                case PTRACE_EVENT_CLONE:
                case PTRACE_EVENT_FORK:
                case PTRACE_EVENT_VFORK:
                {
                    pid_t new_tid = 0;
                    if (my_ptrace("PTRACE_EVENT_FORK",
                                PTRACE_GETEVENTMSG, tid, NULL, &new_tid)) {
                        if (!add_thread(new_tid)) {
                            shared_ptr<TraceProcess> child = make_shared<TraceProcess>(this);
                            m_parent->add_process(child, new_tid);
                            LOG(INFO) << new_tid << " newborn process on PTRACE_EVENT>>>";
                        }
                        else {
                            LOG(INFO) << new_tid << " newborn thread on PTRACE_EVENT>>>";
                        }
                        if (m_cfg->mode == FAST) {
                            if (is_number_in_set(SYS_clone, &g_trace_set[0])) {
                                dump_clone(tid, new_tid);
                            }
                        }
                    }
                    else {
                        ++m_counter.ptrace_error;
                    }
                }
                break;
                case PTRACE_EVENT_EXIT:
                {
                    /*The tracee is stopped early during process exit, when
                     registers are still available.
                     TODO: does it need to call dump_event, dump_maps ?
                     */
                    remove_thread(tid);
                    LOG(INFO) << tid << " thread exited <<<";
                    /*TODO: how to avoid child process enter zombie ?*/
                    m_cont_type = CONTINUE;
                }
                break;
                case PTRACE_EVENT_SECCOMP:
                {
                    ++m_counter.seccomp_stop;

                    /*After a PTRACE_EVENT_SECCOMP stop, seccomp will be rerun, with a
                      SECCOMP_RET_TRACE rule now functioning the same as a SECCOMP_RET_ALLOW.
                      Specifically, this means that if registers are not modified during
                      the PTRACE_EVENT_SECCOMP stop, the system call will then be allowed.
                      */
                }
                break;
            default:
                break;
            }
        }
        break;
    default:
        LOG(INFO)<< "Unknown status (" << status.type() << ") for tracee " << tid;
        break;
    }

do_restart:
    if (!my_ptrace("process_status",
                static_cast<__ptrace_request>(m_cont_type),
                tid, nullptr, reinterpret_cast<void*>(m_sig))) {
        goto fatal_error;
    }

    if (m_cont_type == CONTINUE && m_cfg->mode != FAST) {
        //NOTE: CONTINUE only run once, or all later syscall stop will missing!
        m_cont_type = CONTINUE_SYSCALL;
    }

has_restart:
    m_sig = 0;
    return true;

fatal_error:
    ++m_counter.ptrace_error;
    remove_thread(tid);
    return false;
}

bool TraceProcess::start_record(pid_t pid2)
{
    MDRawSystemInfo info;
    int size = sizeof(info);

    m_pid = pid2;
    m_nonce = g_nonce++;
    LOG(INFO) << "open process " << this << ", for tracee " << m_pid;

    get_system_info(&info);
    if (m_cfg->mode == FAST) {
        memcpy(info.mode, "fast", 4);
    }
    else if (m_cfg->mode == NORMAL) {
        memcpy(info.mode, "hard", 4);
    }

    string filename;
    filename = m_cfg->dump_dir + CONTEXT_FILE_NAME + std::to_string(m_pid);
    m_ctx_file.open(filename.data(), m_cfg->compress_level);
    if (!m_ctx_stream.init(m_pid, m_cfg->shared_buffer_size, &m_ctx_file)) {
        LOG(FATAL) << "Failed to initialized thread stream for " << m_pid;
    }

    if (m_cfg->mode == FAST) {
        filename += ".clone";
        m_ctx_file2.open(filename.data(), m_cfg->compress_level);
        if (!m_ctx_stream2.init(m_pid, 1024*1024, &m_ctx_file2)) {
            LOG(FATAL) << "Failed to initialized thread stream for " << m_pid;
        }
    }

    // dump system info
    m_counter.total_dumped = m_ctx_file.write(&size, sizeof(size));
    m_counter.total_dumped += m_ctx_file.write(&info, sizeof(info));

    filename = m_cfg->dump_dir + MAP_FILE_NAME + std::to_string(m_pid);
    m_maps_file.open(filename.data(), m_cfg->compress_level);

    // dump system info
    m_maps_file.write(&size, sizeof(size));
    m_maps_file.write(&info, sizeof(info));

    m_sig = 0;
    add_thread(m_pid);

    m_cont_type = (FAST == m_cfg->mode) ? CONTINUE : CONTINUE_SYSCALL;

    if (m_cfg->mode == FAST) {
        setup_listener();
    }
    m_begin_time = time(NULL);

    return true;
}

bool TraceProcess::get_sym_address(const char* object_name,
        const char *name, unsigned long *addr)
{
    string libname;
    if (!strcmp(object_name, "libpthread.so.0")) {
        libname = PTHREAD_LIB;
    }
    else {
        libname = "/lib/x86_64-linux-gnu/";
        libname += object_name;
        // TODO: should parse link(libname)
    }
    auto it = m_symbols.find(libname);
    if (it != m_symbols.end()) {
        if (it->second.get()->get_sym_address(name,
                    addr, elf::stt::object)) {
            return true;
        }

        return it->second.get()->get_sym_address(name,
                    addr, elf::stt::func);
    }
    return false;
}

void TraceProcess::break_at_function(pid_t tid)
{
    uintptr_t address = m_breakpoint.address;
    const char* func = m_cfg->break_function.data();
    if (address > 0) {
        if (gdb_break(tid, address, &m_breakpoint.value) >= 0) {
            m_breakpoint.address = address;
            LOG(INFO) << "Set break at " << func
                << ":" << HEX(address) << " for tracee " << tid;
        }
    }
    else {
        LOG(WARNING) << "Not found function: " << func
            << " for tracee " << tid
            << ", will try search at next time !" << tid;
    }
}

bool TraceProcess::remove_break_function(pid_t tid)
{
    USER_REGS regs;

#if defined(__aarch64__)
	static struct iovec io = {
		.iov_base = &regs,
		.iov_len = sizeof(regs)
	};
    ptrace(PTRACE_GETREGSET, tid, NT_PRSTATUS, &io);
#else
    ptrace(PTRACE_GETREGS, tid, nullptr, &regs);
#endif

#if defined(__x86_64__)
    if (regs.rip == m_breakpoint.address + 1) {
        --regs.rip;
#elif defined(__mips64) || defined(__aarch64__) || defined(__sw_64)
    // ARM:     pc + 4 or pc no changed
    // MIPS:    pc no changed
    // SUNWAY:  pc + 4
    if (regs.pc == m_breakpoint.address || regs.pc == m_breakpoint.address + 4) {
        if (regs.pc == m_breakpoint.address + 4) regs.pc -= 4;
#else
    assert(0);
    if (0) {
#endif
        LOG(INFO) << "hit break at "
#if defined(__x86_64__)
            << HEX(regs.rip)
#elif defined(__mips64) || defined(__sw_64) || defined(__aarch64__)
            << HEX(regs.pc)
#else
            << HEX(regs.ip)
#endif
            << " for tracee " << tid;

        m_can_dump = true;
        dump_maps(-SIGTRAP, PROT_EXEC|PROT_WRITE);
        gdb_delete(tid, m_breakpoint.address, m_breakpoint.value);
        m_breakpoint.address = 0;

        // reset pc to the begin of the target function.
#if defined(__aarch64__)
        ptrace(PTRACE_SETREGSET, tid, NT_PRSTATUS, &io);
#else
        ptrace(PTRACE_SETREGS, tid, nullptr, &regs);
#endif
        ptrace(PTRACE_SYSCALL, tid, 0, 0);
        return true;
    }

    return false;
}
