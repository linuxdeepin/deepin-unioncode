# SPDX-FileCopyrightText: 2022 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

import os
from abc import ABC
from io import open
import subprocess

import clang.cindex
from clang.cindex import Config 

from globallog import log
from baseparser import Parser


class CxxParser(Parser, ABC):
    __absLibClangSoPath = ""

    def __init__(self):
        super(CxxParser, self).__init__()
        self.__absLibClangSoPath = self.llvmLibDir() + os.sep + 'libclang.so'
        Config.set_library_file(self.__absLibClangSoPath)
        self.__max_depth = 2
        self.__cur_depth = 0

    def doParse(self, storage, files, file):
        super(CxxParser, self).doParse(storage, files, file)
        index = clang.cindex.Index.create()
        tu = index.parse(file)
        if files.index(file) == 0:
            self.removeParseData(storage)
        self.doParseCursor(storage, files, tu.cursor)
        if files.index(file) == len(files) - 1:
            self.removeLostDir(storage)

    def language(self):
        return ['C/C++']

    def mimetypes(self):
        return ['text/x-chdr', 'text/x-c++hdr', 'text/x-csrc', 'text/x-c++src']

    def rootName(self):
        return 'Cxx'

    def llvmLibDir(self):
        """ get llvm lib save dir path
        :return: str llvm-config execute stdout path from lib directory
        """
        proc = subprocess.Popen(["llvm-config", "--libdir"], shell=False,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, env={"PATH": "/usr/bin"})
        proc.wait(1)
        line = proc.stdout.readline()
        log(__name__).info(str(line, encoding="utf-8"))
        if proc.returncode == 0:
            log(__name__).info("llvm configure lib directory:" + str(line, encoding="utf-8"))
        return str(line, encoding="utf-8").replace('\n', '')

    def declKindFilter(self):
        return {
            "TRANSLATION_UNIT": ["NAMESPACE", "STRUCT_DECL", "CLASS_DECL", "UNION_DECL", "TYPEDEF_DECL", "VAR_DECL",
                                 "FUNCTION_DECL"],
            "STRUCT_DECL": ["STRUCT_DECL", "FIELD_DECL", "CXX_METHOD", "CONSTRUCTOR", "DESTRUCTOR"],
            "UNION_DECL": ["FIELD_DECL", "STRUCT_DECL", "FUNCTION_DECL"],
            "CLASS_DECL": ["CLASS_DECL", "UNION_DECL", "STRUCT_DECL", "TYPEDEF_DECL", "FIELD_DECL", "CXX_METHOD",
                           "CONSTRUCTOR", "DESTRUCTOR"],
            "NAMESPACE": ["NAMESPACE", "CLASS_DECL", "UNION_DECL", "STRUCT_DECL", "TYPEDEF_DECL", "FIELD_DECL",
                          "VAR_DECL", "CXX_METHOD", "FUNCTION_DECL"],
        }

    def defKindFilter(self):
        return {
            "CLASS_DECL": ["COMPOUND_STMT"],
            "STRUCT_DECL": ["COMPOUND_STMT"],
            "CXX_METHOD": ["COMPOUND_STMT"],
            "DESTRUCTOR": ["COMPOUND_STMT"],
            "CONSTRUCTOR": ["COMPOUND_STMT"]
        }

    def refKindFilter(self):
        return {
            "DESTRUCTOR": ["VAR_DECL"],
            "CONSTRUCTOR": ["VAR_DECL"],
            "CXX_METHOD": ["VAR_DECL"],
            "CLASS_DECL": ["FIELD_DECL"],
            "STRUCT_DECL": ["FIELD_DECL"]
        }

    def getCursorDirName(self, cursor):
        find_name = cursor.displayname
        cursor_type = cursor.type.spelling
        cursor_kind_name = cursor.kind.name
        if cursor_kind_name == "FIELD_DECL":
            find_name = cursor_type + " " + cursor.spelling
        elif cursor_kind_name == "CXX_METHOD":
            find_name = cursor_type + " " + cursor.spelling
        elif cursor_kind_name == "NAMESPACE":
            find_name = "namespace " + cursor.spelling
        elif cursor_kind_name == "STRUCT_DECL":
            find_name = "struct " + cursor.spelling
        elif cursor_kind_name == "CLASS_DECL":
            find_name = "class " + cursor.spelling
        elif cursor_kind_name == "UNION_DECL":
            find_name = "union " + cursor.spelling
        elif cursor_kind_name == "VAR_DECL":
            find_name = cursor_type + " " + cursor.spelling
        else:
            pass
        anonymous_start = find_name.find("(anonymous")
        anonymous_end = find_name.find(")")
        if anonymous_start >= 0 and anonymous_end >= 0:
            find_name = find_name[0:anonymous_start] + find_name[anonymous_end+1:]
        else:
            find_name = find_name.replace("/", " ")
        return find_name

    def getCursorDirPath(self, storage, cursor):
        if cursor:
            # self.outCursor("current", cursor)
            suffix_path = os.sep + self.getCursorDirName(cursor)
            parent_cursor = cursor.semantic_parent
            while parent_cursor.kind.name != "TRANSLATION_UNIT":
                # self.outCursor("parent", parent_cursor)
                suffix_path = os.sep + self.getCursorDirName(parent_cursor) + suffix_path
                parent_cursor = parent_cursor.semantic_parent
            return storage + suffix_path
        else:
            return storage

    def outCursor(self, title, cursor):
        print(title)
        print("translation_unit:", cursor.translation_unit.spelling)
        print("cursor_file:", cursor.location.file.name)
        print("extent start:", cursor.extent.start)
        print("extent end:", cursor.extent.end)
        print("this_cursor:", cursor.displayname)
        print("line:", cursor.location.line,
              "col:", cursor.location.column,
              "kind.name:", cursor.kind.name,
              "displayName:", cursor.displayname,
              "type.spelling:", cursor.type.spelling,
              "type.kind.name:", cursor.type.kind.name)
        print("")

    def writeRecordFile(self, cursor_map_path, cursor):
        if os.path.exists(cursor_map_path):
            # write record type info, befor can use query from val_decl
            if cursor.type.kind.name == "RECORD":
                recode_file = cursor_map_path + os.sep + Parser.record()
                write_line = "kind.name=" + cursor.kind.name + os.linesep \
                             + "displayname=" + cursor.displayname
                f = open(recode_file, mode="w+", encoding="utf8")
                f.write(write_line)
                f.close()

    def writeDeclaredFile(self, cursor_map_path, cursor):
        if not os.path.exists(cursor_map_path):
            return
        declared_file = cursor_map_path + os.sep + self.declared()
        cursor_location = cursor.location
        write_line = cursor_location.file.name + ':' + \
                     str(cursor_location.line) + ':' + \
                     str(cursor_location.column) + os.linesep
        if not self.fileHasLine(declared_file, write_line):
            f = open(declared_file, mode="a", encoding="utf8")
            f.write(write_line)
            f.close()

    def writeDefinitionsFile(self, cursor_map_path, cursor):
        if not os.path.exists(cursor_map_path):
            return
        def_file = cursor_map_path + os.sep + self.definitions()
        cursor_location = cursor.location
        write_line = cursor_location.file.name + ':' + \
                     str(cursor_location.line) + ':' + \
                     str(cursor_location.column) + os.linesep
        if not self.fileHasLine(def_file, write_line):
            f = open(def_file, mode="a", encoding="utf8")
            f.write(write_line)
            f.close()

    def writeReferenceFile(self, record_map_path, cursor):
        if not os.path.exists(record_map_path):
            return
        ref_file = record_map_path + os.path.sep + Parser.reference()
        cursor_location = cursor.location
        write_line = cursor_location.file.name + ':' + \
                     str(cursor_location.line) + ':' + \
                     str(cursor_location.column) + os.linesep
        if not self.fileHasLine(ref_file, write_line):
            f = open(ref_file, mode="a", encoding="utf8")
            f.write(write_line)
            f.close()

    def findRecordMapPath(self, storage, recode_display_name):
        """
        :param storage: storage root path
        :param recode_display_name: record saved cursor kind display name
        :return: not file, if has record type, return record type path, or storage root path from param
        """
        for root, dirs, files in os.walk(storage):
            for file in files:
                if file == self.record():
                    record_file = root + os.path.sep + file
                    f = open(record_file, "r", encoding="utf8")
                    read_line = f.readline()
                    while read_line:
                        if recode_display_name in read_line.split("="):
                            return root
                        read_line = f.readline()
                    f.close()
        return ""

    def doParseCursor(self, storage, files, cursor):
        self.__cur_depth += 1
        abs_storage_path = os.path.abspath(storage)
        parent_cursor = cursor.semantic_parent
        translation_unit_file = cursor.translation_unit.spelling
        if parent_cursor:
            if cursor.location.file:
                cursor_file = cursor.location.file.name
                #  and cursor_file in files
                if cursor_file in files:
                    decl_kind_filter = self.declKindFilter()
                    def_kind_filter = self.defKindFilter()
                    ref_kind_filter = self.refKindFilter()

                    # declared to write
                    if parent_cursor.kind.name in decl_kind_filter.keys() \
                            and cursor.kind.name in decl_kind_filter[parent_cursor.kind.name]:
                        cursor_map_path = self.getCursorDirPath(abs_storage_path, cursor)
                        cursor_map_path_list = cursor_map_path.split(os.path.sep)
                        for one in cursor_map_path_list:
                            if len(one) > 256:
                                return
                        cursor_parent_map_path = os.path.sep.join(cursor_map_path_list[0:len(cursor_map_path_list) - 1])
                        if not os.path.exists(cursor_map_path) and os.path.exists(cursor_parent_map_path):
                            # print("mkdir ", cursor_map_path)
                            os.mkdir(cursor_map_path)
                        # write record file
                        self.writeRecordFile(cursor_map_path, cursor)

                        # write declared file
                        if translation_unit_file == cursor_file:
                            self.writeDeclaredFile(cursor_map_path, cursor)

                    # definitions to write
                    if parent_cursor.kind.name in def_kind_filter.keys() \
                            and cursor.kind.name in def_kind_filter[parent_cursor.kind.name]:
                        parent_cursor_map_path = self.getCursorDirPath(abs_storage_path, parent_cursor)
                        # if os.path.exists(parent_cursor_map_path):
                        # write definitions file
                        self.writeDefinitionsFile(parent_cursor_map_path, parent_cursor)

                    if parent_cursor.kind.name in ref_kind_filter.keys() \
                            and cursor.kind.name in ref_kind_filter[parent_cursor.kind.name]:
                        if cursor.type.kind.name == "RECORD":
                            ref_map_path = self.findRecordMapPath(storage, cursor.type.spelling)
                            if ref_map_path != "":
                                self.writeReferenceFile(ref_map_path, cursor)
                            # self.outCursor("parent_cursor", parent_cursor)
                            # self.outCursor("cursor", cursor)

        if self.__cur_depth <= self.__max_depth:
            for next_cursor in cursor.get_children():
                self.doParseCursor(storage, files, next_cursor)
        self.__cur_depth -= 1
