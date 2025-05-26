// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _RECORD_BLOCK_H
#define _RECORD_BLOCK_H

#include <linux/unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdint>
#include <string>
#include <vector>

#include "cpu.h"
#include "zstd_writer.h"

typedef struct tagVmSegment {
    uintptr_t   start;
    uintptr_t   end;
}VmSegment;

typedef struct tagEventHead{
    struct timespec cur_time;
    int16_t         reason;
    uint16_t        thread_num;
    uint16_t        current_tid; /*thread trigger syscall*/
    uint16_t        extra_size;
}EventHead;

typedef struct tagThreadContext{
    bool                            interrupted;
    uint16_t                        tid;
    USER_REGS                       regs;
    USER_FPREGS                     fpregs;
    VmSegment                       stack;
    std::vector<char>               tls;    /*contain errno, ...*/
}ThreadContext;

class TraceStream {
public:
    TraceStream(void);
    ~TraceStream(void);
    bool init(pid_t pid, int buffer_size, ZstdWriter* writer);
    int write(ThreadContext* block);
    int write(EventHead* head, void* extra_buf);
    int write(void* buf, int size);
    int write(VmSegment* segment);
    int read(uintptr_t addr, void* buf, int size);
    int get_pid(void) {return pid;};
    int error_count(void) {return pread_error;};

private:
    int log_pread_error(int error, uintptr_t offset, int got, int req);
    std::vector<char>               buffer;
    int                             pread_error;
    int                             pid;
    int                             mem_file;
    ZstdWriter*                     zstd;
};

#endif /*end #ifndef _RECORD_BLOCK_H*/
