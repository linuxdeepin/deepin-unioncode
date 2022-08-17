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

MACROS_X86 = [
    '__amd64__', '__amd64', '__x86_64__', '__x86_64', '_M_X64',
    '_M_AMD64', 'i386', '__i386', '__i386__',
    '__IA32__', '_M_I86', '_M_IX86', '__X86__', '_X86_',
    '__THW_INTEL__', '__I86__', '__INTEL__', '__386',
    '__ia64__', '_IA64', '__IA64__', '__ia64', '_M_IA64',
    '__itanium__', '__DECIMAL_BID_FORMAT__', '__MMX__',
    '__code_model_small__', '__ATOMIC_HLE_RELEASE', '__SSE2__',
    '__k8', '__SSE__', '__ATOMIC_HLE_ACQUIRE', '__SSE_MATH__',
    '__FXSR__', '__k8__', '__SSE2_MATH__','X86_64','_x86']

MACROS_ARM = [
    '__arm__', '__thumb__', '__TARGET_ARCH_ARM',
    '__TARGET_ARCH_THUMB',
    '__ARM', 'ARM64', '__M_ARM', '__M_ARMT', '__arm', '__ARM_ARCH_2__',
    '__ARM_ARCH_3__',
    '__ARM_ARCH_3M__', '__ARM_ARCH_4T__', '__TARGET_ARM_4T',
    '__ARM_ARCH_5__',
    '__ARM_ARCH_5E__',
    '__ARM_ARCH_5T__', '__ARM_ARCH_5TE__', '__ARM_ARCH_5TEJ__',
    '__ARM_ARCH_6__',
    '__ARM_ARCH_6J__', '__ARM_ARCH_6K__', '__ARM_ARCH_6Z__',
    '__ARM_ARCH_6ZK____ARM_ARCH_6T2__',
    '__ARM_ARCH_7__', '__ARM_ARCH_7A__',
    '__ARM_ARCH_7R__',
    '__ARM_ARCH_7M__', '__ARM_ARCH_7S__',
    '__GNUC_RH_RELEASE', '__FP_FAST_FMA', '__AARCH64EL__',
    '__aarch64__', '__CHAR_UNSIGNED__', '__AARCH64_CMODEL_SMALL__',
    '__FP_FAST_FMAF']

MACROS_MIPS = [
    '__mips__', 'mips', '_R3000', '_R4000', '_R5900', 
    '__mips', '__mips', '__MIPS__', '__MIPS_ISA2__', 
    '__MIPS_ISA3__', '__MIPS_ISA4__', '__MIPS_ISA4__' 
]

MACROS_ALPHA = [
    '__alpha__', '__alpha_ev__', '__alpha', 
    '_M_ALPHA'
]