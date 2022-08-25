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

from soupsieve import match
from tool.match.cpp_matcher import CppMatcher
from tool.tool_config import MatcherType
from tool.match.match_rules import *
from tool.match.makefile_matcher import MakefileMatcher


class MatcherFactory:
    def __init__(self, file_type_dicts, inputs_dict, rules_pattern, rules_dict, progress):
        self.files_type_dicts = file_type_dicts
        self.inputs_dict = inputs_dict
        self.asm_dict = get_asm_dict(inputs_dict['dest_cpu'])
        self.rules_pattern = rules_pattern
        self.rules_dict = rules_dict
        self.progress = progress

    def get_matcher(self, matcher_type):
        if matcher_type == MatcherType.CppMatcher:
            matcher = CppMatcher(self.files_type_dicts['cppfiles'], self.rules_pattern.c, self.rules_dict.c,
                                 self.inputs_dict, self.asm_dict, self.progress)
            return matcher
        elif matcher_type == MatcherType.MakefileMatcher:
            matcher = MakefileMatcher(self.files_type_dicts['makefiles'], self.rules_pattern.make,
                                                            self.rules_dict.make, self.inputs_dict, self.progress)
            return matcher
        return None
