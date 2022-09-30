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

#include "zstd_writer.h"
#include "easylogging++.h"
#include <algorithm>
#include <errno.h>
#include <assert.h>

using namespace std;

ZstdWriter::ZstdWriter(void)
{
    pos = 0;
    total = 0;
    fout = nullptr;
    cstream = nullptr;
    buffInSize = 0;
    buffOutSize = 0;
}

ZstdWriter::~ZstdWriter(void)
{
    close();
}

int ZstdWriter::open(const char* outName, int compress_level)
{
    fout = fopen(outName, "wb");
    if (NULL == fout) {
        LOG(ERROR) << "failed create file : " << outName << ", error:" << errno;
    }
    buffInSize = ZSTD_CStreamInSize();    /* can always read one full block */
    pos = 0;
    total = buffInSize*10;
    buffIn = make_unique<char[]>(total);
    buffOutSize = ZSTD_CStreamOutSize();  /* can always flush a full block */
    buffOut = make_unique<char[]>(buffOutSize);

    cstream = ZSTD_createCStream();
    if (cstream==NULL) {
        LOG(ERROR) << "ZSTD_createCStream() error";
        exit(10);
    }
    size_t const initResult = ZSTD_initCStream(cstream, compress_level);
    if (ZSTD_isError(initResult)) {
        LOG(ERROR) << "ZSTD_initCStream() error : " << ZSTD_getErrorName(initResult);
        exit(11);
    }

    return 0;
}

int ZstdWriter::write(const void* buf, size_t size)
{
    if (total < pos + size) {
        flush(buffIn.get(), pos);
        pos = 0;
    }

    if (total < size) {
        assert (0 == pos);
        flush(buf, size);
    }
    else {
        memcpy(buffIn.get() + pos, buf, size);
        pos += size;
    }

    return static_cast<int>(size);
}

int ZstdWriter::close()
{
    if (cstream) {
        flush(buffIn.get(), pos);

        ZSTD_outBuffer output = { buffOut.get(), buffOutSize, 0 };
        /* close frame */
        size_t const remainingToFlush = ZSTD_endStream(cstream, &output);
        if (remainingToFlush) {
            LOG(ERROR) << "not fully flushed";
        }
        fwrite(buffOut.get(), 1, output.pos, fout);
        fclose(fout);
        fout = nullptr;

        ZSTD_freeCStream(cstream);
        cstream = nullptr;
    }

    return 0;
}

/****************** private methods */
int ZstdWriter::flush(const void* buf, size_t size)
{
    size_t toRead = buffInSize;
    const char* walk = reinterpret_cast<const char*>(buf);
    const char* end = walk + size;

#if 0
    {
        // debug
        char name[128];
        sprintf(name, "/tmp/strace2-%d.bin", size);
        FILE* pf = fopen(name, "wb");
        if (pf) {
            fwrite(buf, 1, size, pf);
            fclose(pf);
        }
    }
#endif

    while (walk < end) {
        ZSTD_inBuffer input = {walk, 0, 0 };

        if (walk + toRead > end) input.size = (end - walk);
        else input.size = toRead;
        walk += input.size;

        while (input.pos < input.size) {
            ZSTD_outBuffer output = { buffOut.get(), buffOutSize, 0 };

            /* toRead is guaranteed to be <= ZSTD_CStreamInSize() */
            toRead = ZSTD_compressStream(cstream, &output , &input);
            if (ZSTD_isError(toRead)) {
                LOG(ERROR) << "ZSTD_compressStream() error : "
                    << ZSTD_getErrorName(toRead);
                return -1;
            }

            /* Safely handle case when `buffInSize` is manually changed to a value <
                ZSTD_CStreamInSize()*/
            if (toRead > buffInSize) toRead = buffInSize;

            /* Safely handle case when `buffInSize` is manually changed to
             * a value < ZSTD_CStreamInSize()*/
            fwrite(buffOut.get(), 1, output.pos, fout);
        }
    }

    return 0;
}

