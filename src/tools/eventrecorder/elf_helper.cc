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
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "elf_helper.h"
#include "easylogging++.h"

static void get_debug_file(const unsigned char* id, char* buffer)
{
    /*
       file /usr/lib/x86_64-linux-gnu/ld-2.24.so:
       ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked,
        BuildID[sha1]=095935d2da92389e2991f2b56d14dab9e6978696, stripped
      file /lib/debug/.build-id/09/5935d2da92389e2991f2b56d14dab9e6978696.debug:
         ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked,
         BuildID[sha1]=095935d2da92389e2991f2b56d14dab9e6978696, not stripped
    */
    char* walk = buffer;
    walk += sprintf(walk, "/lib/debug/.build-id/%02x/", id[0]);
    for (int i = 1; i<20; ++i) {
        walk += sprintf(walk, "%02x", id[i]);
    }
    strcpy(walk, ".debug");
}

SymbolFile::SymbolFile(uintptr_t base, const char* filename, bool use_dwarf)
{
    char debug_file[256];

    m_searched = false;
    m_base = base;

    auto fd = open(filename, O_RDONLY);
    if (fd <= 0) {
        m_valid = false;
        return;
    }

    try {
        m_elf = elf::elf{elf::create_mmap_loader(fd)};
    }
    catch (const std::runtime_error& e) {
        LOG(DEBUG) << " runtime_error:" << e.what()
            << ", when parse " << filename;
    }
    catch(...) {
        LOG(DEBUG) << "unknown exception when parse " << filename;
    }

    if (use_dwarf) {
        try {
            const elf::section& sec = m_elf.get_section(".debug_info");
            if (sec.valid()) {
                m_dwarf = dwarf::dwarf{dwarf::elf::create_loader(m_elf)};
            }
            else {
                // try to load from /lib/.build-id
                const elf::section& buildid = m_elf.get_section(".note.gnu.build-id");
                if (buildid.valid()) {
                    const void* data = buildid.data();
                    get_debug_file((const unsigned char*)data + 16, debug_file);
                    auto fd2 = open(debug_file, O_RDONLY);
                    if (fd2 > 0) {
                        m_elf2 = elf::elf{elf::create_mmap_loader(fd2)};

                        //FIXME: cannot read past end of DWARF section
                        m_dwarf = dwarf::dwarf{dwarf::elf::create_loader(m_elf2)};
                    }
                }
            }
        }
        catch (const std::runtime_error& e) {
            LOG(DEBUG) << " runtime_error:" << e.what()
                << ", when parse " << filename;
        }
        catch(...) {
            LOG(DEBUG) << "unknown exception when parse " << filename;
        }
    }

    if (m_elf.valid()) {
        const elf::section& sec = m_elf.get_section(".dynsym");
        if (sec.valid()) {
            m_dynsym = sec.as_symtab();
            m_valid = true;
        }
        const elf::section& sec2 = m_elf.get_section(".symtab");
        if (sec2.valid()) {
            m_symtab = sec2.as_symtab();
            m_valid = true;
        }
    }

#ifdef _DEBUG2
    LOG(DEBUG) << "SymbolFile constructor " << filename << ", base=" << HEX(base);
#endif
}

SymbolFile::~SymbolFile(void)
{
}

inline bool _sym_address(const elf::symtab& tabs, uintptr_t base,
        const char *name, unsigned long *addr, elf::stt type)
{
    for (auto sym : tabs) {
        if (sym.get_name() == name) {
            auto &d = sym.get_data();
            // TODO: can we support multiple kinds of symbol
            if (d.type() == type) {
                if (d.value > 0x3fffff) {
                    *addr = d.value;
                }
                else {
                    *addr = base + d.value;
                }
                return true;
            }
        }
    }
    return false;
}

bool SymbolFile::get_sym_address(const char *name, unsigned long *addr, elf::stt type)
{
    if (m_dynsym.valid()) {
        if (_sym_address(m_dynsym, m_base, name, addr, type)) return true;
    }
    if (m_symtab.valid()) {
        if (_sym_address(m_symtab, m_base, name, addr, type)) return true;
    }

	return false;
}
