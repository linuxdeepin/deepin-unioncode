# SPDX-FileCopyrightText: 2022 - 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

from baseparser import Parser
import esprima
import os
from syntaxtree import *

class JSAST:
    def __init__(self, filePath: str):
        super(JSAST, self).__init__()
        
        self.filePath = filePath
        self.level = 0
        self.root = None

        f = open(filePath, 'r', encoding='utf8')
        src = f.read()
        f.close()

        ast = esprima.parseScript(src, {'loc': True})
        print(ast)

        self.addRootNode(ast)
        self.traverseAst(ast.body)

    def addRootNode(self, node):
        self.root = SyntaxTreeNode()
        self.root.type = node.type
        self.root.prefix_len = self.level
        self.root.src_file_path = self.filePath
        self.root.string = ""
        self.root.parent = None
        self.root.child = []

    def addTreeNode(self, name, type, location, prefix_len):
        new_node = SyntaxTreeNode()
        new_node.string = name
        new_node.prefix_len = prefix_len
        new_node.child = []
        new_node.type = type
        new_node.location = location
        parent = findParent(self.root, self.level)
        new_node.parent = parent
        parent.child.append(new_node)

    def traverseClass(self, node):
        if node.body:
            print(node.type)
            self.traverseAst(node.body)

    def increaseLevel(self):
        self.level += 1

    def reduceLevel(self):
        self.level -= 1

    def traverseAst(self, nodes):
        self.increaseLevel()
        for node in nodes:
            if node.type == 'VariableDeclaration':
                for declaration in node.declarations:
                    print(f"Name:{declaration.id.name},Location:{declaration.loc.start.line, declaration.loc.start.column}-{declaration.loc.end.line, declaration.loc.end.column}")
                    nodeName = node.kind + ' ' + declaration.id.name
                    self.addTreeNode(nodeName, node.type, [str(declaration.loc.start.line), str(declaration.loc.start.column)], self.level)
            elif node.type == 'FunctionDeclaration':
                print(f"Name:{node.id.name},Location:{node.loc.start.line}-{node.loc.end.line}")
                functionParams = ''
                for param in node.params:
                    functionParams = functionParams + param.name + ","

                functionParams = functionParams[:-1]
                nodeName = node.id.name + ' (' + functionParams + ')'
                self.addTreeNode(nodeName, node.type, [str(node.loc.start.line), str(node.loc.start.column)], self.level)
            elif node.type == 'ClassDeclaration':
                self.addTreeNode(node.id.name, node.type, [str(node.loc.start.line), str(node.loc.start.column)], self.level)
                print(f"Name:{node.id.name},Location:{node.loc.start.line, node.loc.start.column}-{node.loc.end.line, node.loc.end.column}")
                self.addTreeNode(node.id.name, node.type, [str(node.loc.start.line), str(node.loc.start.column)], self.level)
                self.traverseClass(node.body)
            elif node.type == 'ClassBody':
                self.traverseAst(node.body)
            elif node.type == 'MethodDefinition':
                print(f"Name:{node.key.name},Location:{node.loc.start.line}-{node.loc.end.line}")
                functionParams = ''
                for param in node.value.params:
                    functionParams = functionParams + param.name + ","

                functionParams = functionParams[:-1]
                nodeName = node.key.name + ' (' + functionParams + ')'
                self.addTreeNode(nodeName, node.type, [str(node.loc.start.line), str(node.loc.start.column)], self.level)
            elif node.type == 'ExpressionStatement':
                if node.expression.type == 'CallExpression':
                    print(f"Name:{node.expression.callee.name},Location:{node.loc.start.line}-{node.loc.end.line}")
                    self.addTreeNode(node.expression.callee.name, node.type, [str(node.loc.start.line), str(node.loc.start.column)], self.level)
            else:
                print(f"Name:{node.id.name},Location:{node.loc.start.line}-{node.loc.end.line}")
        self.reduceLevel()

class JavascriptParser(Parser):
    def __init__(self):
        super(JavascriptParser, self).__init__()

    def doParse(self, storage, files, file):
        super(JavascriptParser, self).doParse(storage, files, file)
        ast_root = self.getSyntaxTree(file)

        self.writeTreeStructure(storage, ast_root)
        self.writeRecordFile(storage, ast_root)
        self.writeDefinitionsFile(storage, ast_root)

    def getSyntaxTree(self, file: str):
        jsAst = JSAST(file)
        return jsAst.root

    def writeTreeStructure(self, storage, ast_tree: SyntaxTreeNode):
        node = ast_tree
        for n in node.child:
            if n.type in self.defTypeFilter():
                node_path = self.getNodeDirPath(storage, n)
                if not os.path.exists(node_path):
                    os.mkdir(node_path)
                    print(node_path)
            self.writeTreeStructure(storage, n)

    def writeRecordFile(self, storage, ast_tree: SyntaxTreeNode):
        node = ast_tree
        for n in node.child:
            if n.type in ['FunctionDeclaration', 'ClassDeclaration']:
                node_path = self.getNodeDirPath(storage, n)
                if os.path.exists(node_path):
                    record_file_path = node_path + os.sep + self.record()
                    f = open(record_file_path, mode="w+", encoding="utf8")
                    write_line = "type=" + n.type + os.linesep + "displayname=" + n.string
                    f.write(write_line)
                    f.close()
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
    
    def getNodeDirName(self, node: SyntaxTreeNode) -> str:            
        if node.type == 'ClassDeclaration':
            return 'class ' + node.string
        else:
            return node.string

    def getNodeDirPath(self, storage, node: SyntaxTreeNode):
        if node:
            suffix_path = ""
            node_name = self.getNodeDirName(node)
            if node_name and node_name != "":
                suffix_path = os.sep + node_name
            parent_node = node.parent
            while parent_node.type != 'Program':
                parent_node_name = self.getNodeDirName(parent_node)
                if parent_node_name != "":
                    suffix_path = os.sep + parent_node_name + suffix_path
                parent_node = parent_node.parent
            return storage + suffix_path
        return storage

    def defTypeFilter(self):
        return ['VariableDeclaration', 'FunctionDeclaration', 'ClassDeclaration', 'ExpressionStatement', 'MethodDefinition']

    def language(self):
        return ['JS']

    def mimetypes(self):
        return ['application/javascript']

    def rootName(self):
        return 'JS'
    
    
    
