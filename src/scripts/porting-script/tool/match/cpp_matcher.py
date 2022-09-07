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

import re

from tool.util.logger import Logger
from tool.match.comment_delete import CommentDelete
from tool.match.matcher import Matcher
from tool.model.inline_assemble import INLINE_ASSEMBLES
from tool.model.pre_macro import MACROS_X86, MACROS_ARM, MACROS_SW_64, MACROS_MIPS
from tool.tool_config import MatchResult

LOGGER = Logger().getlogger()

# arm
arm_macro_pattern = '(%s)' % '|'.join(MACROS_ARM)
# x86
x86_macro_pattern = '(%s)' % '|'.join(MACROS_X86)
# mips
mips_macro_pattern = '(%s)' % '|'.join(MACROS_MIPS)
# sw_64
sw_64_macro_pattern = '(%s)' % '|'.join(MACROS_SW_64)

cpu_list = ['arm64', 'x86_64', 'mips64el', 'sw_64']

macro_pattern = {'x86_64': x86_macro_pattern, 'arm64': arm_macro_pattern,
                 'mips64el': mips_macro_pattern, 'sw_64': sw_64_macro_pattern}
macro_suggestion = {'x86_64': 'Suggestion: Modify those code to adapt x86_64 platform.',
                    'arm64': 'Suggestion: Modify those code to adapt AArch64 platform.',
                    'mips64el': 'Suggestion: Modify those code to adapt Mips64el platform.',
                    'sw_64': 'Suggestion: Modify those code to adapt sw_64 platform.'}


class InlineAsm:
    '''inline asm will be scaned in cpp file'''
    def __init__(self, dest_cpu, asm_dict):
        self.inline_start = re.compile('(?:(?:^|(?<=\\s))(?:asm|__asm|__asm__|_asm)(?:(?:(?:\\s+)('
                                       '?:volatile|__volatile__|_volatile_|inline|goto)(?:\\s*[({]))|(?:\\s*[({])|(?:\\s[(\\n]+)))', re.I)
        self.inline_end = re.compile('(?:^|\\s)*[)}]\\s*(?:;)?', re.I)
        self.inline_content = re.compile('(\\".*?\\")', re.I)
        self.dest_cpu = dest_cpu
        self.asm_dict = asm_dict
        try:
            self.instructions = self.asm_dict["instructions"].keys()
        except TypeError as err:
            LOGGER.error(
                'Asm json file for arch [%s] is not right, please check it in [config/ASM/*.json]' % self.dest_cpu
            )

    def judge_asm_arch(self, inline_asm_segment):
        for asm in inline_asm_segment:
            try:
                inline_asm = re.search(self.inline_content, asm)
                current_instruction = inline_asm.group().replace('"', "").split()[0]
                if current_instruction.upper() not in self.instructions:
                    LOGGER.debug("find need to migrated [%s] asm :%s" % (self.dest_cpu, asm))
                    return 'migrated'
            except (AttributeError, IndexError):
                continue
        return self.dest_cpu

    def get_asm_content(self, file_path):
        asm_segment = []
        start_line = -1
        inline_result = []
        try:
            with open(file_path, errors='ignore') as (file_desc):
                for line_num, line_content in enumerate(file_desc):
                    match = re.search(self.inline_start, line_content)
                    if match: # start
                        start_line = line_num
                        start_content = match.group()
                        LOGGER.debug("fidn asm start %s in line %s:" % (start_content, start_line))
                    if start_line != -1:
                        asm_segment.append(line_content.strip())
                        match = re.search(self.inline_end, line_content)
                        if match: # end
                            end_line = line_num
                            end_content = match.group()
                            LOGGER.debug('inline_asm end %s in line %s' % (end_content, end_line))
                            cpu_arch = self.judge_asm_arch(asm_segment) # scan asm segment
                            inline_result.append([(start_line, end_line), cpu_arch])
                            start_line = -1
                            asm_segment.clear()
        except FileNotFoundError as err:
            LOGGER.error('%s file not exist:%s.' % (file_path, err))
        return inline_result

    def match_inline_asm(self, file_path):
        LOGGER.debug('Begin to match inline asm file [%s]' % file_path)
        match_results = []
        results = self.get_asm_content(file_path)
        for result in results:
            if result[1] != self.dest_cpu:
                match_results.append(result)
        return match_results
        
