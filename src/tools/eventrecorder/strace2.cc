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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/unistd.h>
#include <linux/capability.h>
#include <libgen.h>
#include <sys/statvfs.h>

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <memory>

#include "WaitStatus.h"
#include "ScopedFd.h"
#include "config.h"
#include "utils.h"
#include "session.h"
#include "replay.h"
#include "easylogging++.h"
#include "event_man.h"
#include "preload/x11preload.h"

INITIALIZE_EASYLOGGINGPP

using namespace std;

static void usage(const char* name)
{
    printf("usage:\n");
    printf("\n%s ps [trace-dir] # list pid of all process recorded\n", name);
    printf("\n%s dump [trace-dir] [pid] # dump raw event\n", name);
    printf("\n%s replay [trace-dir] [pid] # view event in interactive console\n", name);
    printf("\n%s [options] executable-file [executable parameter...] # record event\n", name);
    printf("options: \n");
    printf("  -1: only record current thread stack\n");
    printf("  --stack-size=: specified maximum K-bytes of stack to dump\n");
    printf("  --heap-size=: specified maximum K-bytes of heap to dump\n");
    printf("  --param-size=: specified maximum bytes of syscall parameter to dump\n");
    printf("  --var=[*]var1+size1[,[*]var2+size2,...]: Trace global variables\n");
    printf("  --func=mangle-name: Trace start after the specified function called(see nm -C)\n");
    printf("  --vdso=on/off: Turn on/off intercept function in vdso\n");
    printf("  --sys=[!]filter: filter can be one or more of these\n"
           "    [file,ipc,network,process,signal,desc,memory, or concrete syscall-name / syscall-no]\n"
           "    file,     Trace all system calls which take a file name as an argument.\n"
           "    process,  Trace all system calls which involve process management.\n"
           "    network,  Trace all the network related system calls.\n"
           "    signal,   Trace all signal related system calls.\n"
           "    ipc,      Trace all IPC related system calls.\n"
           "    desc,     Trace all file descriptor related system calls.\n"
           "    memory,   Trace all memory mapping related system calls.\n");
    printf("  --sig=signal-list: signal-list can be one or more of `kill -l`\n");
    printf("  --dbus=type-list: type-list can be one or more of these\n"
           "     1,  method call\n"
           "     2,  method return\n"
           "     3,  error\n"
           "     4,  signal\n");
    printf("  --x11=event-list: event-list can be one or more of these\n"
           "     2,  KeyPress\n"
           "     3,  KeyRelease\n"
           "     4,  ButtonPress\n"
           "     5,  ButtonRelease\n"
           "     9,  FocusIn\n"
           "    10,  FocusOut\n"
           "    16,  CreateNotify\n"
           "    17,  DestroyNotify\n"
           "    18,  UnmapNotify\n"
           "    19,  MapNotify\n");
}

static void rolloutHandler(const char* filename, std::size_t size)
{
    (void)size;
    static unsigned int log_idx = 0;

#ifdef _DEBUG
    // SHOULD NOT LOG ANYTHING HERE BECAUSE LOG FILE IS CLOSED!
    std::cout << "************** Rolling out [" << filename
       << "] because it reached [" << size << " bytes]" << std::endl;
#endif

    // BACK IT UP
    std::stringstream ss;
    ss << "mv " << filename << " " << filename << "-backup." << ++log_idx;
    system(ss.str().c_str());
}

static void spawned_child_fatal_error(const ScopedFd& err_fd, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char* buf;
    if (vasprintf(&buf, format, args) < 0) {
        exit(1);
    }

    char* buf2;
    if (asprintf(&buf2, "%s (%s)", buf, strerror(errno)) < 0) {
        exit(1);
    }
    write(err_fd, buf2, strlen(buf2));
    _exit(1);
}

string read_spawned_task_error(int fd) {
    char buf[1024] = "";
    ssize_t len = 0;
    for (int i = 0; i<2; ++i) {
        len = read(fd, buf, sizeof(buf));
        if (len > 0) {
            buf[len] = 0;
            LOG(ERROR) << "tracee error message: " << buf;
            return string(buf, len);
        }
        sleep(1);
    }
    return string();
}

