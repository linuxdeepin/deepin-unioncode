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
from importlib.abc import ExecutionLoader
import math
import re
from sys import flags

class CommentDelete:
    MULTI_COMMENT_C = ('/*', '*/')

    def __init__(self, contents, single_comment, multi_comment=None):
        self.contents = contents
        self.single_comment = single_comment
        self.multi_comment = multi_comment
        self.solve_head = 0
        self.multi_comment_begin_index = -1
        self.exclude_comments = []

    def get_pattern(self):
        if self.multi_comment:
            return '(%s)|(%s)|(%s)' % (
                self.multi_comment[0].replace('*', '\\*'),
                self.multi_comment[1].replace('*', '\\*'),
                self.single_comment
            )
        if self.single_comment == '!':
            return '(?=\\s|)(%s)(?!=)' % self.single_comment
        return '(?=\\s|)(%s)' % self.single_comment

    def solve_single_comment(self,begin):
        if self.multi_comment_begin_index != -1:
            return
        if begin > 0:
            if not self.contents[(begin - 1)].isspace():
                return
        self.exclude_comments.append(self.contents[self.solve_head:begin])
        new_line = self.contents[begin:].find('\n')
        self.solve_head = begin + new_line
        if new_line == -1:
            self.solve_head = len(self.contents)
        
    def solve_multi_comment(self, kind, begin, end):
        if not self.multi_comment:
            return
        elif kind == self.multi_comment[0]:
            self.exclude_comments.append(self.contents[self.solve_head:begin])
            self.multi_comment_begin_index = begin
        else:
            if kind == self.multi_comment[1]:
                keep_newline = re.sub('[^\\n]', '', self.contents[self.multi_comment_begin_index:end])
                self.solve_head = end
                self.exclude_comments.append(keep_newline)
                self.multi_comment_begin_index = -1

    def delete_comment(self):
        pattern = self.get_pattern()
        matches = re.finditer(pattern, self.contents, re.M)
        for match in matches:
            kind = match.group(0)
            begin = match.start()
            if begin < self.solve_head:
                pass
            elif kind == self.single_comment:
                self.solve_single_comment(begin)
            else:
                self.solve_multi_comment(kind, begin, match.end())
        else:
            self.exclude_comments.append(self.contents[self.solve_head:])
            exclude_comments = ''.join(self.exclude_comments)
            return exclude_comments

if __name__ == "__main__":
    content = "/*multi- \n comment */ here is code! // single-comment"
    print ("string: " + content)

    comment = CommentDelete(content, "//", CommentDelete.MULTI_COMMENT_C)
    pattern = comment.get_pattern()
    print ("pattern: " + pattern)

    # test match
    result = re.match(pattern, content)
    print ("test match:")
    print (result)

    # test search
    result = re.search(pattern, content)
    print ("test search: ")
    print (result)

    # test findallall
    result = re.findall(pattern, content)
    print ("test findall: ")
    print (result)

    # test delete comment
    result = comment.delete_comment()
    print ("test delete comment:")
    print (result)
        