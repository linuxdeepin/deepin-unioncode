// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _TRACE_READER
#define _TRACE_READER

#include <string>
#include <vector>
#include "event_man.h"

typedef struct tagMapsEntry{
    long                offset; /*offset in trace file*/
    double              time;  /*unit is millisecond*/
}MapsEntry;


///////////////////////////////////////////////////////////////////////////////
// TraceReader
//
class TraceReader{
public:
    TraceReader(void);
    ~TraceReader(void);
    bool Open(const char* filename, bool zstd = false);
    bool BuildEventTable(std::vector<EventEntry>* table);
    bool BuildMapsTable(std::vector<MapsEntry>* table);
    bool Seek(long offset/*relative to file begin*/);
    long Current(void) { return ftell(fin);};

    /*NOTE: block-size-field occupied 4bytes*/
    int ReadBlock(std::string& out);

    /*NOTE: no block-size-field field*/
    int ReadBlock(void* buf, int size);

    template <typename T> bool ReadValue(T* ptr) {
        return fread(ptr, 1, sizeof(T), fin) == sizeof(T);
    }

private:
    FILE* fin;
    long  fsize;
};

#endif /*end #ifndef _TRACE_READER*/
