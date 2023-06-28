# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import re
from tool.match.comment_delete import CommentDelete
from tool.match.matcher import Matcher
from tool.tool_config import MatchResult
from tool.util.logger import Logger

LOGGER = Logger().getlogger()
LIB_DICT = {'-lc++' : 'libstdc++.so'}

class MakefileMatcher(Matcher):
    def __init__(self, files_list, pattern, diffs, inputs_dict, progress):
        super().__init__(files_list, pattern, diffs, inputs_dict, progress)
        self.name = 'makefile'
        self.make_lib = []

    def match(self):
        LOGGER.info('Begin to match makefiles [totoal num: %s]' % len(self.file_list))
        for file_path in self.file_list:
            self.match_make_main(file_path, re.M)
        self.finally_save_result()
        self.dump_result()
        return self.scan_results

    def match_make_main(self, file_path, mode):
            try:
                with open(file_path, errors='ignore') as (file_make):
                    try:
                        contents = file_make.read()
                        contents = CommentDelete(contents, '#').delete_comment()
                        results = self.match_file_with_pattern(contents, mode)
                    except UnicodeDecodeError as err:
                        try:
                            LOGGER.error('%s decode error. Only the utf-8 format is supported. Except:%s.', file_path, err)
                            raise
                        finally:
                            err = None  
                            del err
                self.save_match_result(file_path, results)
            except FileNotFoundError as err:
                LOGGER.error('%s file not exist:%s.' % (file_path, err))

    def save_match_result(self, file_path, results):
        for result in results:
            if result[0][0:2] == '-l':
                if result[0] not in self.make_lib:
                    self.make_lib.append(result[0])
            if result[0] in self.diffs.keys():
                try:
                    self.scan_results.append(
                        MatchResult(file_path, result[1], ' ' + result[0], self.diffs[result[0]]['suggestion'][self.dest_cpu], 'makefiles')
                    )
                except:
                    LOGGER.warn("%s suggestion not supported." % self.dest_cpu)
                    pass


    def finally_save_result(self):
        for item in self.make_lib:
            name = self.adjust_lib_name(item)
            self.scan_results.append(MatchResult(name, ' ', ' ', 'check the compatibility of the library on the target platform', 'sofiles'))

    def adjust_lib_name(self, lib_name):
        if lib_name in LIB_DICT:
            return LIB_DICT.get(lib_name)
        else:
            index = lib_name.find('.so')
            if lib_name.startswith('-l'):
                lib_name = 'lib' + lib_name[2:] + '.so'
            else:
                if index == -1:
                    lib_name = 'lib' + lib_name + '.so'
        return lib_name

    def match_file_with_pattern(self, contens, mode):
        result = []
        match = re.finditer(self.pattern, contens, mode)
        for item in match:
            start_line, end_line = self.get_line_number(contens, item)
            split_list = item.group().split()
            if split_list:
                key = split_list[0]
                result.append([key, (start_line, end_line)])
        return result

    def dump_result(self):
        LOGGER.debug('match results in make matcher: ')
        for result in self.scan_results:
            LOGGER.debug('file_path: %s, code_range: %s, key: %s' % (result.file_path, result.code_range, result.key))