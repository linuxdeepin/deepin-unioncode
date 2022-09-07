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

from datetime import datetime
from enum import Enum, unique
import os
from pickle import TRUE

from tool.classify.check_file import FileClassify
from tool.match.cpp_matcher import LOGGER
from tool.match.match_factory import MatcherFactory
from tool.match.match_rules import get_asm_dict, get_c_rules, get_make_ruls
from tool.report.report_csv import CsvReport
from tool.tool_config import RulesPattern
from tool.tool_config import file_matcher


class MigrateTaskTactory:
    def __init__(self, type, progress):
        self.tasktype = type
        self.progress = progress

    def get_task_type(self):
        migrate_task = None
        if self.tasktype['src']:
            migrate_task = SrcMigrateTask(self.tasktype, self.progress, self.tasktype['src'])
        elif self.tasktype['deb']:
            # TODO(mozart):todo
            pass
        else:
            LOGGER.error('task type [%s] not supported.' % self.tasktype)
            raise Exception('task type not suppported.')
        return migrate_task

class MigrateTask:
    def __init__(self, inputs):
        self.id = datetime.now().strftime('%Y%m%d%H%M%S')
        self.inputs = inputs
        self.rules_pattern = None
        self.rules_dict = None
        self.no_makefile_list = []

    def start_scan(self): # virtual
        pass

    def scan_src(self): # virtual
        pass

    def set_rules(self, rules_pattern, rules_dict):
        self.rules_pattern = rules_pattern
        self.rules_dict = rules_dict

    def import_rules(self):
        c_pattern, c_rules_dict = get_c_rules(self.inputs['src_cpu'])
        make_pattern, make_rules_dict = get_make_ruls(self.inputs['src_cpu'])
        dest_asm_dict = get_asm_dict(self.inputs['dest_cpu'])
        rules_pattern = RulesPattern(c=c_pattern, make=make_pattern, dest_asm=dest_asm_dict)
        return rules_pattern, c_rules_dict

@unique
class ScanPhase(Enum):
    InvalidPhase = -1
    StartPhase = 0
    ImportRulePhase = 1
    DebDepend = 2

class SrcMigrateTask(MigrateTask):

    def __init__(self, inputs, progress, src_path):
        super().__init__(inputs)
        self.src_path = src_path
        self.file_type_dict = {}
        self.file_classifier = FileClassify(self.inputs)
        self.scan_results = []
        self.report_suffix = ''
        self.progress = progress

    def check_makefile(self, src_path):
        no_make_file = []
        self.progress.info = 'checking makefiles in dirs'
        have_makefile = self.file_classifier.check_source_path_makefile(src_path)
        if have_makefile:
            no_make_file.append(src_path)
        return no_make_file
            
    def start_scan(self):
        src_path = self.inputs['src']
        self.progress.start_phase(0, 99, ScanPhase.DebDepend, '') # TODO(mozart)
        self.no_makefile_list = self.check_makefile(src_path)
        if len(self.no_makefile_list):
            return self.no_makefile_list
        else:
            self.scan_src()
            self.generate_report(self.id)       
        return []

    def scan_src(self):
        self.report_suffix = os.path.split(self.src_path.rstrip('/'))[1]
        self.progress.set_info("setup files type")
        self.file_type_dict = self.file_classifier.setup_files_type(self.src_path)
        matcher_factory = MatcherFactory(self.file_type_dict, self.inputs, self.rules_pattern, self.rules_dict, self.progress)
        for file_type in self.file_type_dict.keys():
            if not len(self.file_type_dict[file_type]):
                LOGGER.info('no [%s] in path [%s] ' % (file_type, self.src_path))
                continue
            matcher_type = file_matcher[file_type]
            matcher = matcher_factory.get_matcher(matcher_type)
            if matcher:
                self.progress.set_info("%s matcher" % file_type)
                results = matcher.match()
                self.scan_results.extend(results)
            else:
                LOGGER.error("there isn't matcher for [%s]" % file_type)
        return self.scan_results
    
    def need_migrate(self):
        return TRUE if len(self.scan_results) else False

    def generate_report(self, task_id):
        files = set()
        for result in self.scan_results:
            files.add(result[0])
        if len(files):
            # LOGGER.info('save migration files info [need to be migrated files numer %s]' % len(files))
            reproter = CsvReport()
            reproter.generate(task_id, self.report_suffix, self.scan_results)

