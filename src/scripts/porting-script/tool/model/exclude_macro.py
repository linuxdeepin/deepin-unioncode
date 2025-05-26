# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

'''The files or directories that will be scaned should related to the CPU arch, if not,
ignore it.'''

COMMON_EXCLUDE_MACROS = [
    'PPC', 'PPC64', 's390', 's390x', 'powerpc'
    'itanium', 'win32', 'win64', 'intel', 'freebsd', 'sparc', 'sparc64', 'm68k',
    'hppa', 'riscv']

# x86
FROM_X86_EXCLUDE_MACROS = ['mips', 'arm', 'aarch64', 'sw_64', 'loongarch64']
FROM_X86_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

# arm
FROM_ARM_EXCLUDE_MACROS = ['x86', 'mips', 'sw_64', 'loongarch64']
FROM_ARM_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

# mips
FROM_MIPSEL64_EXCLUDE_MACROS = ['x86', 'arm', 'aarch64', 'sw_64', 'loongarch64']
FROM_MIPSEL64_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

# sw_64
FROM_SW_64_EXCLUDE_MACROS = ['mips', 'arm', 'aarch64', 'x86', 'loongarch64']
FROM_SW_64_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

# loongarch64
FROM_LOONGARCH64_EXCLUDE_MACROS = ['mips', 'arm', 'aarch64', 'x86', 'sw_64']
FROM_LOONGARCH64_EXCLUDE_MACROS.extend(COMMON_EXCLUDE_MACROS)

exclude_pattern = {
    "arm64": FROM_ARM_EXCLUDE_MACROS, 
    "mips64el": FROM_MIPSEL64_EXCLUDE_MACROS,
    "x86_64": FROM_X86_EXCLUDE_MACROS, 
    "sw_64": FROM_SW_64_EXCLUDE_MACROS,
    "loongarch64": FROM_LOONGARCH64_EXCLUDE_MACROS}
