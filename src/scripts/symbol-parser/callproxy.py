# SPDX-FileCopyrightText: 2022 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import mimetypes
import os
from baseparser import Parser
from globallog import log


class CallProxy:
    __parsers = []
    __storage = ''
    __language = ''
    __workspace = ''

    def __init__(self):
        pass

    def workspace(self):
        return self.__workspace

    def setWorkspace(self, workspace):
        self.__workspace = workspace

    def setStorage(self, storage):
        self.__storage = storage
        if not os.path.exists(storage):
            os.mkdir(storage)

    def storage(self):
        return self.__storage

    def setLanguage(self, language):
        self.__language = language

    def language(self):
        return self.__language

    def supportLanguages(self):
        languages = []
        for ins in self.__parsers:
            languages += ins.language()
        return languages

    def addParser(self, parser):
        if isinstance(parser, Parser):
            self.__parsers.append(parser)
        else:
            log().info("Can't regiest handler for not Parser")

    def doParse(self):
        abs_files = []
        for root, dirs, files in os.walk(self.__workspace):
            for file in files:
                abs_files.append(os.path.abspath(root + os.sep + file))

        for parser in self.__parsers:
            lang_files = []
            all_file = abs_files
            # Parse according to the given order
            print(parser.language())
            if self.__language not in parser.language():
                continue
            for parser_mine in parser.mimetypes():
                for file in all_file:
                    file_mime = mimetypes.guess_type(file)[0]
                    if file_mime == parser_mine:
                        lang_files.append(file)
                for file in lang_files:
                    saved_path = self.__storage + os.sep + parser.rootName()
                    if not os.path.exists(saved_path):
                        os.mkdir(saved_path)
                    parser.doParse(saved_path, lang_files, file)
