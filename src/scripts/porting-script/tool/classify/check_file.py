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

import subprocess
import os
import re
from collections import defaultdict
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

    def setup_files_type(self, src_path):
        LOGGER.info('Begin to Classify files in path [%s]' % src_path)
        if os.path.isdir(src_path):
            self.make_and_compiledb_main(src_path)
        return dict(self.file_type_dict)

    def make_and_compiledb_main(self, path):
        # Clang's Compilation Database generator for make-based build systems
        json_content = self.make_and_compiledb(path)
        if json_content:
            LOGGER.info('compiledb is successful, json file is [%s]' % path)
            self.classifyFileType(json_content)
        else:
            LOGGER.info(
                "compile_commands.json is None, scan files manually in this path:[%s]" % path)
            self.scan_make_file(path)

    def make_and_compiledb(self, path):
        compiledbJson = path + '/compile_commands.json'
        stdout_path = '/dev/null'
        write_make_file = './make_content.txt'
        if os.path.exists(compiledbJson):
            os.remove(compiledbJson)
        os.chdir(path)

        command = 'cd ' + path + '&&make clean'
        with open(stdout_path, 'w+') as f:
            subprocess.call(command, stdout=f,
                            stderr=subprocess.DEVNULL, shell=True)
        try:
            command = 'cd ' + path + '&&make -nkwi > make_log.txt'
            with open(write_make_file, 'w+') as fp:
                subprocess.call(command, stdout=fp,
                                stderr=subprocess.DEVNULL, shell=True)
        except IOError:
            LOGGER.info('Error: IO error')
        else:
            LOGGER.info('make log write successfully')
            fp.close()

        command = 'cd ' + path + '&&compiledb --parse make_content.txt'
        subprocess.call(command, stderr=subprocess.DEVNULL, shell=True)
        json_content = ReadJsonToDict.dict_from_path(compiledbJson)
        return json_content

    def classifyFileType(self, json_content):
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
                    elif ext.lower() in value:
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
