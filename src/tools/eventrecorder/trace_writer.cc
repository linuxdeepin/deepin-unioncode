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
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <memory>
#include <map>
#include <string>

#include "event_man.h"
#include "trace_writer.h"
#include "easylogging++.h"

static int open_mem_file(pid_t pid)
{
    int fd = 0;
    char* tmp_path = NULL;
    if (asprintf(&tmp_path, "/proc/%i/mem", pid) != -1) {
        fd = open(tmp_path, O_RDONLY);
        free(tmp_path);
    }

    return fd;
}

TraceStream::TraceStream(void)
{
}

TraceStream::~TraceStream(void)
{
    if (mem_file > 0) close(mem_file);
}

bool TraceStream::init(pid_t pid2, int buffer_size, ZstdWriter* writer)
{
    pid = pid2;
    pread_error = 0;

    mem_file = -1;
    zstd = writer;

    buffer.resize(buffer_size);

    return true;
}

/* binary layout for one event, always 4bytes align:
 time(sizeof(struct timespec)), event_type(2byte), thread_num(2byte),
 current_tid(2byte), event_extra_size(2byte)[,event_extra_data]
 {
    tid(2byte), cpu_context_size(2byte)[, user_regs_struct+user_fpregs_struct],
    tls_size(4byte)[, tls_data]
    stack_addr(uintptr_t), stack_length(4byte)[,stack_data]
 }

 ...

 {
    tid(2byte), cpu_context_size(2byte)[, user_regs_struct+user_fpregs_struct],
    tls_size(4byte)[, tls_data]
    stack_addr(uintptr_t), stack_length(4byte)[,stack_data]
 }
 heap_count(4byte)[, [addr(uintptr_t)+size(4byte)+data], ...]
 */
int TraceStream::write(ThreadContext* block)
{
    int total = 0;
    uint16_t cpu_size = 0;
    uint32_t tls_size = 0;

    total += zstd->write(&block->tid, sizeof(block->tid));

    cpu_size = sizeof(block->regs) + sizeof(block->fpregs);
    total += zstd->write(&cpu_size, sizeof(cpu_size));
    total += zstd->write(&block->regs, sizeof(block->regs));
    total += zstd->write(&block->fpregs, sizeof(block->fpregs));

    tls_size = 0; // block->tls.size()*sizeof(struct user_desc);
    total += zstd->write(&tls_size, sizeof(tls_size));
    if (tls_size > 0) {
        total += zstd->write(&block->tls[0], tls_size);
    }

    // stack
    if (mem_file <= 0) {
        mem_file = open_mem_file(pid);
        if (mem_file <= 0) {
            // failed to read stack
            int size = 0;
            total += zstd->write(&block->stack.start,
                    sizeof(block->stack.start));
            total += zstd->write(&size, sizeof(size));
            return total;
        }
    }

    total += write(&block->stack);

    return total;
}

int TraceStream::write(void* buf, int size)
{
    return zstd->write(buf, size);
}

int TraceStream::read(uintptr_t addr, void* buf, int size)
{
    if (mem_file <= 0) {
        mem_file = open_mem_file(pid);
        if (mem_file <= 0) {
            return -1;
        }
    }

    int got = pread64(mem_file, buf, size, addr);
    if (got < size) {
        log_pread_error(errno, addr, got, size);
    }
    return got;
}

int TraceStream::write(EventHead* head, void* extra_buf)
{
    static_assert(sizeof(head->cur_time) +
            sizeof(head->reason) +
            sizeof(head->thread_num) +
            sizeof(head->current_tid) +
            sizeof(head->extra_size) == sizeof(*head),
            "struct EventHead aligned error!");

    int ret = zstd->write(head, sizeof(*head));
    if (head->extra_size > 0 && extra_buf) {
        ret += zstd->write(extra_buf, head->extra_size);
    }

    return ret;
}

int TraceStream::write(VmSegment* segment)
{
    int ret = 0;
    // save address
    assert((segment->start & 3 ) == 0);
    zstd->write(&segment->start, sizeof(segment->start));

    // save size
    int size = segment->end - segment->start;
    assert((size & 3 ) == 0);

    zstd->write(&size, sizeof(size));
    ret = size + sizeof(size) + sizeof(segment->start);

    // save data
    int chunk_bytes = buffer.size();
    uintptr_t offset = segment->start;

    while (size > 0) {
        if (size < chunk_bytes) chunk_bytes = size;

        /*FIXME: is need lock current segment memory page*/
        ssize_t got = pread64(mem_file, &buffer[0], chunk_bytes, offset);
        if (got < chunk_bytes) {
            log_pread_error(errno, offset, got, chunk_bytes);
            if (got > 0) {
                memset(&buffer[got], 0xcc, chunk_bytes - got);
                zstd->write(&buffer[0], chunk_bytes);
                memset(&buffer[0], 0xcc, got);
                offset += chunk_bytes;
                size -= chunk_bytes;
            }
            else {
                memset(&buffer[0], 0xcc, chunk_bytes);
            }
            break;
        }

        zstd->write(&buffer[0], chunk_bytes);
        offset += chunk_bytes;
        size -= chunk_bytes;
    }

    while (size > 0) {
        if (size < chunk_bytes) chunk_bytes = size;

        // fill 0xccc if pread failed
        zstd->write(&buffer[0], chunk_bytes);
        offset += chunk_bytes;
        size -= chunk_bytes;
    }

    return ret;
}

int TraceStream::log_pread_error(int error, uintptr_t offset, int got, int req)
{
    /* If an address is unmapped in the process, reading from the corresponding
     * offset in the file returns EIO (Input/output error).
     */
    if (error != 0 && got < 1) {
        ++pread_error;
        LOG(DEBUG) << "pread from " << pid
            << " at " << HEX(offset)
            << " failed, error=" << error;
    }
    else {
        LOG(DEBUG) << "pread from " << pid
            << " at " << HEX(offset)
            << " got " << got
            << " less required " << req;
    }

    return 0;
}