/**
 * Prepare this process and its ancestors for recording/replay by
 * preventing direct access to sources of nondeterminism, and ensuring
 * that rr bugs don't adversely affect the underlying system.
 */
static void set_up_process(const ScopedFd& err_fd,
        int send_sock, int send_sock_fd_number)
{
    if (send_sock_fd_number != dup2(send_sock, send_sock_fd_number)) {
        spawned_child_fatal_error(err_fd, "error duping to RESERVED_SOCKET_FD");
    }

    /* If we're in setuid_sudo mode, we have CAP_SYS_ADMIN, so we don't need to
     set NO_NEW_PRIVS here in order to install the seccomp filter later. In,
     emulate any potentially privileged, operations, so we might as well set
     no_new_privs */
    if (!has_effective_caps()) {
        if (0 > prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
          spawned_child_fatal_error( err_fd,
              "prctl(NO_NEW_PRIVS) failed, SECCOMP_FILTER is not available: your "
              "kernel is too old. Use `record -n` to disable the filter.");
        }
    }
}

int run_child (const ScopedFd& error_fd,
    int send_sock,
    int send_sock_fd_number,
    const char* exe_path_cstr,
    const vector<string>& argv,
    const vector<string>& envp)
{
    pid_t pid = getpid();
    StringVectorToCharArray argv_array(argv);
    StringVectorToCharArray envp_array(envp);

    set_up_process(error_fd, send_sock, send_sock_fd_number);

    /* Signal to tracer that we're configured.
     * Induce a ptrace stop. Tracer (our parent)
     * will resume us with PTRACE_SYSCALL and display
     * the immediately following execve syscall.
     * Can't do this on NOMMU systems, we are after
     * vfork: parent is blocked, stopping would deadlock.
     */
    kill(pid, SIGSTOP);

    execve(exe_path_cstr, argv_array.get(), envp_array.get());

    switch (errno) {
    case ENOENT:
        spawned_child_fatal_error( error_fd,
              "execve failed: '%s' (or interpreter) not found", exe_path_cstr);
        break;
    default:
        spawned_child_fatal_error(error_fd, "execve of '%s' failed", exe_path_cstr);
        break;
    }
    // Never returns!

    return 0;
}

int attach_thread(pid_t tid, bool trace_syscall)
{
    intptr_t options = PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK |
        PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT;
    if (trace_syscall) {
        options |= PTRACE_O_TRACESYSGOOD;
    }
    int ret = ptrace(PTRACE_SEIZE, tid, nullptr, (void*)(options | PTRACE_O_EXITKILL));
    if ((ret < 0) && (errno == EINVAL)) {
        // PTRACE_O_EXITKILL was added in kernel 3.8, and we only need
        // it for more robust cleanup, so tolerate not having it.
        ret = ptrace(PTRACE_SEIZE, tid, nullptr, (void*)options);
    }
    if (ret < 0 ) {
        LOG(ERROR) << "PTRACE_SEIZE failed for tid:" << tid
                   << ", ret:" << ret << ", errno:" << errno;
    }

    return ret;
}

static string find_preload_library(const string& name, const char* preload_list)
{
    string lib_path("./");
    size_t pos = name.rfind('/');
    if ( pos != string::npos) {
        lib_path = name.substr(0, pos+1);
    }

    string ret;
    string file_name;
    bool done = false;
    const char* walk = preload_list;

    lib_path += "lib";
    while (!done) {
        const char* pos = strchr(walk, ',');
        if (pos != nullptr) {
            file_name = lib_path + string(walk, pos - walk) + "preload.so";
            walk = pos + 1;
        }
        else {
            file_name = lib_path + string(walk) + "preload.so"; // last item
            done = true;
        }
        if (access(file_name.c_str(), F_OK) == 0) {
            ret += file_name;
            if (!done) ret += ":";
        }
    }

    LOG(INFO) << "user preload:" << ret.data();

    return ret;
}

string find_needed_library_starting_with(const string& exe_file,
        const string& prefix);

