// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include <vector>
#include <list>

using namespace std;

typedef struct tagVariable {
    int         max_size;
    bool        is_pointer;
    string      sym_name;
}Variable;

typedef enum tagRunMode {
    DRY_RUN = 0,
    NORMAL,
    FAST,
}RunMode;

typedef struct tagDumpConfig {
    RunMode                 mode;
    bool                    dump_pthread_list;
    bool                    dump_robust_mutex_list;
    bool                    current_thread_only;
    int                     max_heap_size;  /*Disable dump stack if set to zero*/
    int                     max_stack_size; /*Disable dump stack if set to zero*/
    int                     max_param_size;
    int                     compress_level;
    int                     shared_buffer_size;
    long                    max_dump_bytes;

    string                  dump_dir;
    string                  break_function;
    list<string>            modules;
    vector<Variable>        vars;
    vector<int>             sigs;

    bool                    log_debug;
    bool                    log_to_stdout;
    bool                    log_to_file;
    int                     log_file_max_size;
    int                     log_flush_threshold;
}DumpConfig;

int load_config(DumpConfig& cfg);

#endif /* end #ifndef _CONFIG_H */
