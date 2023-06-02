# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from abc import ABC

from baseparser import Parser
from globallog import log
import parser
import ast
import os
from syntaxtree import *


class PyAstCvt:
    def __init__(self, filePath: str):
        super(PyAstCvt, self).__init__()
        self.__filePath = filePath
        self.__stack = 0
        self.root = None
        f = open(filePath, 'r', encoding='utf8')
        src = f.read()
        f.close()
        module_tree = ast.parse(src)
        self.convert(module_tree)

    def convert(self, node):
        if not self.root:
            self.root = SyntaxTreeNode()
            self.root.type = node.__class__.__name__
            self.root.prefix_len = self.__stack
            self.root.src_file_path = self.__filePath
            self.root.string = ""
            self.root.parent = None
            self.root.child = []
        else:
            # add node to ast
            new_node = SyntaxTreeNode()
            new_node.prefix_len = self.__stack
            new_node.child = []
            new_node.type = node.__class__.__name__
            if hasattr(node, "col_offset") and hasattr(node, "lineno"):
                new_node.location = [str(getattr(node, "lineno")), str(getattr(node, "col_offset"))]
            parent = findParent(self.root, self.__stack)
            new_node.parent = parent
            parent.child.append(new_node)
        # print("n", self.__stack, " " * self.__stack, node.__class__.__name__)

        if not isinstance(node, ast.AST):
            return

        # add node child field
        self.__stack += 1
        for field in node._fields:
            val = getattr(node, field)
            parent = findParent(self.root, self.__stack)
            field_node = SyntaxTreeNode()
            field_node.child = []
            field_node.prefix_len = self.__stack
            field_node.parent = parent
            field_node.type = val.__class__.__name__
            field_node.string = ""
            parent.child.append(field_node)
            # print("f", self.__stack, " " * self.__stack, field, "=", val, type(val))
            if isinstance(val, list):  # next ast list
                field_node.type = field
                self.__stack += 1
                for one in val:
                    self.convert(one)
                self.__stack -= 1
            elif isinstance(val, ast.AST):  # next ast
                field_node.type = field
                self.__stack += 1
                self.convert(val)
                self.__stack -= 1
            else:
                field_node.string = val
        self.__stack -= 1


