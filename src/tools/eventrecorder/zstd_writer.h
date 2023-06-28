// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
