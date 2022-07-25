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

class Matcher:
    def __init__(self, files_list, pattern, diffs, inputs_dict, progress):
        self.name = ''
        self.file_list = files_list
        self.pattern = pattern
        self.diffs = diffs
        self.scan_results = []
        self.dest_cpu = inputs_dict["dest_cpu"]
        self.sr_cpu = inputs_dict["src_cpu"]
        self.progress = progress

    def match(self):
        return self.scan_results

    @staticmethod
    def get_line_number(contents, elem):
        start_lineno = contents[:elem.start()].count('\n')
        end_lineno = contents[:elem.end()].count('\n')
        return start_lineno, end_lineno

# test
import re
if __name__ == '__main__':
    inputs = {"dest_cpu": "dest", "src_cpu": "src"}
    matcher = Matcher([], '', '', inputs, '')
    contents = 'this is first line\n this is second line'
    match = re.finditer('this', contents)
    for item in match:
        start_line, end_line = matcher.get_line_number(contents, item)
        key = item.group().split()[0]
        print ("start_line:%d" % start_line)
        print ("end_line:%d" % end_line)
        print ("key:" + key)
        print ('\n')

