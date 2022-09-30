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
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
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
#include <iomanip>

#include "event_man.h"
#include "easylogging++.h"
#include "replay.h"
#include "linenoise.h"

using namespace std;

static void get_program_file(char* linkname)
{
    FILE* pf = fopen(linkname, "rb");
    if (pf) {
        fscanf(pf, "%s", linkname);
        fclose(pf);
    }
}

bool is_prefix(const char* s, const char* of)
{
    return strstr(s, of) == s;
}

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> out{};
    std::stringstream ss {s};
    std::string item;

    while (std::getline(ss,item,delimiter)) {
        out.push_back(item);
    }

    return out;
}

static void run_coredump(const char* program_file, const char* core_file, int verbose)
{
#if 1
    char cmdline[1024];
    if (verbose) {
        snprintf(cmdline, sizeof(cmdline), "gdb %s %s",
            program_file, core_file);
    }
    else {
        snprintf(cmdline, sizeof(cmdline), "gdb -q %s %s",
            program_file, core_file);
    }
    system(cmdline);
#else
    // TODO: run gdb --interpret=mi2
    pid_t pid = fork();
    if (0 == pid) {
        char* argv[4];
        vector<char*> env;

        char** envp = environ;
        for (; *envp; ++envp) {
            env.push_back(*envp);
        }

        argv[0] = (char *)"gdb";
        argv[1] = (char *)program_file;
        argv[2] = (char *)core_file;
        argv[3] = nullptr;

        execve(argv[0], argv, &env[0]);
        // never return.
    }

    int status = 0;
    while (pid == waitpid(-1, &status, __WALL))
    {
        if (WIFEXITED(status) || WIFSTOPPED(status)) {
            break;
        }
    }
#endif
}

static void print_event(void* timeline, const EventEntry& entry, int i)
{
    char info[EVENT_EXTRA_INFO_SIZE];
    if (entry.type < DUMP_REASON_signal) {
        // syscall
        if (entry.syscall_result < 0xffff) {
            info[0] = 0;
            if (entry.syscall_result < 0) {
                snprintf(info, sizeof(info), "(%s)",
                    errno_name(-entry.syscall_result));
            }
            cerr << setw(8) << i
            << ": " << setw(16) << get_event_name(entry.type)
            << " ths=" << setw(2) << entry.thread_num
            << " tid=" << setw(5) << entry.tid
            << " result=" << entry.syscall_result << info
            << endl;
        } else {
            cerr << setw(8) << i
            << ": " << setw(16) << get_event_name(entry.type)
            << " ths=" << setw(2) << entry.thread_num
            << " tid=" << setw(5) << entry.tid
            << " result=" << HEX(entry.syscall_result) << endl;
        }

        if (get_event_extra_info(timeline, i, info, sizeof(info)) > 0) {
            cerr << "\t\t\t" << info;
        }
    }
    else if (entry.type < DUMP_REASON_dbus) {
        // signal
        cerr << setw(8) << i
            << ": " << setw(16) << get_event_name(entry.type)
            << " ths=" << setw(2) << entry.thread_num
            << " tid=" << setw(5) << entry.tid << endl;
    }
    else if (entry.type < DUMP_REASON_x11) {
        // dbus
        get_event_extra_info(timeline, i, info, sizeof(info));
        cerr << setw(8) << i
            << ": " << setw(16) << get_event_name(entry.type)
            << " ths=" << setw(2) << entry.thread_num
            << " tid=" << setw(5) << entry.tid
            << " " << info << endl;
    }
    else  {
        info[0] = 0;
        // x11
        get_event_extra_info(timeline, i, info, sizeof(info));
        cerr << setw(8) << i
            << ": " << setw(16) << get_event_name(entry.type)
            << " ths=" << setw(2) << entry.thread_num
            << " tid=" << setw(5) << entry.tid
            << " " << info << endl;
    }
}

static void list_event(void* timeline, int count, int begin, int end)
{
    EventEntry entry;
    if (end >= count -1) end = count -1;

    for (int i = begin; i<=end; ++i) {
        get_event(timeline, i, &entry);
        print_event(timeline, entry, i);
    }
}

