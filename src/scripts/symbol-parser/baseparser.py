# SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from abc import ABCMeta, abstractmethod
from globallog import log
from syntaxtree import SyntaxTreeNode, findParent
import os

class Parser:
    __metaclass = ABCMeta

    def __init__(self):
        pass

    @abstractmethod
    def doParse(self, storage, files, file):
        log(__name__).info('doParser ' + 'language:\"' + ','.join(self.language()) + '\", ' +
                               'file:\"' + file + '\", ' +
                               'storage:\"' + storage + '\"')

    @abstractmethod
    def astTreeFromOut(self, ast_dump_out: str) -> SyntaxTreeNode:
        return None

    @abstractmethod
    def language(self):
        return []

    @abstractmethod
    def mimetypes(self):
        return []

    @abstractmethod
    def rootName(self):
        return ''

    @staticmethod
    def reference():
        return '.reference'

    @staticmethod
    def declared():
        return '.declared'

    @staticmethod
    def definitions():
        return '.definitions'

    @staticmethod
    def record():
        return '.record'

    @staticmethod
    def fileHasLine(file, line):
        has_line = False
        if os.path.exists(file):
            f = open(file, mode="r", encoding="utf8")
            r_line = f.readline()
            while r_line:
                if line == r_line:
                    has_line = True
                    break
                r_line = f.readline()
            f.close()
        return has_line

    @staticmethod
    def removeParseData(storage):
        for root, dirs, files in os.walk(storage):
            for file_name in files:
                file_path = root + os.path.sep + file_name
                log(__name__).info("remove mapping file:" + os.path.abspath(file_path))
                os.remove(file_path)

    @staticmethod
    def removeLostDir(storage):
        for name in os.listdir(storage):
            path = storage + os.path.sep + name
            if os.path.isdir(path):
                if not os.listdir(path):
                    os.removedirs(path)
                    log(__name__).info("removeLostDir: " + os.path.abspath(path))
                else:
                    Parser.removeLostDir(path)