class CppMatcher(Matcher):
    def __init__(self, files_list, pattern, diffs, inputs, asm_dict, progress):
        super().__init__(files_list, pattern, diffs, inputs, progress)
        self.precompile_macro_pattern = '(^((|[ \r\f\v\t]+)#)(|\\s+)(if|elif|ifdef|ifndef)\\W+)|(^((|[ \r\f\v\t]+)#)(|\\s+)(endif|else)(|\\W+))'
        self.inline_asm_in_file = False
        self.asm_dict = asm_dict

    def match(self):
        # LOGGER.info('Begin to match files [totoal num: %s]' % len(self.file_list))
        current_num = 0
        for file_path in self.file_list:
            self.match_main(file_path, re.M)
            current_num += 1
            self.progress.step_file(current_num, len(self.file_list))
        self.dump_result()
        return self.scan_results

    def match_main(self, file_path, mode):
        try:
            with open(file_path, errors='ignore') as (file_desc):
                try:
                    contents = file_desc.read()
                    # remove comment
                    contents = CommentDelete(contents, '//', CommentDelete.MULTI_COMMENT_C).delete_comment()
                    # scan pre-macro
                    contents = self.macro_preprocess(contents, file_path)
                    # scan according json file & asm inline scan
                    results = self.match_file_with_pattern(contents, mode, file_path)
                except UnicodeEncodeError as err:
                    try:
                        LOGGER.error('%s decode error. Only utf-8 format is supported. Except:%s.', file_path, err)
                        raise
                    finally:
                        err = None
                        del err
            self.save_match_result(file_path, results)
        except FileNotFoundError as err:
            LOGGER.error('%s file not exist:%s.' % (file_path, err))

    def save_match_result(self, file_path, results):
        for result in results:
            self.scan_results.append(
                MatchResult(file_path, result[1], result[0], self.diffs[result[0]]["suggestion"][self.dest_cpu], 'cppfiles')
            )
    
    def match_file_with_pattern(self, contents, mode, file_path):
        '''Match the source code according to the keyword in json or other configuration 
        files, return the matched key and start & end lines, return [[key1,(10,10)],[key2,(10,20)], [asm,(30,' '40)]]'''
        result = []
        match = re.finditer(self.pattern, contents, mode)
        for item in match:
            start_line, end_line = self.get_line_number(contents, item)
            key = item.group().split()[0]
            if key in INLINE_ASSEMBLES.keys() or key in ['__asm__(', 'asm(', '__asm(']:
                self.inline_asm_in_file = True
            else:
                result.append([key, (start_line, end_line)])
        # asm match
        if self.inline_asm_in_file:
            self.inline_asm_in_file = False
            match_results = InlineAsm(self.dest_cpu, self.asm_dict).match_inline_asm(file_path)
            for match in match_results:
                result.append(['__assembles__', match[0]])
        return result

    def get_marco_line(self, contents):
        ''''Get the line number and content of the precompiled macro #if #ifdef, etc. 
        such as [[ifdef,10,#ifdef X86_RMF),[elif,12,#elif ARM]]'''
        result = []
        match = re.finditer(self.precompile_macro_pattern, contents, re.M)
        line_list = contents.split('\n')
        for item in match:
            linenum = contents[:item.span()[0]].count('\n')
            key = re.sub('\\W', '', item.group())
            result.append([key, linenum, line_list[linenum]])
        return result

    def get_macro_type(self, pre_macro):
        if pre_macro[0] in ['if', 'elif', 'ifdef']:
            for cpu in cpu_list:
                match = re.findall(macro_pattern[cpu], pre_macro[2], re.I)
                if len(match):
                    return cpu
        return 'all'

    def macro_branch_judge(self, macro_segment):
        '''Input macro_segment: a list of code segments of #if -> #endif.
        output whether migration is required and the range of code lines.
        such as: 
        [['if', 308, '#if __x86__', 'x86'], ['elif', 310, '#elif aarch64', 'arm'], 
        ['elif', 312, #elif mips', 'mips'], ['endif', 316, '#endif', 'all']]'''
        start_line = macro_segment[0][1]
        end_line = macro_segment[len(macro_segment) - 1][1]
        need_migrate = 1
        for i in range(len(macro_segment)):
            if macro_segment[i][3] not in ['all']:
                break
            need_migrate = 0
        for i in range(len(macro_segment)):
            macro_top = macro_segment[i]
            if macro_top[0] in ['if', 'elif', 'ifdef', 'ifndef'] and macro_top[3] in [self.dest_cpu]:
                need_migrate = 0
                return start_line, end_line, need_migrate
        return start_line, end_line, need_migrate

    def macro_preprocess(self, contents, file_path):
        pre_macros = self.get_marco_line(contents)
        for pre_macro in pre_macros:
            macro_type = self.get_macro_type(pre_macro)
            pre_macro.append(macro_type)
        if len(pre_macros):
            LOGGER.debug('in file %s :pre macros %s' % (file_path, pre_macros))
        macro_segment = []
        for i in range(len(pre_macros)):
            pre_macro = pre_macros[i]
            macro_segment.append(pre_macro)
            if pre_macro[0] == 'endif':
                start_line, end_line, need_migrate = self.macro_branch_judge(macro_segment)
                macro_segment.clear()
                if need_migrate:
                    self.scan_results.append(
                        MatchResult(file_path, (start_line, end_line), '#if', macro_suggestion[self.dest_cpu],
                        'cppfiles')
                    )
        return contents

    def dump_result(self):
        LOGGER.debug('match results in cpp matcher: ')
        for result in self.scan_results:
            LOGGER.debug('file_path: %s, code_range: %s, key: %s' % (result.file_path, result.code_range, result.key))