static string lookup_by_path(const string& name) {
    if (name.find('/') != string::npos) {
        return name;
    }
    const char* env = getenv("PATH");
    if (!env) {
        return name;
    }
    char* p = strdup(env);
    char* s = p;
    while (*s) {
        char* next = strchr(s, ':');
        if (next) {
            *next = 0;
        }
        string file = string(s) + "/" + name;
        struct stat st;
        if (!stat(file.c_str(), &st) && S_ISREG(st.st_mode) &&
                !access(file.c_str(), X_OK)) {
            free(p);
            return file;
        }
        if (!next) {
            break;
        }
        s = next + 1;
    }
    free(p);
    return name;
}

void init_env(vector<string>& env,
        const string& exe_path,
        const string& preload_lib_path)
{
    char** envp = environ;
    for (; *envp; ++envp) {
        env.push_back(*envp);
    }

    string full_path = lookup_by_path(string(exe_path));
    if (!preload_lib_path.empty()) {
        string ld_preload = "LD_PRELOAD=";
        string libasan = find_needed_library_starting_with(full_path, "libasan");
        if (!libasan.empty()) {
            // Put an LD_PRELOAD entry for it before our preload library, because
            // it checks that it's loaded first
            ld_preload += libasan + ":";
        }
        // Our preload lib should come first if possible, because that will
        // speed up the loading of the other libraries. We supply a placeholder
        // which is then mutated to the correct filename in
        // Monkeypatcher::patch_after_exec.
        ld_preload += preload_lib_path;
        auto it = env.begin();
        for (; it != env.end(); ++it) {
            if (it->find("LD_PRELOAD=") != 0) {
                continue;
            }
            // Honor old preloads too.  This may cause
            // problems, but only in those libs, and
            // that's the user's problem.
            ld_preload += ":";
            ld_preload += it->substr(it->find("=") + 1);
            break;
        }
        if (it == env.end()) {
            env.push_back(ld_preload);
        }
        else {
            *it = ld_preload;
        }
        LOG(INFO) << "final preload:" << ld_preload.data();
    }

    env.push_back("RUNNING_UNDER_RR=1");

    // Stop Mesa using the GPU
    env.push_back("LIBGL_ALWAYS_SOFTWARE=1");

    // Stop sssd from using shared-memory with its daemon
    // env.push_back("SSS_NSS_USE_MEMCACHE=NO");

    // Disable Gecko's "wait for gdb to attach on process crash" behavior, since
    // it is useless when running under rr.
    env.push_back("MOZ_GDB_SLEEP=0");

    // OpenSSL uses RDRAND, but we can disable it. These bitmasks are inverted
    // and ANDed with the results of CPUID. The number below is 2^62, which is the
    // bit for RDRAND support.
    env.push_back("OPENSSL_ia32cap=~4611686018427387904:~0");
}

void init_log(DumpConfig& cfg, const char* filename)
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
    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format, "%datetime %level %msg");
    defaultConf.set(el::Level::Warning, el::ConfigurationType::Format, "%datetime %level %msg");
    defaultConf.set(el::Level::Error, el::ConfigurationType::Format, "%datetime %level %msg");
    defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime %level %msg");

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

// This function doesn't really need to do anything. The signal will cause
// // waitpid to return EINTR and that's all we need.
static void handle_alarm_signal(__attribute__((unused)) int sig) {}

static void ensure_dir(const string& dir, mode_t mode)
{
    string d = dir;
    while (!d.empty() && d[d.length() - 1] == '/') {
        d = d.substr(0, d.length() - 1);
    }

    struct stat st;
    if (0 > stat(d.c_str(), &st)) {
        if (errno != ENOENT) {
          LOG(ERROR) << "Error accessing " << dir << "'";
        }

        size_t last_slash = d.find_last_of('/');
        if (last_slash == string::npos || last_slash == 0) {
          LOG(ERROR) << "Can't find directory `" << dir << "'";
        }
        ensure_dir(d.substr(0, last_slash), mode);

        // Allow for a race condition where someone else creates the directory
        if (0 > mkdir(d.c_str(), mode) && errno != EEXIST) {
          LOG(ERROR) << "Can't create `" << dir << "'";
        }
        if (0 > stat(d.c_str(), &st)) {
          LOG(ERROR) << "Can't stat `" << dir << "'";
        }
    }

    if (!(S_IFDIR & st.st_mode)) {
        LOG(ERROR) << "`" << dir << "' exists but isn't a directory.";
    }
    if (access(d.c_str(), W_OK)) {
        LOG(ERROR) << "Can't write to `" << dir << "'.";
    }
}

