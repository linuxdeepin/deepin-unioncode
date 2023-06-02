# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from logging import root

from baseparser import Parser
from globallog import log
from syntaxtree import SyntaxTreeNode, findParent
import os


class JavaParser(Parser):
    def __init__(self):
        super(JavaParser, self).__init__()

    def getSourceFilePath(self, node: SyntaxTreeNode):
        if node.type == "COMPILATION_UNIT":
            return node.src_file_path
        else:
            return self.getSourceFilePath(node.parent)

    def defKindFilter(self):
        return ["CLASS_DEF", "VARIABLE_DEF", "METHOD_DEF", "ENUM_DEF", "ENUM_CONSTANT_DEF"]

    def getNodeRecursionString(self, node: SyntaxTreeNode):
        if not node:
            return ""
        line = ""
        for n in node.child:
            line += n.string.strip(" ")
            line += self.getNodeRecursionString(n)
        return line.strip(" ")

    def getMenuConstantDefString(self, node: SyntaxTreeNode):
        if node.type != "ENUM_CONSTANT_DEF":
            return ""

        def_line = ""
        for c in node.child:
            if c.type == "ANNOTATIONS":
                continue
            else:
                def_line += " " + c.string
        return def_line.strip(" ")

    def getMenuDefString(self, node: SyntaxTreeNode):
        if node.type != "ENUM_DEF":
            return ""

        def_line = ""
        for child in node.child:
            if child.type in ["MODIFIERS", "OBJBLOCK"]:
                continue
            else:
                def_line += " " + child.string

        return def_line.strip(" ")

    def getClassDefString(self, node: SyntaxTreeNode):
        if node.type != "CLASS_DEF":
            return ""

        def_line = ""
        for idx in range(0, len(node.child)):
            if node.child[idx].type == "OBJBLOCK":
                continue

            elif node.child[idx].type == "MODIFIERS":
                mod_line = ""
                for cc in node.child[idx].child:
                    mod_line += " " + cc.string
                mod_line.strip(" ")
                def_line += " " + mod_line
            else:
                def_line += " " + node.child[idx].string
        return def_line.strip(" ")

    def getVariableDefString(self, node: SyntaxTreeNode):
        if node.type != "VARIABLE_DEF":
            return ""

        def_line = ""
        for child in node.child:
            if child.type == "SEMI":
                continue
            if child.type == "MODIFIERS":
                continue
            elif child.type == "TYPE":
                def_line += self.getNodeRecursionString(child)
            else:
                def_line += " " + child.string
        return def_line.strip(" ")

    def getParametersString(self, node: SyntaxTreeNode):
        params_line = ""
        if node.type == "PARAMETERS":
            for c_node in node.child:
                if c_node.type == "PARAMETER_DEF":
                    for cc_node in c_node.child:
                        if cc_node.type == "MODIFIERS":
                            continue
                        elif cc_node.type == "TYPE":
                            params_line += " " + self.getNodeRecursionString(cc_node)
                        else:
                            params_line += " " + cc_node.string.strip(" ")
                else:
                    params_line += " " + c_node.string.strip(" ")
        return params_line.strip(" ")

    def getMethodDefString(self, node: SyntaxTreeNode):
        if node.type != "METHOD_DEF":
            return ""

        def_line = ""
        if node.type == "METHOD_DEF":
            for idx in range(0, len(node.child)):
                if node.child[idx].type == "MODIFIERS":
                    node_mod = ""
                    for tn in node.child[idx].child:
                        node_mod += " " + tn.string
                    node_mod.strip(" ")
                    def_line += " " + node_mod
                elif node.child[idx].type == "SLIST":
                    continue
                elif node.child[idx].type == "TYPE":
                    node_type = ""
                    for tn in node.child[idx].child:
                        node_type += " " + tn.string
                    def_line += " " + node_type.strip(" ")
                elif node.child[idx].type == "PARAMETERS":
                    def_line += self.getParametersString(node.child[idx])
                elif node.child[idx].type == "IDENT":
                    def_line += " " + node.child[idx].string
                elif node.child[idx].type in ["LPAREN", "RPAREN"]:
                    def_line += node.child[idx].string
                else:
                    def_line += " " + node.child[idx].string
        return def_line.strip(" ")

    def getNodeDirName(self, node: SyntaxTreeNode):
        if node.type == "CLASS_DEF":
            return self.getClassDefString(node)
        elif node.type == "VARIABLE_DEF":
            return self.getVariableDefString(node)
        elif node.type == "METHOD_DEF":
            return self.getMethodDefString(node)
        elif node.type == "ENUM_DEF":
            return self.getMenuDefString(node)
        elif node.type == "ENUM_CONSTANT_DEF":
            return self.getMenuConstantDefString(node)
        else:
            return ""

    def getNodeDirPath(self, storage, node: SyntaxTreeNode):
        if node:
            suffix_path = ""
            node_name = self.getNodeDirName(node)
            if node_name != "":
                suffix_path = os.sep + node_name
            parent_node = node.parent
            while parent_node.type != "COMPILATION_UNIT":
                parent_node_name = self.getNodeDirName(parent_node)
                if parent_node_name != "":
                    suffix_path = os.sep + parent_node_name + suffix_path
                parent_node = parent_node.parent
            return storage + suffix_path
        return storage

    def getNodeRecordIdEntLine(self, node: SyntaxTreeNode):
        for child in node.child:
            if child.type == "IDENT":
                return "identifier=" + child.string
        return ""

    def getNodeRecordTypeLine(self, node: SyntaxTreeNode):
        if node.type in self.defKindFilter():
            return "type=" + node.type
        return ""

    def getNodeDefinitionsLine(self, node: SyntaxTreeNode):
        for child in node.child:
            if child.type == "IDENT":
                return self.getSourceFilePath(node) \
                       + ":" + child.location[0] \
                       + ":" + child.location[1]
        return ""

    def writeDefinitionsFile(self, storage: str, ast_tree: SyntaxTreeNode):
        node = ast_tree
        for n in node.child:
            if n.type in self.defKindFilter():
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

    def writeRecordFile(self, storage: str, ast_tree: SyntaxTreeNode):
        node = ast_tree
        for n in node.child:
            if n.type in self.defKindFilter():
                node_path = self.getNodeDirPath(storage, n)
                if os.path.exists(node_path):
                    record_file_path = node_path + os.sep + self.record()
                    ident_line = self.getNodeRecordIdEntLine(n)
                    type_line = self.getNodeRecordTypeLine(n)
                    f = open(record_file_path, mode="w+", encoding="utf8")
                    if not self.fileHasLine(record_file_path, ident_line):
                        f.write(ident_line)
                        f.write("\n")
                    if not self.fileHasLine(record_file_path, type_line):
                        f.write(type_line)
                        f.write("\n")
                    f.close()
            self.writeRecordFile(storage, n)

    def writeTreeStructure(self, storage: str, ast_tree: SyntaxTreeNode):
        node = ast_tree
        for n in node.child:
            if n.type in self.defKindFilter():
                node_path = self.getNodeDirPath(storage, n)
                if not os.path.exists(node_path):
                    os.mkdir(node_path)
                    print(self.getSourceFilePath(n))
            self.writeTreeStructure(storage, n)

    def astTreeFromOut(self, ast_dump_out: str):
        e_tree = None
        lines = ast_dump_out.split("\n")
        for line in lines:
            if line != "":
                line = line.replace("\n", "")
                elms = line.split(" -> ")
                str_range = elms[1].split(" ")
                e_type = elms[0]
                e_string = str_range[0]
                e_range = str_range[1].replace("[", "").replace("]", "").split(":")
                e_types = e_type.replace("|--", "   ").replace("`--", "   ").replace("|", " ").split(" ")
                e_type = e_types[len(e_types) - 1]
                if len(e_types) == 1:
                    e_tree = SyntaxTreeNode
                    e_tree.location = e_range
                    e_tree.type = e_type
                    e_tree.string = e_string
                else:
                    prefix_len = len(e_types[0:len(e_types) - 1])
                    parent = findParent(e_tree, prefix_len)
                    if parent:
                        node = SyntaxTreeNode()
                        node.prefix_len = prefix_len
                        node.type = e_type
                        node.location = e_range
                        node.parent = parent
                        node.string = e_string
                        node.child = []
                        log(__name__).info("find parent: " + parent.string + " add child: " + e_string)
                        parent.child.append(node)
        return e_tree

    def doParse(self, storage, files, file):
        super(JavaParser, self).doParse(storage, files, file)
        p = os.popen("/usr/bin/java -jar "
                     + os.path.abspath(os.path.dirname(__file__))
                     + os.path.sep + "3rd/checkstyle-10.3.4-all.jar -t "
                     + file)
        ast_tree = self.astTreeFromOut(p.read())
        ast_tree.src_file_path = file

        if files.index(file) == 0:
            self.removeParseData(storage)

        self.writeTreeStructure(storage, ast_tree)
        self.writeRecordFile(storage, ast_tree)
        self.writeDefinitionsFile(storage, ast_tree)

        if files.index(file) == len(files) - 1:
            self.removeLostDir(storage)

    def language(self):
        return ['Java']

    def mimetypes(self):
        return ['text/x-java']

    def rootName(self):
        return 'Java'