enum CompareOp{
    None = 0,
    LT,
    LTE,
    EQ,
    GTE,
    GT,
};

static CompareOp parse_compare_op(const char* op, long* value)
{
    bool hex = (strstr(op, "0x") || strstr(op, "0X"));
    int base = hex ? 16:10;
    if (op[0] == '<') {
        if (op[1] == '=') {
            *value = strtol(op + 3, nullptr, base);
            return LTE;
        }

        *value = strtol(op + 2, nullptr, base);
        return LT;
    }
    else if (op[0] == '=') {
        if (op[1] == '=') {
            *value = strtol(op + 3, nullptr, base);
            return EQ;
        }
    }
    else if (op[0] == '>') {
        if (op[1] == '=') {
            *value = strtol(op + 3, nullptr, base);
            return GTE;
        }

        *value = strtol(op + 2, nullptr, base);
        return GT;
    }

    return None;
}

static void search_event(void* timeline, int count, int type_begin,
        int type_end, CompareOp op = None, long param = 0)
{
    EventEntry entry;

    for (int i = 0, j = 0; i<count; ++i) {
        get_event(timeline, i, &entry);
        if (entry.type >= type_begin && entry.type <= type_end) {
            switch (op) {
            case LT:
                if (entry.syscall_result >= param) continue;
                break;
            case LTE:
                if (entry.syscall_result > param) continue;
                break;
            case EQ:
                if (entry.syscall_result != param) continue;
                break;
            case GTE:
                if (entry.syscall_result < param) continue;
                break;
            case GT:
                if (entry.syscall_result <= param) continue;
                break;
            default:
                break;
            }

            print_event(timeline, entry, i);
            ++j;
            if (0 == (j & 31)) {
                cerr << "---Type c <return> to continue, or q <return> to quit---" << endl;
                char in;
                cin >> in;
                if ('q' == in) {
                    break;
                }
            }
        }
    }
}

static void search_event_by_tid(void* timeline, int count, int tid)
{
    EventEntry entry;

    for (int i = 0, j = 0; i<count; ++i) {
        get_event(timeline, i, &entry);
        if (entry.tid == tid) {
            print_event(timeline, entry, i);
            ++j;
            if (0 == (j & 31)) {
                cerr << "---Type c <return> to continue, or q <return> to quit---" << endl;
                char in;
                cin >> in;
                if ('q' == in) {
                    break;
                }
            }
        }
    }
}

static bool get_range(const char* range, int *begin, int* end)
{
    if (0 == range[-1] || 0 == range[0]) {
        cerr << "Miss parameter." << endl;
        return false;
    }

    char* stop = nullptr;
    *begin = strtol(range, &stop, 10);
    if (*stop > 0x20) {
        *end = strtol(stop + 1, &stop, 10);
        if (*end < *begin) *end = *begin;
    }
    else {
        *end = *begin;
    }

    return (*end >= *begin) && (*begin >= 0);
}

