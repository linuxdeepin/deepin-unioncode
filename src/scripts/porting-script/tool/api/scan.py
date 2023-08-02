# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import threading
from tool.match.match_rules import get_asm_dict, get_c_rules, get_make_ruls
from tool.migrate_task import MigrateTaskTactory
from tool.tool_config import RulesDict, RulesPattern
from tool.util.logger import Logger
from tool.util.progress import Progress


LOGGER = Logger().getlogger()

class ScanApi:
    def __init__(self, inputs):
        self.inputs = inputs
        self.event = threading.Event()
        self.event.set()
        self.no_make_list = []
        self.deb_src_dict = {}
        self.progress = Progress()
        self.migrate_task = MigrateTaskTactory(inputs, self.progress).get_task_type()

    def start_scan(self):
        self.no_make_list = self.migrate_task.start_scan()
        return self.no_make_list

    def scan_src(self):
        self.migrate_task.scan_src()

    def import_rules(self):
        c_pattern, c_rules_dict = get_c_rules(self.inputs['src_cpu'])
        make_pattern, make_rules_dict = get_make_ruls(self.inputs['src_cpu'])
        dest_asm_dict = get_asm_dict(self.inputs['dest_cpu'])
        rules_pattern = RulesPattern(c=c_pattern, make=make_pattern)
        rules_dict = RulesDict(c=c_rules_dict, make=make_rules_dict, dest_asm=dest_asm_dict)

        self.migrate_task.set_rules(rules_pattern, rules_dict)
        return rules_pattern, rules_dict