class PythonParser(Parser, ABC):
    def __init__(self):
        super(PythonParser, self).__init__()
        self.__depth = 0

    def getSyntaxTree(self, file: str):
        cvt = PyAstCvt(file)
        return cvt.root

    def defTypeFilter(self):
        return [ast.ClassDef.__name__, ast.FunctionDef.__name__]

    def getFuncDefArgsString(self, node: SyntaxTreeNode) -> str:
        args = []
        for arg_node in find_nodes_by_type(node, ast.arg.__name__, True):
            for str_node in find_nodes_by_type(arg_node, str.__name__, True):
                args.append(str_node.string)
        return ",".join(args)

    def getClassDefNameString(self, node: SyntaxTreeNode):
        names = find_nodes_by_type(node, str.__name__, False)
        return names[0].string

    def getFuncDefNameString(self, node: SyntaxTreeNode) -> str:
        name_nodes = find_nodes_by_type(node, str.__name__, False)
        return name_nodes[0].string

    def getFuncDefString(self, node: SyntaxTreeNode) -> str:
        return self.getFuncDefNameString(node)  # to long can mkdir + "(" + self.getFuncDefArgsString(node) + ")"

    def getClassDefString(self, node: SyntaxTreeNode) -> str:
        return "class " + self.getClassDefNameString(node)

    def getNodeDirName(self, node: SyntaxTreeNode) -> str:
        if node.type == ast.FunctionDef.__name__:
            return self.getFuncDefString(node)
        elif node.type == ast.ClassDef.__name__:
            return self.getClassDefString(node)
        else:
            return ""

    def getNodeDirPath(self, storage, node: SyntaxTreeNode):
        if node:
            suffix_path = ""
            node_name = self.getNodeDirName(node)
            if node_name != "":
                suffix_path = os.sep + node_name
            parent_node = node.parent
            while parent_node.type != ast.Module.__name__:
                parent_node_name = self.getNodeDirName(parent_node)
                if parent_node_name != "":
                    suffix_path = os.sep + parent_node_name + suffix_path
                parent_node = parent_node.parent
            return storage + suffix_path
        return storage

    def writeTreeStructure(self, storage, ast_tree: SyntaxTreeNode):
        node = ast_tree
        for n in node.child:
            if n.type in self.defTypeFilter():
                node_path = self.getNodeDirPath(storage, n)
                if not os.path.exists(node_path):
                    os.mkdir(node_path)
                    # print(node_path)
            self.writeTreeStructure(storage, n)

    def getClassDefRecordBasesString(self, node: SyntaxTreeNode):
        ids = []
        bases = find_nodes_by_type(node, "bases")
        for base in bases:
            for id_node in find_nodes_by_type(base, "str"):
                ids.append(id_node.string)
        return ",".join(ids)

    def getFuncDefDecoratorListIdsString(self, node: SyntaxTreeNode):
        ids = []
        nodes = find_nodes_by_type(node, "decorator_list")
        for child in nodes:
            for id_node in find_nodes_by_type(child, "str"):
                ids.append(id_node.string)
        return ",".join(ids)

    def getFuncDefRecordDecoratorListIdsLine(self, node: SyntaxTreeNode):
        return "decorator_list_ids=" + self.getFuncDefDecoratorListIdsString(node)

    def getClassDefRecordNameLine(self, node: SyntaxTreeNode):
        return "name=" + self.getClassDefNameString(node)

    def getFuncDefRecordNameLine(self, node: SyntaxTreeNode):
        return "name=" + self.getFuncDefNameString(node)

    def getFuncDefRecordArgsLine(self, node: SyntaxTreeNode):
        return "args=" + self.getFuncDefArgsString(node)

    def getClassDefRecordBasesLine(self, node: SyntaxTreeNode):
        return "bases=" + self.getClassDefRecordBasesString(node)

    def writeClassRecord(self, recordFilePath, node: SyntaxTreeNode):
        f = open(recordFilePath, mode="w+", encoding="utf8")
        name_line = self.getClassDefRecordNameLine(node)
        if not self.fileHasLine(recordFilePath, name_line):
            f.write(name_line)
            f.write("\n")

        bases_line = self.getClassDefRecordBasesLine(node)
        if not self.fileHasLine(recordFilePath, bases_line):
            f.write(bases_line)
            f.write("\n")

        type_line = "type=" + node.type
        if not self.fileHasLine(recordFilePath, type_line):
            f.write(type_line)
            f.write("\n")
        f.close()

    def writeFuncRecord(self, recordFilePath, node: SyntaxTreeNode):
        f = open(recordFilePath, mode="w+", encoding="utf8")

        name_line = self.getClassDefRecordNameLine(node)
        if not self.fileHasLine(recordFilePath, name_line):
            f.write(name_line)
            f.write("\n")

        args_line = self.getFuncDefRecordArgsLine(node)
        if not self.fileHasLine(recordFilePath, args_line):
            f.write(args_line)
            f.write("\n")

        decorator_list_ids_line = "decorator_list_ids" + self.getFuncDefDecoratorListIdsString(node)
        if not self.fileHasLine(recordFilePath, decorator_list_ids_line):
            f.write(decorator_list_ids_line)
            f.write("\n")

        type_line = "type=" + node.type
        if not self.fileHasLine(recordFilePath, type_line):
            f.write(type_line)
            f.write("\n")

        f.close()

    def writeRecordFile(self, storage, ast_tree: SyntaxTreeNode):
        node = ast_tree
        for n in node.child:
            if n.type in self.defTypeFilter():
                node_path = self.getNodeDirPath(storage, n)
                if os.path.exists(node_path):
                    record_file_path = node_path + os.sep + self.record()
                    if n.type == ast.ClassDef.__name__:
                        self.writeClassRecord(record_file_path, n)
                    if n.type == ast.FunctionDef.__name__:
                        self.writeFuncRecord(record_file_path, n)
            self.writeRecordFile(storage, n)

    def getNodeDefinitionsLine(self, node: SyntaxTreeNode):
        return get_source_file_path(node) \
               + ":" + node.location[0] \
               + ":" + node.location[1]

    def writeDefinitionsFile(self, storage, ast_tree: SyntaxTreeNode):
        node = ast_tree
        for n in node.child:
            if n.type in self.defTypeFilter():
                node_path = self.getNodeDirPath(storage, n)
                if os.path.exists(node_path):
                    def_file_path = node_path + os.sep + self.definitions()
                    def_line = self.getNodeDefinitionsLine(n)
                    f = open(def_file_path, mode="w+", encoding="utf8")
                    if not self.fileHasLine(def_file_path, def_line):
                        f.write(def_line)
                        f.write("\n")
                    f.close()
            self.writeDefinitionsFile(storage, n)

    def doParse(self, storage, files, file):
        super(PythonParser, self).doParse(storage, files, file)
        ast_root = self.getSyntaxTree(file)

        if files.index(file) == 0:
            self.removeParseData(storage)

        self.writeTreeStructure(storage, ast_root)
        self.writeRecordFile(storage, ast_root)
        self.writeDefinitionsFile(storage, ast_root)

        if files.index(file) == len(files) - 1:
            self.removeLostDir(storage)

    def language(self):
        return ['Python']

    def mimetypes(self):
        return ['text/x-python']

    def rootName(self):
        return 'Python'
