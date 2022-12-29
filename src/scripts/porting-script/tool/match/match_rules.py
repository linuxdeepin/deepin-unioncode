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

import os

from tool.tool_config import ToolConfig
from tool.util.read_json import ReadJsonToDict
from tool.model.inline_assemble import INLINE_ASSEMBLES, INLINE_ASSEMBLES_QUALIFIERS, ASSEMBLES_DICT

asm_dicts = {"arm64": "ARM64_ASM.json", "mips64el": "MIPS64EL_ASM.json"}

def get_asm_dict(cpu_arch):
    json_path = os.path.join(ToolConfig.config_dir, 'ASM', asm_dicts[cpu_arch])
    asm_dict = ReadJsonToDict.dict_from_path(json_path)
    return asm_dict

def get_intrinsics_pattern(cpu_arch):
    json_path = os.path.join(ToolConfig.config_dir, 'GCC_8_3', 'X86_64_INTRINSICS_GCC_8_3.json')
    intrinsics = ReadJsonToDict.dict_from_path(json_path)
    keys = []
    keywords = intrinsics.keys()
    for key in keywords:
       keys.append(key)
    return ('(?<=\\W)(?:%s)(?=(?:\\s{0,}\\((?:[\\s\\S]*)\\)))' % '|'.join(keys), intrinsics)

def get_inline_assembler_pattern(cpu_arch):
    keywords = INLINE_ASSEMBLES.keys()
    qualifiers = INLINE_ASSEMBLES_QUALIFIERS.keys()
    return (
        '(?:(?:^|(?<=\\s))(?:%s)(?:(?:(?:\\s+)(?:%s)(?:\\s*[({]))|(?:\\s*[({])|(?:\\s[^(\\n]+)))' % (
            '|'.join(keywords), '|'.join(qualifiers)), ASSEMBLES_DICT)

def get_library_pattern():
    return '(?<=[\\s=])-l(?:(?:(?:[a-zA-Z0-9-_./+]+)[a-zA-Z0-9]*)|(?:\\w{1}))(?=\\s|$|;)'

all_asm_dict = {'x86_64': 'X86_64_ASM.json', 'arm64': 'ARM64_ASM.json', 'mips64el':'MIPS64EL_ASM.json', 'sw_64': 'SW_64_ASM.json'}
gcc_dict = {'x86_64': 'x86_64_GCC_OPTION.json', 'arm64': 'arm64_GCC_OPTION.json', 'mips64el':'mips64_GCC_OPTION.json', 'sw_64': 'sw_64_GCC_OPTION.json'}

def get_asm_dict(cpu_arch):
    json_path = os.path.join(ToolConfig.config_dir, 'ASM', all_asm_dict[cpu_arch])
    asm_dict = ReadJsonToDict.dict_from_path(json_path)
    return asm_dict

def merge_dicts(*dict_args):
    result = {}
    for item in dict_args:
        result.update(item)
    else:
        return result

def get_c_rules(cpu_arch):
    intrinsics_pattern, intrinsics_dict = get_intrinsics_pattern(cpu_arch)
    inline_assemble_pattern, inline_assemble_dic = get_inline_assembler_pattern(cpu_arch)
    pattern = '(%s)|(%s)' % (intrinsics_pattern, inline_assemble_pattern)
    c_dict = merge_dicts(intrinsics_dict, inline_assemble_dic)
    return pattern, c_dict

def get_gcc_options(cpu_arch):
    json_path = os.path.join(ToolConfig.config_dir, 'GCC_8_3', gcc_dict[cpu_arch])
    gcc_options = ReadJsonToDict.dict_from_path(json_path)
    keys = []
    for key, value in gcc_options.items():
        value_range = value.get('range')
        if not value_range:
            keys.append('(?:%s(?=\\s+|\\s*\\n|\\s*$|;))' % key)
        else:
            keys.append('(?:%s(?=\\s*\\=\\s*(?:%s)))' % (
                key, '|'.join(value_range)))
    else:
        if not keys:
            return (None, None)
    return ('(?<=\\W)(?:%s)' % '|'.join(keys), gcc_options)

def get_make_ruls(cpu_arch):
    library_pattern = get_library_pattern()
    gcc_options, make_dict = get_gcc_options(cpu_arch)
    pattern = '(%s)|(%s)' % (library_pattern, gcc_options)
    make_dict = merge_dicts(make_dict)
    return pattern, make_dict
