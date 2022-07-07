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
# global config parameters

from collections import namedtuple
from enum import Enum, unique

MatchResult = namedtuple('MatchResult', [
    'file_path',
    'code_range',
    'key',
    'suggestion',
    'file_type'
])

RulesPattern = namedtuple('RulesPattern', [
    'c',
    'make'
])

RulesDict = namedtuple('RulesDict', [
    'c',
    'make',
    'dest_asm'
])

class ToolConfig:
    SUPPORT_CPU_ARCH= ["x86_64", "arm64", "mips64el", "alpha"]
    tool_dir = ''
    work_dir = ''
    config_dir = ''
    dirs = {}
    log_name = 'temp'
    porting_tool_db_path = ''