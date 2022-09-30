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
#ifndef _ELF_HELPER_H
#define _ELF_HELPER_H
#include <inttypes.h>
#include <libelf.h>
#include <gelf.h>
#include "dwarf/dwarf++.hh"
#include "elf/elf++.hh"

class SymbolFile {
public:
    SymbolFile(uintptr_t base, const char* filename, bool use_dwarf = false);
    ~SymbolFile(void);
    bool get_sym_address(const char *symname, unsigned long *addr, elf::stt type);

    bool            m_valid;
    bool            m_searched;
private:
    uintptr_t       m_base;

    elf::elf        m_elf;
    dwarf::dwarf    m_dwarf;
    elf::elf        m_elf2; // debug file in /lib/.build-id/xx/xxxx.debug

    // for .symtab + .dynsym
    elf::symtab     m_dynsym;
    elf::symtab     m_symtab;
};

#endif /*#ifndef _ELF_HELPER_H*/
