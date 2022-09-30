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
