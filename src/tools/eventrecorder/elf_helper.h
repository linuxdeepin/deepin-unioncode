// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
