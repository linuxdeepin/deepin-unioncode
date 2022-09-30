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
#ifndef _ZSTD_WRITER_H
#define _ZSTD_WRITER_H

#include <cstdint>
#include <memory>
#include <vector>
#include <zstd.h>

class ZstdWriter {
public:
    ZstdWriter(void);
    ~ZstdWriter(void);
    int open(const char* filename, int compress_level);
    int write(const void* buf, size_t size);
    int close(void);
    bool valid(void) { return (fout != NULL);}

private:
    int flush(const void* buf, size_t size);

    FILE* fout;
    ZSTD_CStream* cstream;
    size_t pos;
    size_t total;
    size_t buffInSize;
    std::unique_ptr<char[]> buffIn;
    size_t buffOutSize;
    std::unique_ptr<char[]> buffOut;
};

#endif /*end #ifndef _ZSTD_WRITER_H*/