static void create_trace_dir(string& parent_dir, const char* exe_path)
{
    string dir;
    int ret;
    int nonce = 0;
    const char* pbasename = basename((char*)exe_path);

    if (parent_dir[parent_dir.size()-1] != '/')
        parent_dir += '/';
    do {
        stringstream ss;
        ss << parent_dir.data() << pbasename << "-"
           << nonce++;
        dir = ss.str();
        ret = mkdir(dir.c_str(), S_IRWXU | S_IRWXG);
    } while (ret && EEXIST == errno);

    if (ret) {
        LOG(ERROR) << "Unable to create trace directory '" << dir << "'";
    }
    parent_dir = dir + '/';
    LOG(INFO) << "trace dir:" << parent_dir.data();
}

static void check_free_disk_space(DumpConfig& cfg)
{
    struct statvfs tmpInfo;
    struct statvfs homeInfo;
    int ret = statvfs("/tmp", &tmpInfo);
    if (ret) {
        fprintf(stderr, "ERROR: failed to statvfs(/tmp) -> %d\n", errno);
        exit(-1);
    }
    ret = statvfs(cfg.dump_dir.data(), &homeInfo);
    if (ret) {
        fprintf(stderr, "ERROR: failed to statvfs(%s) -> %d\n",
                cfg.dump_dir.data(), errno);
        exit(-2);
    }

#define MIN_RESERVED_SPACE (256*1024*1024)

    long tmpFree = tmpInfo.f_bsize * tmpInfo.f_bavail;
    long homeFree = homeInfo.f_bsize * homeInfo.f_bavail;
    if (tmpFree < MIN_RESERVED_SPACE) {
        fprintf(stderr, "ERROR: /tmp volume too small, must larger than %dMB\n",
                MIN_RESERVED_SPACE/(1024*1024));
        exit(-3);
    }
    if (homeFree < MIN_RESERVED_SPACE) {
        fprintf(stderr, "ERROR: %s volume too small, must larger than %dMB\n",
                cfg.dump_dir.data(), MIN_RESERVED_SPACE/(1024*1024));
        exit(-4);
    }

    if (cfg.max_dump_bytes + MIN_RESERVED_SPACE > tmpFree) {
        cfg.max_dump_bytes = tmpFree - MIN_RESERVED_SPACE;
    }
    if (cfg.max_dump_bytes + MIN_RESERVED_SPACE > homeFree) {
        cfg.max_dump_bytes = homeFree - MIN_RESERVED_SPACE;
    }
}


static void make_latest_trace(const string& trace_dir)
{
    string link_name(trace_dir.c_str(), trace_dir.size() -1);
    int pos = link_name.rfind('/');
    link_name.replace(pos + 1, link_name.size() - pos -1, LATEST_TRACE_NAME);

    // Try to update the symlink to |this|.  We only try attempt
    // to set the symlink once.  If the link is re-created after
    // we |unlink()| it, then another rr process is racing with us
    // and it "won".  The link is then valid and points at some
    // very-recent trace, so that's good enough.
    unlink(link_name.c_str());

    // Link only the trace name, not the full path, so moving a directory full
    // of traces around doesn't break the latest-trace link.
    string trace_name(trace_dir.c_str() + pos + 1, trace_dir.size() - pos -2);
    int ret = symlink(trace_name.c_str(), link_name.c_str());
    if (ret < 0 && errno != EEXIST) {
        LOG(ERROR) << "Failed to update symlink `" << link_name << "' to `"
            << trace_dir << "'.";
    }
}

extern void set_syscall_filter(const char* filter, bool reset);


