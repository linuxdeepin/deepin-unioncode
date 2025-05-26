# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

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

