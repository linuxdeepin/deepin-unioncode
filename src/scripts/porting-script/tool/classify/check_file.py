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

from pickle import TRUE
import subprocess
import os
import re
from collections import defaultdict
from tool.tool_config import ToolConfig
from tool.util.read_json import ReadJsonToDict
from tool.model.exclude_macro import exclude_pattern
from tool.util.logger import Logger


LOGGER = Logger().getlogger()

POSTFIX_TYPES = {
    'cppfiles': ('.c', '.cc', '.cpp', 'c++', '.cp', '.cxx', '.ii'),
    'cheaders': ('.h', '.hh', '.hpp', '.H', '.tcc'),
    'makefiles': ('.mk', '.make', '.makefile', '.mak'),
    'sofiles': ('.so')
}

SOURCE_MAKEFILE = ['Makefile', 'makefile', 'GNUmakefile']
MAKEFILE = ['makefile.in', 'makefile.am', 'makefile',
            'nmakefile', 'makefile.mk', 'gnumakefile']


class FileClassify:
    def __init__(self, inputs) -> None: 
        self.file_type_dict = defaultdict(list)
        self.inputs = inputs
        self.headlist = []

    def setup_files_type(self, src_path):
        # LOGGER.info('Begin to Classify files in path [%s]' % src_path)
        if os.path.isdir(src_path):
            self.make_and_compiledb_main(src_path)
        return dict(self.file_type_dict)

    def classify_header_file(self):
        headers = self.find_head_file_path(self.file_type_dict['cppfiles'])
        self.file_type_dict['cheaders'].extend(headers)

    def make_and_compiledb_main(self, path):
        # Clang's Compilation Database generator for make-based build systems
        check_cmd = 'which compiledb'
        res = subprocess.Popen(check_cmd, stdout = subprocess.PIPE, shell = True)
        compiledb = res.stdout.read().decode("utf8", "ignore")
        if compiledb:
            json_content = self.make_and_compiledb(ToolConfig.build_dir)
            if json_content:
                LOGGER.info('compiledb is successful, json file is [%s]' % ToolConfig.build_dir)
                self.classify_file_type(json_content)
                self.classify_header_file()
            else:
                LOGGER.info(
                    "compiledb is failed, scan files manually in this path:[%s]" % path)
                self.scan_make_file(path)
                # self.classify_header_file()
        else:
            LOGGER.info(
                "compiledb no installed! scan files manually in this path:[%s]" % path)
            self.scan_make_file(path)
            # self.classify_header_file()

    def make_and_compiledb(self, path):
        compiledbJson = path + '/compile_commands.json'
        if os.path.exists(compiledbJson):
            os.remove(compiledbJson)
        os.chdir(path)

        command = 'cd ' + path + '&&compiledb -n make'
        subprocess.call(command, stderr = subprocess.DEVNULL, shell = True)
        json_content = ReadJsonToDict.dict_from_path(compiledbJson)
        return json_content

    def classify_file_type(self, json_content):
        for item in json_content:
            if os.path.exists(item['file']):
                full_path = os.path.abspath(item['file'])
                self.makefile_or_cpp(full_path)
            else:
                full_path = item['directory'] + '/' + item['file']
                self.makefile_or_cpp(full_path)

    def makefile_or_cpp(self, full_path):
        ''' judge extension is makefile or cpp '''
        extension = os.path.splitext(full_path)
        if extension[1] in POSTFIX_TYPES['makefiles']:
            self.file_type_dict['makefiles'].append(full_path)
        if extension[1] in POSTFIX_TYPES['cppfiles']:
            self.file_type_dict['cppfiles'].append(full_path)

    def scan_make_file(self, src_path):
        LOGGER.info('full path scan file: [%s]' % src_path)
        make_re = re.compile('make\\.\\w+', re.I)

        for (root, _, files) in os.walk(src_path):
            for filename in files:
                absolute_path = os.path.join(root, filename)
                _, ext = os.path.splitext(filename)
                if self.exclude_file(absolute_path):
                    continue

                for key, value in POSTFIX_TYPES.items():
                    if key == 'makefiles' and (
                        make_re.match(filename) or ext.lower() in value or filename.lower() in MAKEFILE):
                        self.file_type_dict[key].append(absolute_path)
                        break
                    # if key == "cheaders": 
                    #     #In order to ensure that the header file scanning is as accurate as possible, 
                    #     #the header files to be used are deduced from the c/cpp files
                    #     #follow-up comment: deduced will cost lot of time, so get header directly.
                    elif (ext.lower() in value):
                        self.file_type_dict[key].append(absolute_path)
                        break

    def exclude_file(self, file_path):
        '''exclude file in the exclude list'''
        exclude_file_pattern = '|'.join(
            exclude_pattern[self.inputs['src_cpu']])
        match = re.search(exclude_file_pattern, file_path, re.I)
        return True if match else False

    def check_source_path_makefile(self, source_path):
        if os.path.isdir(source_path):
            sub_files = os.listdir(source_path)
            for file_name in sub_files:
                absolute_path = os.path.join(source_path, file_name)
                if os.path.isfile(absolute_path) and file_name in SOURCE_MAKEFILE:
                    return True
        return False

    def find_all_include_file(self, file_type_list):
        '''find all include files accord cpp.'''
        headfilelist = []
        for file in file_type_list:
            if os.path.isfile(file):
                with open(file, errors='ignore') as f:
                    contents = f.read()
                    matches = re.finditer('(^\\s*#\\s*)include(\\s*)(")(\\s*\\S*)(")', contents, re.M)
                    for match in matches:
                        headfilelist.append(match.group(4))
        return list(set(headfilelist))

    def find_include_file_path(self, head_file_list, out):
        '''return header absolute path.'''
        headpathlist = []
        headfilepattern = '(%s)'%'|'.join(POSTFIX_TYPES["cheaders"]).replace('.', '\.')
        for filename in head_file_list:
            file,_ = os.path.splitext(filename)
            special_characters_list = ['*', '+', '.']
            for special_characters in special_characters_list:
                if special_characters in file:
                    replace_str = "\\"+special_characters
                    file = file.replace(special_characters, replace_str)
            pattern = '^\\s*\\S*' + file + headfilepattern + '\\S*'
            matches = re.finditer(pattern, out, re.M)
            for match in matches:
                if os.path.splitext(match.group(0))[1] not in POSTFIX_TYPES["cheaders"]:
                    continue
                if match.group(0) not in headpathlist:
                    headpathlist.append(match.group(0))
        return headpathlist

    def find_head_file_path(self, file_type_list, out = '', level = 0):
        '''return header absolute path list according to cpp.'''
        if out == '':
            # LOGGER.info("start find head file path.")
            cmd = 'find ' + self.inputs['src'] + ' -name \'*\''
            res = subprocess.Popen(cmd, stdout = subprocess.PIPE, shell = True)
            out = res.stdout.read().decode("utf8", "ignore")

        headlist=[]
        headfilelist = self.find_all_include_file(file_type_list)
        headlist = self.find_include_file_path(headfilelist, out)
        if headlist:
            [self.headlist.append(i) for i in headlist if i not in self.headlist]
            if level > 0:
                level -= 1
                self.find_head_file_path(headlist, out, level)
        return list(set(self.headlist))