// [*]var1+size1[,[*]var2+size2,...]
static void parse_global_var(DumpConfig& cfg, const char* namelist)
{
    Variable tmp;

    char* pos = NULL;
    const char* p = namelist;
    while (*p) {
        tmp.is_pointer = (*p == '*');
        if (tmp.is_pointer) ++p;

        // get size
        pos = (char*)strchr(p, '+');
        if (NULL == pos) {
            break;
        }
        tmp.sym_name = string(p, int(pos - p));
        tmp.max_size = strtol(pos + 1, &pos, 10);
        if (tmp.max_size > 0) {
            if (tmp.max_size < 256) tmp.max_size = 256;
            cfg.vars.push_back(tmp);
        }

        if (*pos != ',') break;
        p = pos + 1;
    }
}

static void parse_signal_filter(DumpConfig& cfg, const char* filter)
{
    const char* walk = filter;
    char* stop = nullptr;
    while (*walk > 0) {
        int v = strtol(walk, &stop, 10);
        cfg.sigs.push_back(v);
        if (0 == *stop) break;
        walk = stop + 1;
    }
}

#ifndef SIMPLE_X11_HOOK
static void (*g_stop_record_x11)(void) = nullptr;
static int (*g_start_record_x11)(const char* displayName, const char* filter) = nullptr;

static void* x11_record_thread(void* param)
{
    int ret = -1;
    if (g_start_record_x11) {
        ret = g_start_record_x11(":0", (const char*)param);
        if (ret < 0) {
        }
    }

    return nullptr;
}
#endif

bool is_elf(const char* filename);


