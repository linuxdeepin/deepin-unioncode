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
