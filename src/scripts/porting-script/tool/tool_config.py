# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
# global config parameters

from collections import namedtuple
from enum import Enum, unique


@unique
class MatcherType(Enum):
    InvalidMatcher = -1
    CppMatcher = 0
    MakefileMatcher = 1
    HeadfileMatcher = 2


file_matcher = {'cppfiles': MatcherType.CppMatcher,
                'makefiles': MatcherType.MakefileMatcher,
                'cheaders': MatcherType.HeadfileMatcher,
                'sofiles': ''} # TODO(mozart):todo

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
    SUPPORT_CPU_ARCH = ["x86_64", "arm64", "mips64el", "sw_64", "loongarch64"]
    tool_dir = ''
    work_dir = ''
    config_dir = ''
    dirs = {}
    log_name = 'temp'
    porting_tool_db_path = ''
    build_dir = ''