int main (int argc, char** argv)
{
#ifndef SIMPLE_X11_HOOK
    bool record_x11_running = false;
#endif

    bool hook_vdso = false;
    int exe_index = 1;
    string preload;
    int fds[2];
    ScopedFd error_fd;
    ScopedFd spawned_task_error_fd;
    vector<string> envp;
    vector<string> argv_child;
    int tid = 0;
    int pid = 0;
    int raw_status = 0;
    WaitStatus status;
    DumpConfig cfg;

    cfg.mode = NORMAL;
    load_config(cfg);
    ensure_dir(cfg.dump_dir, S_IRWXU);
    init_log(cfg, "emd.log");

    if (argc < 2) {
        usage(argv[0]);
        return 0;
    }

    if (!strcmp(argv[1], "ps")) {
        return list_pid(cfg.dump_dir.c_str(), argc < 3 ? nullptr : argv[2]);
    }

    if (!strcmp(argv[1], "dump")) {
        if (argc < 2) {
            usage(argv[0]);
            return 0;
        }

        if (argc < 3) {
            return dump(cfg.dump_dir.c_str(), nullptr, 0);
        }
        if (argc < 4) {
            pid = atoi(argv[2]);
            if (pid > 0) {
                return dump(cfg.dump_dir.c_str(), nullptr, pid);
            }

            return dump(cfg.dump_dir.c_str(), argv[2], 0);
        }

        return dump(cfg.dump_dir.c_str(), argv[2], atoi(argv[3]));
    }

    if (!strcmp(argv[1], "replay")) {
        if (argc < 2) {
            usage(argv[0]);
            return 0;
        }

        if (argc < 3) {
            return replay(cfg.dump_dir.c_str(), nullptr, 0);
        }
        if (argc < 4) {
            pid = atoi(argv[2]);
            if (pid > 0) {
                return replay(cfg.dump_dir.c_str(), nullptr, pid);
            }

            return replay(cfg.dump_dir.c_str(), argv[2], 0);
        }

        return replay(cfg.dump_dir.c_str(), argv[2], atoi(argv[3]));
    }

    assert(cfg.mode <= FAST);

    set_syscall_filter("!all", true); /*default clear all syscall*/

    for (int i=1; i<argc; ++i) {
        if (!strncmp(argv[i], "--sys=", 6)) {
            if (argv[i][6] > 0) {
                set_syscall_filter(argv[i] + 6, true);
            }
        }
        else if (!strncmp(argv[i], "--dbus=", 7)) {
            if (argv[i][7] > 0) {
                string dbusenv = "ST2_DBUS_FILTER=";
                envp.push_back(dbusenv + (argv[i] + 7));
                preload += "dbus,";
            }
        }
        else if (!strncmp(argv[i], "--x11=", 6)) {
#ifdef SIMPLE_X11_HOOK
            if (argv[i][6] > 0) {
                string x11env = "ST2_X11_FILTER=";
                envp.push_back(x11env + (argv[i] + 6));
                preload += "x11,";
            }
#else
            void* handle = dlopen("libx11preload.so", RTLD_NOW);
            if (handle) {
                void* proc = dlsym(handle, "start_record_x11");
                memcpy(&g_start_record_x11, &proc, sizeof(void*));
                proc = dlsym(handle, "stop_record_x11");
                memcpy(&g_stop_record_x11, &proc, sizeof(void*));

                if (g_stop_record_x11 && g_stop_record_x11) {
                    pthread_t record_x11_thread;
                    pthread_create(&record_x11_thread, nullptr,
                            x11_record_thread, (void*)(argv[i] + 6));
                    record_x11_running = true;
                }
            }
            else {
                LOG(ERROR) << "failed to load libx11preload.so, errno=" << errno;
            }
#endif
        }
        else if (!strncmp(argv[i], "--sig=", 6)) {
            if (argv[i][6] > 0) {
                parse_signal_filter(cfg, argv[i] + 6);
            }
        }
        else if (!strncmp(argv[i], "-1", 2)) {
            // will override mode field in configure file
            cfg.current_thread_only = true;
        }
        else if (!strncmp(argv[i], "--var=", 6)) {
            if (argv[i][6] > 0) {
                parse_global_var(cfg, argv[i] + 6);
            }
        }
        else if (!strncmp(argv[i], "--func=", 7)) {
            if (argv[i][7] > 0) {
                cfg.break_function = argv[i] + 7;
            }
        }
        else if (!strncmp(argv[i], "--vdso=", 7)) {
            hook_vdso = !strcmp(argv[i] + 7, "on");
        }
        else if (!strncmp(argv[i], "--param-size=", 13)) {
            cfg.max_param_size = atoi(argv[i] + 13);
            if (cfg.max_param_size < 0) cfg.max_param_size = 0;
        }
        else if (!strncmp(argv[i], "--stack-size=", 13)) {
            cfg.max_stack_size = atoi(argv[i] + 13)*1024;
            if (cfg.max_stack_size < 0) cfg.max_stack_size = 0;
        }
        else if (!strncmp(argv[i], "--heap-size=", 12)) {
            cfg.max_heap_size = atoi(argv[i] + 12)*1024;
            if (cfg.max_heap_size < 0) cfg.max_heap_size = 0;
        }
        else {
            break;
        }
        LOG(INFO) << "parameter " << i << ":" << argv[i];

        exe_index = i + 1;
        if (exe_index >= argc) {
            usage(argv[0]);
#ifndef SIMPLE_X11_HOOK
            if (record_x11_running) g_stop_record_x11();
#endif
            return 0;
        }
    }
    if (!is_elf(argv[exe_index])) {
        usage(argv[0]);
        return 0;
    }

    string preload_path = find_preload_library(argv[0], "syscall");
    bool x11_dbus = preload.find("x11") != string::npos || preload.find("dbus") != string::npos;
    // X11,DBUS can't use fast mode, because the x11/dbus hook thread is not meaning!
    // if (x11_dbus) cfg.current_thread_only = false;
    if (cfg.current_thread_only &&
        !x11_dbus &&
        !cfg.max_heap_size &&
        cfg.vars.empty() &&
        !preload_path.empty()) {
        cfg.mode = FAST;
        if (string::npos == preload.find("syscall")) {
            preload += "syscall,";
        }
        string syscallenv = "ST2_SYSCALL_BUFFER_SIZE=";
        envp.push_back(syscallenv + to_string(cfg.shared_buffer_size));

        if (hook_vdso) {
            string hookvdsoenv = "ST2_HOOK_VDSO=1";
            envp.push_back(hookvdsoenv);
        }
    }
    else if (hook_vdso) {
#if !defined(__sw_64)
        /* hook these function in vdso
        __vdso_clock_gettime
        __vdso_gettimeofday
        __vdso_time
        __vdso_getcpu
        */
        preload += "vdso,";
#endif
        set_syscall_filter("gettimeofday,time,clock_gettime", false);
    }
    create_trace_dir(cfg.dump_dir, argv[exe_index]);

    check_free_disk_space(cfg);

    for (int i = exe_index; i<argc; ++i) {
        argv_child.push_back(argv[i]);
    }

    if (0 != pipe2(fds, O_CLOEXEC)) {
        LOG(ERROR) << "failed to create pipe!";
        return 0;
    }
    error_fd = fds[1];
    spawned_task_error_fd = fds[0];
    if (-1 == fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL) | O_NONBLOCK)) {
        LOG(WARNING) << "failed to set spawned_task_error_fd to O_NONBLOCK, errno=" << errno;
    }

    if (!preload.empty()) {
        if (preload[preload.size() - 1] == ',')
            preload.erase(preload.size() - 1);
        preload_path = find_preload_library(argv[0], preload.data());
        init_env(envp, string(argv[exe_index]), preload_path);
    }
    else {
        init_env(envp, string(argv[exe_index]), string(""));
    }

    TraceProcess process(&cfg);
    int send_socket_fd_number = 0;
    int send_sock = process.setup_socket(&send_socket_fd_number);

    do {
        pid = fork();
    } while ((pid<0) && (errno == EAGAIN));

    if (0 == pid) {
        run_child(error_fd, send_sock, send_socket_fd_number,
                argv[exe_index], argv_child, envp);
        /*target child process never returns*/
    }

    if (pid < 0) {
        LOG(ERROR) << "Failed to fork:" << errno;

        goto fatal_error;
    }

    if (attach_thread(pid, cfg.mode != FAST) < 0) {
        goto fatal_error;
    }

    // Install signal handler here, so that when creating the first TraceProcess
    // it sees the exact same signal state in the parent as will be in the child.
    struct sigaction sa;
    sa.sa_handler = handle_alarm_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // No SA_RESTART, so waitpid() will be interrupted
    sigaction(SIGALRM, &sa, nullptr);

    tid = waitpid(-1, &raw_status, __WALL);
    status = WaitStatus(raw_status);
    if ((status.type() == WaitStatus::PTRACE_EVENT) &&
            (status.ptrace_event() == PTRACE_EVENT_EXIT)) {
        LOG(ERROR) << "Tracee died before reaching SIGSTOP";
        goto fatal_error;
    }
    // SIGSTOP can be reported as a signal-stop or group-stop depending on
    // whether PTRACE_SEIZE happened before or after it was delivered.
    if (SIGSTOP != status.stop_sig() && SIGSTOP != status.group_stop()) {
        LOG(ERROR) << "Unexpected stop " << status.type();
        goto fatal_error;
    }

    make_latest_trace(cfg.dump_dir);

    /*restart tracee*/
    ptrace(PTRACE_SYSCALL, tid, nullptr, 0);

    LOG(INFO) << "start trace:" << argv[exe_index]
        << ", pid=" << pid << ", mode=" << cfg.mode;

    /*FIXME: is need call prctl(PR_SET_DUMPABLE, 1)*/
    process.start_record(pid);

    for(;;) {
        tid = waitpid(-1, &raw_status, __WALL);
        int wait_errno = errno;
        if (tid < 0) {
            if ((wait_errno == ECHILD) && process.empty()) {
                LOG(INFO) << "waitpid failed:" << wait_errno
                          << ", " << strerror(wait_errno);
                break;
            }

            continue;
        }

        TraceProcess* proc = process.get_process(tid);
        if (proc != nullptr) {
            if (!proc->process_status(raw_status, tid)) {
                break; // fatal error found
            }
        }

        if (process.empty()) {
            break;
        }
    }

fatal_error:
    process.stop_record();

    read_spawned_task_error(spawned_task_error_fd);

    if (send_sock) close(send_sock);
    if (pid > 0) kill(pid, SIGKILL);

#ifndef SIMPLE_X11_HOOK
    if (record_x11_running) g_stop_record_x11();
#endif

    return 0;
}
