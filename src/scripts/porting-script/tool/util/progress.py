# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from collections import namedtuple

MigrateProgress = namedtuple('MigrateProgress', ['status', 'progress', 'phase', 'info'])

class Progress:
    def __init__(self):
        self.status = 0
        self.progress = 0
        self.phase = 0
        self.info = 'preparing...'
        self.base = 0
        self.max = 100
    
    def get(self):
        return self.status, self.phase, self.progress, self.info

    def set_info(self, text):
        self.info = text

    def start_phase(self, base, ulimit, phase, info):
        self.base = base 
        self.ulimit = ulimit
        self.phase = phase
        self.info = info
    
    def step_file(self, current_num, total_num):
        self.progress = self.base + current_num / total_num * (self.ulimit - self.base)
