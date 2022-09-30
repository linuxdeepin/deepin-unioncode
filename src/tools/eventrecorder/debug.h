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
#ifndef _DEBUG_H
#define _DEBUG_H
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* ptrace_event_name(int event);
int write_mem(int pid, uintptr_t addr, const void* buf, int buf_size);
int read_mem(int pid, uintptr_t addr, void* buf, int buf_size);

// these function used by gdb console to debug dead-lock/crash in recording
int gdb_disass(int pid, void* base, int len);
int gdb_mem(int pid, void* base, int len, int group);
int gdb_reg(int tid, USER_REGS* out);
int gdb_pause(int pid);
int gdb_step(int pid);
int gdb_cont(int pid);
int gdb_bt(int tid);
int gdb_break(int tid, uintptr_t address, uint32_t* old_value);
int gdb_delete(int tid, uintptr_t address, uint32_t old_value);
int gdb_print(int pid, uintptr_t module_base, const char* filename,
        uintptr_t var_address, const char* type_name);

#ifdef __cplusplus
}
#endif

#endif

