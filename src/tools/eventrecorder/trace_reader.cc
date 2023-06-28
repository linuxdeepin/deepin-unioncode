// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <elf.h>
#include <link.h>
#include <unistd.h>
#include <zstd.h>
#include <iomanip>

#include "trace_writer.h"
#include "easylogging++.h"
#include "trace_reader.h"
#include "md_types.h"

using namespace std;

static double timespec2ms(const struct timespec* spec)
{
    return (spec->tv_sec*1000.0 + spec->tv_nsec/(1000.0*1000.0));
}

static long get_file_size(const char* filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

static bool decompress_file(FILE* fin, FILE* fout)
{
    bool ret = false;
    size_t buffInSize = 0;
    void*  buffIn = nullptr;
    /* Guarantee to successfully flush at least one complete compressed block in all
     * circumstances. */
    size_t buffOutSize = 0;
    void*  buffOut = nullptr;
    ZSTD_DStream* dstream = nullptr;
    size_t initResult = 0;
    size_t read = 0, toRead = 0;

    buffInSize = ZSTD_DStreamInSize();
    buffIn  = malloc(buffInSize);
    /* Guarantee to successfully flush at least one complete compressed block in all
     * circumstances. */
    buffOutSize = ZSTD_DStreamOutSize();
    buffOut = malloc(buffOutSize);

    dstream = ZSTD_createDStream();
    if (dstream==NULL) {
        LOG(ERROR) << "ZSTD_createDStream() error";
        goto error_found;
    }

    /* In more complex scenarios, a file may consist of multiple appended frames (ex : pzstd).
    *  The following example decompresses only the first frame.
    *  It is compatible with other provided streaming examples */
    initResult = ZSTD_initDStream(dstream);
    if (ZSTD_isError(initResult)) {
        LOG(ERROR) << "ZSTD_initDStream() error : "
            << ZSTD_getErrorName(initResult);
    }

    toRead = initResult;
    while ( (read = fread(buffIn, 1, toRead, fin)) ) {
        ZSTD_inBuffer input = {buffIn, read, 0};

        while (input.pos < input.size) {
            ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
            /* toRead : size of next compressed block */
            toRead = ZSTD_decompressStream(dstream, &output , &input);
            if (ZSTD_isError(toRead)) {
                LOG(ERROR) << "ZSTD_decompressStream() error : "
                    << ZSTD_getErrorName(toRead);
                goto error_found;
            }
            if (fwrite(buffOut, 1, output.pos, fout) != output.pos) {
                LOG(ERROR) << "failed to write error : " << errno;
                goto error_found;
            }
        }
    }

    ret = true;

error_found:
    if (dstream) ZSTD_freeDStream(dstream);
    if (buffIn) free(buffIn);
    if (buffOut) free(buffOut);

    return ret;
}

static FILE* decompress(const char* filename, long* fsize)
{
    char path[NAME_MAX];
    char path2[NAME_MAX];
    FILE* ret = nullptr;
    FILE* fout = nullptr;
    FILE* fin = nullptr;

    snprintf(path, sizeof(path), "/tmp/%s.dec", basename(filename));
    if (access(path, F_OK) == 0) {
        // has decompressed !
        *fsize = get_file_size(path);
        if (*fsize > 0) return fopen(path, "rb");
    }

    fin = fopen(filename, "rb");
    if (nullptr == fin) {
        LOG(ERROR) << "failed to open " << filename << ", errno=" << errno;
        goto error_found;
    }

    fout = fopen(path, "wb");
    if (nullptr == fout) {
        LOG(ERROR) << "failed to create " << path << ", errno=" << errno;
        goto error_found;
    }

    decompress_file (fin, fout);

    // test xxx.clone is exist
    snprintf(path2, sizeof(path2), "%s.clone", filename);
    if (access(path2, F_OK) == 0) {
        fclose(fin);
        fin = fopen(path2, "rb");
        if (nullptr != fin) {
            decompress_file (fin, fout);
        }
    }

error_found:
    if (fin) fclose(fin);
    if (fout) fclose(fout);

    *fsize = get_file_size(path);
    if (*fsize > 0) ret = fopen(path, "rb");

    return ret;
}

bool TraceReader::Open(const char* filename, bool zstd)
{
    if (!zstd) {
        fsize = get_file_size(filename);
        if (fsize > 0) fin = fopen(filename, "rb");
    } else {
        fin = decompress(filename, &fsize);
    }

    return fin != nullptr;
}

TraceReader::TraceReader(void)
{
    fin = nullptr;
    fsize = 0;
}

TraceReader::~TraceReader(void)
{
    fclose(fin);
    fin = nullptr;
}

int TraceReader::ReadBlock(string& out)
{
    /*NOTE: block-size-field occupied 4bytes*/
    int size = 0;
    if (!ReadValue<int>(&size)) {
        out.clear();
        return 0; //file reach end!
    }

    if (size > 0) {
        out.resize(size, '\0');
        int bytes = fread((void*)out.data(), 1, size, fin);
        return bytes;
    }

    out.clear();
    out.insert(0, "empty or invalid block size"); // return empty will cause the caller crash
    return 0;
}

int TraceReader::ReadBlock(void* buf, int size)
{
    /*NOTE: no block-size-field field*/
    int bytes = fread(buf, 1, size, fin);
    return bytes;
}

static bool update_syscall_result(std::vector<EventEntry>* table, EventEntry& event)
{
    uint16_t syscall = event.type - DUMP_REASON_syscall_exit;
    for (auto i = table->rbegin(); i != table->rend(); ++i) {
        if (event.tid == i->tid && i->type == syscall) {
            assert(i->time <= event.time);
            i->syscall_result = event.syscall_result;
            i->duration = event.time - i->time;
            LOG(DEBUG) << "    " << get_event_name(event.type)
                << " duration:" << i->duration;
            return true;
        }
    }

    LOG(ERROR) << "Not found syscall enter for tracee " << event.tid
        << " syscall=" << syscall << " at " << event.offset;
    return false;
}

bool TraceReader::BuildEventTable(std::vector<EventEntry>* table)
{
    EventEntry event;
    int size = 0;
    uint32_t heap_count = 0;
    uint32_t tid = 0;
    long stack_addr = 0;
    struct timespec event_time;

    for (;;)
    {
        event.extra_size = 0;
        event.syscall_result = 0;
        event.offset = ftell(fin);

        if (event.offset >= fsize) {
            LOG(DEBUG) << "reach end at " << event.offset << ">=" << fsize;
            break;
        }

        if (!ReadBlock(&event_time, sizeof(event_time)) ||
            (0 == event_time.tv_sec)){
            LOG(ERROR) << "invalid time stamp at "
                << event.offset << "/" << fsize;
            break; // reach end, zero bytes padding to 128K!
        }
        event.time = timespec2ms(&event_time);

        if (!ReadValue<int16_t>(&event.type) || (event.type >= DUMP_REASON_ptrace)) {
            LOG(ERROR) << "invalid event type at "
                << event.offset << "/" << fsize;
            break;
        }
        if (!ReadValue<uint16_t>(&event.thread_num)) {
            LOG(ERROR) << "invalid thread count at "
                << event.offset << "/" << fsize;
            break; // reach end, zero bytes padding to 128K!
        }
        if (!ReadValue<int>(&size)) {
            LOG(ERROR) << "invalid tid at "
                << event.offset << "/" << fsize;
            break;
        }
        event.tid = size & 0xffff;
        size >>= 16;
        if (event.type < __NR_Linux) {
            if (sizeof(long) != size) {
                LOG(ERROR) << "invalid syscall result at "
                    << event.offset << "/" << fsize;
                break;
            }

            ReadValue<long>(&event.syscall_result);
            update_syscall_result(table, event);

            LOG(DEBUG) << "  " << get_event_name(event.type)
                << " exit for tracee " << event.tid
                << " ret=" << event.syscall_result
                << " time=" << event.time
                << " at [" << event.offset
                << ", " << ftell(fin) << "]";
            continue;
        }
        else {
            // skip event extra data, e.g. DBusMessage, XEvent.
            event.extra_size = size;
            if (size == sizeof(long)) {
                // load syscall result recorded in fast mode!
                ReadValue<long>(&event.syscall_result);
            }
            else {
                fseek(fin, size, SEEK_CUR);
            }
        }

        LOG(DEBUG) << "  thread_num:" << event.thread_num;
        for (uint16_t i = 0; i < event.thread_num; ++i) {
            tid = 0;
            ReadValue<uint32_t>(&tid);

            fseek(fin, (tid>>16), SEEK_CUR);

            size = 0;
            ReadValue<int>(&size);
            if (size > 0) {
                LOG(DEBUG) << "    tls size:" << size;
                fseek(fin, size, SEEK_CUR);
            }

            // skip stack address
            ReadValue<long>(&stack_addr);
            size = 0;
            ReadValue<int>(&size);
            // skip stack data
            if (size > 0) {
                LOG(DEBUG) << "    stack size:" << size << ", addr:" << HEX(stack_addr);
                fseek(fin, size, SEEK_CUR);
            }
        }

        heap_count = 0;
        ReadValue<uint32_t>(&heap_count);
        LOG(DEBUG) << "  heap count:" << heap_count << ", at " << ftell(fin);
        for (uint32_t i = 0; i < heap_count; ++i) {
            uintptr_t addr = 0;
            ReadValue<uintptr_t>(&addr);
            size = 0;
            ReadValue<int>(&size);
            LOG(DEBUG) << "    address:" << HEX(addr) << ", size:" << size;
            // skip heap data
            fseek(fin, size, SEEK_CUR);
        }

        event.duration = 0;
        table->push_back(event);

        LOG(DEBUG) << table->size() << ":" << get_event_name(event.type)
            << " for tracee " << event.tid
           << " time=" << event.time
            << " at [" << event.offset
            << ", " << ftell(fin) << "]";
    }

    return true;
}

bool TraceReader::BuildMapsTable(std::vector<MapsEntry>* table)
{
    bool valid = false;
    int size = 0;
    MapsEntry entry;
    MDRawDebug debug;
    struct timespec entry_time;

    // see TraceSession::dump_maps, dump_proc_file, dump_auxv

    ReadBlock(&entry_time, sizeof(entry_time));
    entry.offset = ftell(fin);
    entry.time = timespec2ms(&entry_time);

    // skip auxv
    size = 0;
    ReadValue<int>(&size);
    fseek(fin, size, SEEK_CUR);

    // skip cmdline
    size = 0;
    ReadValue<int>(&size);
    fseek(fin, size, SEEK_CUR);

    // skip environ
    size = 0;
    ReadValue<int>(&size);
    fseek(fin, size, SEEK_CUR);

    // skip vdso
    size = 0;
    ReadValue<int>(&size);
    fseek(fin, size, SEEK_CUR);

    table->push_back(entry);

    for (;;)
    {
        entry.offset = ftell(fin);
        if (entry.offset >= fsize)
            break;

        ReadBlock(&entry_time, sizeof(entry_time));
        if (0 == entry_time.tv_sec) {
            break; // reach end, zero bytes padding to 128K!
        }
        entry.offset += sizeof(entry_time);
        entry.time = timespec2ms(&entry_time);

        // skip maps
        size = 0;
        ReadValue<int>(&size);
        fseek(fin, size, SEEK_CUR);

        // skip module list
        size = 0;
        ReadValue<int>(&size);
        if (size > 0) {
            fseek(fin, size * sizeof(MDRawModule), SEEK_CUR);
        }

        // skip dso link map
        size = 0;
        ReadValue<int>(&size);
        if (size == sizeof(debug)) {
            ReadBlock(&debug, sizeof(debug));
            if (debug.dso_count > 0) {
                valid = true;
                fseek(fin, debug.dso_count * sizeof(MDRawLinkMap), SEEK_CUR);
            }

            // skip dynamic data
            size = 0;
            ReadValue<int>(&size);
            fseek(fin, size, SEEK_CUR);
        }

        if (valid) {
            valid = false;
            table->push_back(entry);
        }
        else {
            LOG(DEBUG) << "ignore empty dso entry:" << entry.offset
                << "," << entry_time.tv_sec
                << " " << entry_time.tv_nsec;
        }
    }

    return true;
}

bool TraceReader::Seek(long offset)
{
    if (fin) {
        fseek(fin, offset, SEEK_SET);
    }
    return true;
}

/*end*/
