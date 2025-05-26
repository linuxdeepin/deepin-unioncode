# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

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
        elif matcher_type == MatcherType.HeadfileMatcher:
            matcher = CppMatcher(self.files_type_dicts["cheaders"], self.rules_pattern.c, self.rules_dict.c,
                                 self.inputs_dict,
                                 self.asm_dict, self.progress)
            return matcher
        return None
