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

from collections import namedtuple

MigrateProgress = namedtuple('MigrateProgress', ['status', 'progress', 'phase', 'info'])

class Progress:
    def __init__(self):
        self.status = 0
        self.progress = 0
        self.phase = 0
        self.info = 'has not start'
        self.base = 0
        self.max = 100
    
    def get(self):
        return self.status, self.phase, self.progress, self.info

    def start_phase(self, base, ulimit, phase, info):
        self.base = base 
        self.ulimit = ulimit
        self.phase = phase
        self.info = info
    
    def step_file(self, current_num, total_num):
        self.progress = self.base + current_num / total_num * (self.ulimit - self.base)