static bool handle_command(void* timeline, int count,
        int& current, int& verbose, const char* command)
{
    bool can_run = false;

    if (is_prefix(command, "p")) {
        /*prev*/
        if (current > 0) {
            --current;
            can_run = true;
        }
        else {
            cerr << "Reach the head!" << endl;
        }
        cerr << "Current-index:" << current << endl;
    }
    else if (is_prefix(command, "n")) {
        /*next*/
        if (current + 1 < count) {
            ++current;
            can_run = true;
        }
        else {
            cerr << "Reach the tail!" << endl;
        }
        cerr << "Current-index:" << current << endl;
    }
    else if (isdigit(command[0])) {
        int n = atoi(command);
        if (n >= 0 && n < count) {
            current = n;
            can_run = true;
        }
        else {
            cerr << "Index is not valid! max-index=" << count -1 << endl;
        }
        cerr << "Current-index:" << current << endl;
    }
    else if(is_prefix(command, "h")) {
        cerr << "All commands:"
        "\nh\t\t\t\t# show this help."
        "\np\t\t\t\t# view previous event within gdb"
        "\nn\t\t\t\t# view next event within gdb"
        "\nq\t\t\t\t# quit"
        "\nlog 0/1\t\t\t\t# 1:verbose;0:silent"
        "\nlist xxx,yyy\t\t\t# list event in range [xxx,yyy]"
        "\nsys xxx[,yyy] [ret op imm]\t# search syscall xxx or [xxx,yyy] event"
        "\n                          \t# op is one of >, >=, ==, <=, <;ret is syscall result"
        "\nsig xxx[,yyy]\t\t\t# search signal xxx or [xxx,yyy] event"
        "\nx11 xxx[,yyy]\t\t\t# search x11 xxx or [xxx,yyy] event"
        "\ndbus xxx[,yyy]\t\t\t# search dbus xxx or [xxx,yyy] event"
        "\ntid xxx       \t\t\t# list all event of thread xxx"
        "\nevent-index (0,1,2,..."
         << count - 1 <<  ")\t# view specified event within gdb" << endl;
    }
    else if(is_prefix(command, "log")) {
        verbose = (command[4] == '1');
        cerr << verbose << endl;
    }
    else if(is_prefix(command, "list")) {
        int begin = -1, end = -1;
        if (get_range(command + 5, &begin, &end)) {
            list_event(timeline, count, begin, end);
        }
    }
    else if(is_prefix(command, "sys")) {
        int begin = -1, end = -1;
        if (get_range(command + 4, &begin, &end)) {
            long value = 0;
            CompareOp op = None;
            const char* cond = strstr(command + 4, "ret ");
            if (cond) {
                op = parse_compare_op(cond + 4, &value);
            }
            search_event(timeline, count, begin, end, op, value);
        }
    }
    else if(is_prefix(command, "sig")) {
        int begin = -1, end = -1;
        if (get_range(command + 4, &begin, &end))
            search_event(timeline, count,
                begin + DUMP_REASON_signal,
                end + DUMP_REASON_signal);
    }
    else if(is_prefix(command, "x11")) {
        int begin = -1, end = -1;
        if (get_range(command + 4, &begin, &end))
            search_event(timeline, count,
                begin + DUMP_REASON_x11,
                end + DUMP_REASON_x11);
    }
    else if(is_prefix(command, "tid")) {
        int begin = -1, end = -1;
        if (get_range(command + 4, &begin, &end))
            search_event_by_tid(timeline, count, begin);
    }
    else if(is_prefix(command, "dbus")) {
        int begin = -1, end = -1;
        if (get_range(command + 5, &begin, &end))
            search_event(timeline, count,
                begin + DUMP_REASON_dbus,
                end + DUMP_REASON_dbus);
    }
    else {
        cerr << "Unknown command, press h for more help!\n";
    }

    return can_run;
}

static int find_dump(const char* parent_dir)
{
    int pid = 0;
    struct dirent *dir = nullptr;
    DIR* d = opendir(parent_dir);
    if (d) {
        int len = strlen(MAP_FILE_NAME);
        while ((dir = readdir(d)) != nullptr) {
            if (!memcmp(dir->d_name, MAP_FILE_NAME, len)) {
                pid = atoi(dir->d_name + len);
                break;
            }
        }
        closedir(d);
    }
    return pid;
}

static string get_trace_dir(const char* parent_dir, const char* trace_dir)
{
    string dir;
    if (nullptr == trace_dir) {
        string linkname = parent_dir;
        linkname += LATEST_TRACE_NAME;
        dir = parent_dir;
        int pos = dir.size();
        dir.resize(512, 0);
        int len = readlink(linkname.data(),
                (char *)dir.data() + pos, 512 - pos);
        if (len < 0) {
            fprintf(stderr, "failed to readlink %s, errno=%d\n",
                    linkname.data(), errno);
            return ("");
        }
        dir.resize(len + pos);
        assert(dir[dir.size()] == 0);
    }
    else {
        dir = trace_dir;
    }
    if ('/' != dir[dir.size()-1]) dir += '/';

    return dir;
}

