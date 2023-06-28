// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _SESSION_H
#define _SESSION_H

#include <vector>
#include <string>
#include <map>
#include <memory>

/*third party lib*/
#include <elf.h>
#include <link.h>
#include <sys/auxv.h>
#include <unistd.h>

#include "config.h"
#include "trace_writer.h"
#include "zstd_writer.h"
#include "WaitStatus.h"
#include "shared_mem_dump.h"
#include "md_types.h"
#include "elf_helper.h"

#define  RESERVED_SOCKET_FD         (1001)

struct MappingInfo {
    uintptr_t start_addr;
    int size;
    int offset;  // offset into the backed file.
    unsigned char exec:1;
    char name[NAME_MAX];
};

typedef std::map<std::string, std::shared_ptr<SymbolFile>> ElfSymbolFiles;


const int SYSCALL_ENTER_MASK = 0x80000000;
const int SYSCALL_NO_MASK = 0x7fffffff;
typedef struct tagSyscallState{
    // bit 0~30 is syscall number;
    // bit 31 is syscall enter flag;
    int         state;
    // bool        interrupted;
    uintptr_t   args[6];
}SyscallState;

class TraceProcess {
public:
    typedef enum tagThreadState{
        TS_RUNNING  = 0,
        TS_SLEEPING,
        TS_DISK_SLEEP,
        TS_STOPPED,
        TS_TRACING_STOP,
        TS_ZOMBIE,
        TS_DEAD,
    }ThreadState;

    enum ContinueType {
        DONT_CONTINUE = 0,
        CONTINUE = PTRACE_CONT,
        CONTINUE_SYSCALL = PTRACE_SYSCALL,
    };

    enum SyscallBeforeSeccomp {
        SYSCALL_BEFORE_SECCOMP = 0,
        SECCOMP_BEFORE_SYSCALL,
        SYSCALL_BEFORE_SECCOMP_UNKNOWN,
    };

    TraceProcess(DumpConfig* config);
    TraceProcess(TraceProcess* parent);
    ~TraceProcess(void);
    void stop_record(void);
    int setup_socket(int* fd_number);
    int handle_connect(void);
    int setup_listener(void);

    bool process_status(int status, pid_t tid);
    bool start_record(pid_t tid);
    bool empty(void) {
        for (auto& i : m_childs) {
            if (!i.get()->get_threads().empty()) return false;
        }
        return m_syscall_state.empty();
    }
    DumpConfig* get_config(void) { return m_cfg;}
    int dump_event(int reason, pid_t tid, void* extra_data, int extra_data_size);
    int dump_clone(pid_t tid, pid_t newtid); //only used in fast mode
    int dump_maps(int reason, int map_prot);

    std::map<pid_t, SyscallState>& get_threads(void) { return m_syscall_state; }

    typedef struct tagVariableEx {
        bool        is_loaded;
        bool        is_pointer;
        bool        is_pointer_loaded;
        int         max_size;
        uintptr_t   address;
        uintptr_t   address_ptr; // pointer point to memory
        string      sym_name;
    }VariableEx;

    TraceProcess* get_process(pid_t tid) {
        if (m_syscall_state.find(tid) != m_syscall_state.end()) {
            return this;
        }

        for (auto& i : m_childs) {
            std::map<pid_t, SyscallState>& ids = i.get()->get_threads();
            if (ids.find(tid) != ids.end()) {
                return i.get();
            }
        }

        return nullptr;
    }

    void add_process(shared_ptr<TraceProcess> child, pid_t tid) {
        m_childs.push_back(child);
        child.get()->start_record(tid);
    }

    bool add_thread(pid_t tid) {
        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/task/%d", m_pid, tid);
        if (access(path, F_OK) == 0) {
            SyscallState st = {0, false, 0};
            m_syscall_state.insert(pair<pid_t, SyscallState>(tid, st));
            return true;
        }

        // A newborn process found!
        return false;
    }

    void remove_thread(pid_t tid) {m_syscall_state.erase(tid);}
    bool get_sym_address(const char* object_name,
        const char *name, unsigned long *addr);

private:

    TraceProcess*       m_parent;
    long                m_sig;
    ContinueType        m_cont_type;
    pid_t               m_pid;
    int                 m_nonce;

    DumpConfig*         m_cfg;
    bool                m_can_dump;
    bool                m_exec_stop;

    struct {
        uintptr_t       address;
        uint32_t        value;
    }m_breakpoint;

    int                 m_page_size;

    struct {
        int             syscall_enter;
        int             syscall_exit;
        int             syscall_exit_miss;
        int             seccomp_stop;
        int             flush_buffer;
        int             maps_change;
        int             event_dumped;
        int             ptrace_error;
        long            hook_dumped;
        long            total_dumped;
    }m_counter;

    MemoryDumper*       m_syscall_dumper;

    std::map<pid_t, SyscallState> m_syscall_state;
    std::list<std::shared_ptr<TraceProcess>> m_childs;

    std::vector<elf_aux_val_t> m_auxv;
    std::vector<VmSegment> m_heaps;
    std::vector<VmSegment> m_heaps_temp; // use to restore m_heaps
    std::vector<VmSegment> m_syscall_memblks;
    std::vector<VariableEx> m_global_vars;
    std::vector<ThreadContext> m_thread_ctx;
    std::vector<shared_ptr<MappingInfo> > m_mappings;

    ElfSymbolFiles      m_symbols;

    ZstdWriter          m_maps_file;
    TraceStream         m_ctx_stream;
    ZstdWriter          m_ctx_file;

    // the file only record clone event in fast mode
    TraceStream         m_ctx_stream2;
    ZstdWriter          m_ctx_file2;

    int                 m_recv_fd;
    int                 m_server_socket;
    int                 m_recv_socket;
    int                 m_send_socket;

    time_t              m_begin_time;
    time_t              m_end_time;

    bool remove_break_function(pid_t tid);
    void break_at_function(pid_t tid);
    void dump_debugger_count(void);
    void reset_debugger_count(void);
    bool handle_seccomp_event(pid_t tid);
    void process_magic_syscall(pid_t tid, int no, uintptr_t* args);
    bool process_syscall_exit(pid_t tid, int no, uintptr_t* args, long result);
    bool process_syscall_enter(pid_t tid, int no, uintptr_t* args);
    bool process_signal(pid_t tid);
    int prepare_clone(pid_t tid, int no, uintptr_t* args);
    int dump_thread_list(pid_t tid);
    int dump_thread_status(pid_t tid);
    int dump_thread_context(ThreadContext* ctx);
    int dump_thread_stack(ThreadContext* ctx);
    int dump_proc_file(pid_t m_pid, const char* name);
    int dump_auxv(pid_t m_pid);
    void post_exec_syscall(pid_t tid);
    void init_shared_buffers(int size);
    void get_share_name(uintptr_t name);
    int get_shared_buffers(char** buf);
    int flush_shared_buffers(void);
    void merge_heap(vector<VmSegment>& rw_segs);
    void check_global_pointer_var_is_assigned(void);

    std::map<pid_t, SyscallState>::iterator get_thread_state(pid_t tid) {
        auto it = m_syscall_state.find(tid);
        if (it != m_syscall_state.end()) {
            return it;
        }

        assert(0);
        return m_syscall_state.end();
    }
};

#endif

