# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""cli entry"""
import re
import os


class WordCounter:
    namingStyleDic = {}
    __storage = ''
    __language = ''
    __workspace = ''
    CxxTokenMap = {
        "NAMESPACE": ["namespace," "namespace"],
        "STRUCT_DECL": ["struct", "declaration"],
        "CLASS_DECL": ["class", "declaration"],
        "TYPEDEF_DECL": ["type", "declaration"],
        "VAR_DECL": ["variable", "declaration"],
        "FUNCTION_DECL": ["function", "declaration"]
    }

    JavaTokenMap = {
        "CLASS_DEF": ["class", "definition"],
        "VARIABLE_DEF": ["variable", "definition"],
        "METHOD_DEF": ["method", "definition"],
        "ENUM_DEF": ["enum", "definition"],
    }

    PythonTokenMap = {
        "FunctionDef": ["function", "definition"],
        "ClassDef": ["class", "definition"],
    }

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
        languages = ["C/C++", "Python", "Java"]
        return languages

    def isLowerCamel(self, str):
        pattern = re.compile(r"[a-z]+((\d)|([A-Z0-9][a-z0-9]+))*")
        result = pattern.findall(str)
        return result

    def isUpperCamel(self, str):
        pattern = re.compile(r"[A-Z][a-z0-9]+((\d)|([A-Z0-9][a-z0-9]+))*")
        result = pattern.findall(str)
        return result

    def isSnake(self, str):
        pattern = re.compile(r"[a-z][a-z0-9_]*")
        result = pattern.findall(str)
        return result

    def nameFlag(self, name):
        flag = 3
        if self.isUpperCamel(name):
            flag = 0
        elif self.isLowerCamel(name):
            flag = 1
        elif self.isSnake(name):
            flag = 2
        return flag

    def scanFile(self, fileName, language):
        file = open(fileName, 'r')
        txtContent = file.readlines()
        lineNums = len(txtContent)
        if language == "C/C++":
            self.scanCxxFileContent(txtContent, lineNums)
        elif language == "Java":
            self.scanJavaFileContent(txtContent, lineNums)
        elif language == "Python":
            self.scanPyFileContent(txtContent, lineNums)
        else:
            return

    def scanCxxFileContent(self, txtContent, lineNums):
        i = 0
        flag = 3
        while i < lineNums:
            txt_line = txtContent[i]

            if 'kind.name=' in txt_line:
                kind_name = txt_line.split("kind.name=")[1].replace('\n', '')
            elif 'displayname=' in txt_line and kind_name in self.CxxTokenMap:
                displayname = txt_line.split("displayname=")[1].replace('\n', '')
                flag = self.nameFlag(displayname)
                if kind_name not in self.namingStyleDic.keys():
                    self.namingStyleDic[kind_name] = [0, 0, 0, 0]  # lower upper snake others
                self.namingStyleDic[kind_name][flag] += 1
            i += 1

    def scanJavaFileContent(self, txtContent, lineNums):
        i = 0
        flag = 3
        while i < lineNums:
            txt_line = txtContent[i]

            if 'identifier=' in txt_line:
                identifier = txt_line.split("identifier=")[1].replace('\n', '')
                flag = self.nameFlag(identifier)

            elif 'type=' in txt_line:
                defType = txt_line.split("type=")[1].replace('\n', '')
                if defType not in self.namingStyleDic.keys():
                    self.namingStyleDic[defType] = [0, 0, 0, 0]  # lower upper snake others
                self.namingStyleDic[defType][flag] += 1
            i += 1

    def scanPyFileContent(self, txtContent, lineNums):
        i = 0
        flag = 3
        while i < lineNums:
            txt_line = txtContent[i]

            if 'name=' in txt_line:
                name = txt_line.split("name=")[1].replace('\n', '')
                flag = self.nameFlag(name)

            elif 'type=' in txt_line:
                defType = txt_line.split("type=")[1].replace('\n', '')
                if defType not in self.namingStyleDic.keys():
                    self.namingStyleDic[defType] = [0, 0, 0, 0]  # lower upper snake others
                self.namingStyleDic[defType][flag] += 1
            i += 1

    def scanDir(self, filePath, language):
        files = os.listdir(filePath)

        for file in files:
            file_d = os.path.join(filePath, file)

            if os.path.isdir(file_d):
                self.scanDir(file_d, language)
            else:
                self.scanFile(file_d, language)

    def doWordcount(self):
        self.scanDir(self.workspace(), self.language())
        f = open(self.storage() + os.sep + ".wordcount", mode="w+")
        choose_map = {}
        if self.language() == "C/C++":
            choose_map = self.CxxTokenMap
        elif self.language() == "Java":
            choose_map = self.JavaTokenMap
        else:
            choose_map = self.PythonTokenMap

        for defType, nameList in self.namingStyleDic.items():
            total = sum(nameList)
            f.write(choose_map[defType][0] + " " + choose_map[defType][1])
            for index, item in enumerate(nameList):
                record = item / total
                self.namingStyleDic[defType][index] = format(record, '.2f')
                f.write(" " + str(self.namingStyleDic[defType][index]) + " ")
            f.write('\n')
        print(self.namingStyleDic)