int list_pid(const char* parent_dir, const char* trace_dir)
{
    string dirname = get_trace_dir(parent_dir, trace_dir);
    struct dirent *dir = nullptr;
    DIR* d = opendir(dirname.data());
    vector<int> allpid;
    if (d) {
        int len = strlen(MAP_FILE_NAME);
        while ((dir = readdir(d)) != nullptr) {
            if (!memcmp(dir->d_name, MAP_FILE_NAME, len)) {
                allpid.push_back(atoi(dir->d_name + len));
            }
        }
        closedir(d);
    }

    sort(allpid.begin(), allpid.end());

    for (const auto& i:allpid) {
        printf("\t%d\n", i);
    }

    return 0;
}

int replay(const char* parent_dir, const char* trace_dir, int pid)
{
    char maps_file[512];
    char context_file[512];
    char program_file[512];
    char core_file[512];
    void* timeline = nullptr;
    string dir = get_trace_dir(parent_dir, trace_dir);

    if (0 == pid) {
        pid = find_dump(dir.data());
        if (pid < 1) {
            fprintf(stderr, "failed to find pid in %s\n", dir.data());
            return 0;
        }
    }

    snprintf(maps_file, sizeof(maps_file),
            "%s%s%d", dir.data(), MAP_FILE_NAME, pid);
    snprintf(context_file, sizeof(context_file),
            "%s%s%d", dir.data(), CONTEXT_FILE_NAME, pid);
    snprintf(program_file, sizeof(program_file),
            "%s%s%d", dir.data(), EXEC_FILE_NAME, pid);
    get_program_file(program_file);
    fprintf(stderr, "start replay:%s\n\t%s\n\t%s\n\t%s\n",
                dir.data(), maps_file, context_file, program_file);
    int count = create_timeline(maps_file, context_file, &timeline);
    if (nullptr == timeline) {
        fprintf(stderr, "Failed to create timeline, error=%d\n", count);
        return 0;
    }

    snprintf(core_file, sizeof(core_file), "/tmp/corefile-%d.core", pid);

    int verbose = 0;
    int current = -1;
    char* line = nullptr;

    cerr << "press h for more help, event max index is:"
         << count - 1 << endl;

    while ((line = linenoise("emd> ")) != nullptr) {
        if (is_prefix(line, "quit") || is_prefix(line, "q")) {
            break;
        }

        if (handle_command(timeline, count, current, verbose, line)) {
            if (0 == generate_coredump(timeline, current, core_file, verbose)) {
                run_coredump(program_file, core_file, verbose);
            }
            else {
                cerr << "Failed to create coredump file" << endl;
            }
        }

        linenoiseHistoryAdd(line);
        linenoiseFree(line);
    }

    return 0;
}

int dump(const char* parent_dir, const char* trace_dir, int pid)
{
    char maps_file[512];
    char context_file[512];
    char program_file[512];
    void* timeline = nullptr;
    string dir = get_trace_dir(parent_dir, trace_dir);

    if (0 == pid) {
        pid = find_dump(dir.data());
        if (pid < 1) {
            fprintf(stderr, "failed to find pid in %s\n", dir.data());
            return 0;
        }
    }

    snprintf(maps_file, sizeof(maps_file),
            "%s%s%d", dir.data(), MAP_FILE_NAME, pid);
    snprintf(context_file, sizeof(context_file),
            "%s%s%d", dir.data(), CONTEXT_FILE_NAME, pid);
    snprintf(program_file, sizeof(program_file),
            "%s%s%d", dir.data(), EXEC_FILE_NAME, pid);
    get_program_file(program_file);
    fprintf(stderr, "start dump:%s\n\t%s\n\t%s\n\t%s\n",
                dir.data(), maps_file, context_file, program_file);
    int count = create_timeline(maps_file, context_file, &timeline);
    if (nullptr == timeline) {
        fprintf(stderr, "Failed to create timeline, error=%d\n", count);
        return 0;
    }

    list_event(timeline, count, 0, count - 1);

    destroy_timeline(timeline);

    return 0;
}
