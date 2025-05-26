# SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import re

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
        