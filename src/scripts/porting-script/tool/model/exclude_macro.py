"""
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
*
"""

'''The files or directories that will be scaned should related to the CPU arch, if not,
ignore it.'''

COMMON_EXCLUDE_MACROS = [
    'PPC', 'PPC64', 's390', 's390x', 'powerpc'
    'itanium', 'win32', 'win64', 'intel', 'freebsd', 'sparc', 'sparc64', 'm68k',
    'hppa', 'riscv']

# x86
FROM_X86_EXCLUDE_MACROS = ['mips', 'arm', 'aarch64', 'alpha']
FROM_X86_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

# arm
FROM_ARM_EXCLUDE_MACROS = ['x86', 'mips', 'alpha']
FROM_ARM_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

# mips
FROM_MIPSEL64_EXCLUDE_MACROS = ['x86', 'arm', 'aarch64', 'alpha']
FROM_MIPSEL64_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

# alpha
FROM_ALPHA_EXCLUDE_MACROS = ['mips', 'arm', 'aarch64', 'x86']
FROM_ALPHA_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

exclude_pattern = {
    "arm64": FROM_ARM_EXCLUDE_MACROS, 
    "mips64el": FROM_MIPSEL64_EXCLUDE_MACROS,
    "x86_64": FROM_X86_EXCLUDE_MACROS, 
    "alpha": FROM_ALPHA_EXCLUDE_MACROS}
