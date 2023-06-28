// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clangparser.h"
#include "clangcursor.h"

#include <QDir>
#include <QDebug>
#include <clang-c/Index.h>
#include <linux/limits.h>

using Dict = QMap<QString, QStringList>;

CXChildVisitResult cursorVisitor(CXCursor _cursor, CXCursor _parent, CXClientData client_data);

static QString kLinesep = "\n";
static QString kRootName = "Cxx";

bool ClangParser::parse(const QString &workSpace, const QString &symbolLocation, const QString &language)
{
    QDir dir(workSpace);

    if (!dir.exists()) {
        qWarning() << "Error: " << dir << " does not exist";
        return false;
    }
    // only support c++ now.
    if (language == "C/C++") {
        QStringList filters;
        filters << "cpp" << "h" << "hpp" << "cxx" << "hxx";
        QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

        for (auto entry : entries) {
            if (entry.isFile()) {
                if (filters.contains(entry.suffix())) {
                    QString cxxSymbolDir = symbolLocation + "/" + kRootName;
                    if (!QDir().exists(cxxSymbolDir)) {
                        QDir().mkdir(cxxSymbolDir);
                    }
                    parseSingleFile(entry.absoluteFilePath(), cxxSymbolDir);
                }
            } else {
                parse(entry.absoluteFilePath(), symbolLocation, language);
            }
        }
    }
    return true;
}

bool ClangParser::parseSingleFile(const QString &filePath, const QString &symbolLocation)
{
    // Create an index object
    CXIndex index = clang_createIndex(1, 0);

    // Parse the source code
    auto stdString = filePath.toStdString();
    CXTranslationUnit translationUnit = clang_parseTranslationUnit(index, stdString.c_str(), nullptr,
                                                                  0, nullptr, 0,
                                                                  CXTranslationUnit_None);

    // Retrieve the created AST
    CXCursor rootCursor = clang_getTranslationUnitCursor(translationUnit);

    stdString = symbolLocation.toStdString();
    char *storagePtr = const_cast<char *>(stdString.c_str());
    // Visit all nodes of the AST
    clang_visitChildren(rootCursor, *cursorVisitor, storagePtr);

    // Release all resources used
    clang_disposeTranslationUnit(translationUnit);
    clang_disposeIndex(index);

    return true;
}

Dict declKindFilter()
{
    return {
        {"TranslationUnit", {"Namespace", "StructDecl", "ClassDecl", "UnionDecl", "TypdefDecl", "VarDecl", "FunctionDecl"}},
        {"StructDecl", {"StructDecl", "FieldDecl", "CxxMethod", "Constructor", "Destructor"}},
        {"UnionDecl", {"FieldDecl", "StructDecl", "FunctionDecl"}},
        {"ClassDecl", {"ClassDecl", "UnionDecl", "StructDecl", "TypedefDecl", "FieldDecl", "CxxMethod", "Constructor", "Destructor"}},
        {"Namespace", {"Namespace", "ClassDecl", "UnionDecl", "StructDecl", "TypedefDecl", "FieldDecl", "VarDecl", "CxxMethod", "FunctionDecl"}}
    };
}

Dict defKindFilter()
{
    return {
        {"ClassDecl", {"CompoundStmt"}},
        {"StructDecl", {"CompoundStmt"}},
        {"CxxMethod", {"CompoundStmt"}},
        {"Destructor", {"CompoundStmt"}},
        {"Constructor", {"CompoundStmt"}}
    };
}

Dict refKindFilter()
{
    return {
        {"Destructor", {"VarDecl"}},
        {"Constructor", {"VarDecl"}},
        {"CxxMethod", {"VarDecl"}},
        {"ClassDecl", {"FieldDecl"}},
        {"StructDecl", {"FieldDecl"}}
    };
}

QString record()
{
    return ".record";
}

QString declared()
{
    return ".declared";
}

QString definitions()
{
    return ".definitions";
}

QString reference()
{
    return ".reference";
}

QString getCursorDirName(const ClangCursor &cursor)
{
    QString findName = cursor.displayName();
    QString cursorType = cursor.typeSpelling();
    QString cursorKindName = cursor.kindName();
    QString cursorSpelling = cursor.spelling();
    if (cursorKindName == "FieldDecl") {
        findName = cursorType + " " + cursorSpelling;
    } else if (cursorKindName == "CxxMethod") {
        findName = cursorType + " " + cursorSpelling;
    } else if (cursorKindName == "Namespace") {
        findName = "Namespace " + cursorSpelling;
    } else if (cursorKindName == "StructDecl") {
        findName = "struct " + cursorSpelling;
    } else if (cursorKindName == "ClassDecl") {
        findName = "class " + cursorSpelling;
    } else if (cursorKindName == "UnionDecl") {
        findName = "union " + cursorSpelling;
    } else if (cursorKindName == "VarDecl") {
        findName = cursorType + " " + cursorSpelling;
    } else {
        // nothing to do
    }

    QString anonymous = "(anonymous)";
    if (findName.contains(anonymous)) {
        QStringList items = findName.split(anonymous);
        findName = items.join("");
    } else {
        findName = findName.replace("/", " ");
    }
    return findName;
}

QString getCursorDirPath(const QString &storage, const ClangCursor &cursor)
{
    if (cursor.isValid()) {
        QString suffixPath = "/" + getCursorDirName(cursor);
        auto parentCursor = cursor.sematicParent();
        QString parentKindName = parentCursor.kindName();
        while (parentKindName != "TranslationUnit") {
            suffixPath = "/" + getCursorDirName(parentCursor) + suffixPath;
            parentCursor = parentCursor.sematicParent();
            parentKindName = parentCursor.kindName();
        }
        return storage + suffixPath;
    }
    return storage;
}

void writeRecordFile(const QString &cursor_map_path, const ClangCursor &cursor)
{
    if (QDir().exists(cursor_map_path)) {
        if (cursor.typeKindName() == "Record") {
            QString recodeFile = cursor_map_path + "/" + record();
            QString writeLine = "kind.name=" + cursor.kindName() + "\r\n" \
                                        + "displayname=" + cursor.displayName();
            QFile file(recodeFile);
            file.open(QFile::ReadWrite);
            file.write(writeLine.toUtf8());
            file.close();
        }
    }
}

bool fileHasLine(const QString &_file, const QString &line)
{
    bool hasLine = false;
    if (QFile::exists(_file)) {
        QFile file(_file);
        file.open(QFile::ReadOnly);
        while (file.canReadLine()) {
            QString rLine = file.readLine();
            if (rLine == line) {
                hasLine = true;
                break;
            }
        }
        file.close();
    }
    return hasLine;
}

void writeDeclaredFile(const QString &cursorMapPath, const ClangCursor &cursor)
{
    if (!QDir().exists(cursorMapPath))
        return;

    QString declaredFile = cursorMapPath + "/" + declared();
    QString file;
    uint line = 0;
    uint column = 0;
    cursor.location(file, line, column);
    QString writeLine = file + ':' + \
            QString::number(line) + ':' + \
            QString::number(column) + kLinesep;

    if (!fileHasLine(declaredFile, writeLine)) {
        QFile file(declaredFile);
        file.open(QFile::Append);
        file.write(writeLine.toUtf8());
        file.close();
    }
}

void writeDefinitionsFile(const QString &cursorMapPath, const ClangCursor &cursor)
{
    if (!QDir().exists(cursorMapPath))
        return;

     QString defFile = cursorMapPath + "/" + definitions();
     QString file;
     uint line = 0;
     uint column = 0;
     cursor.location(file, line, column);
     QString writeLine = file + ':' + \
             QString::number(line) + ':' + \
             QString::number(column) + kLinesep;

     if (!fileHasLine(defFile, writeLine)) {
         QFile file(defFile);
         file.open(QFile::Append);
         file.write(writeLine.toUtf8());
         file.close();
     }
}

void writeReferenceFile(const QString &cursorMapPath, const ClangCursor &cursor)
{
    if (!QDir().exists(cursorMapPath))
        return;
     QString refFile = cursorMapPath + "/" + reference();
     QString file;
     uint line = 0;
     uint column = 0;
     cursor.location(file, line, column);
     QString write_line = file + ':' + \
             QString::number(line) + ':' + \
             QString::number(column) + kLinesep;

     if (!fileHasLine(refFile, write_line)) {
         QFile file(refFile);
         file.open(QFile::Append);
         file.write(write_line.toUtf8());
         file.close();
     }
}

QString findRecordMapPath(const QString &storage, const QString &recodeDisplayName)
{
    QDir dir(storage);
    QStringList nameFilters;
    nameFilters << record();
    QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable);

    for (auto f : files) {
        QFile file(f);
        file.open(QFile::ReadOnly);
        while (file.canReadLine()) {
            QString line = file.readLine();
            if (line.split("=").contains(recodeDisplayName))
                return storage;
        }
        file.close();
    }
    return "";
}

bool visitDeclKind(const QString &storage, const ClangCursor &cursor)
{
    QString cursorMapPath = getCursorDirPath(storage, cursor);
    QStringList cursorMapPathList = cursorMapPath.split("/");
    for (auto item : cursorMapPathList) {
        if (item.size() > NAME_MAX)
            return false;
    }
    cursorMapPathList.removeLast();
    QString cursor_parent_map_path = cursorMapPathList.join("/");
    if (!QDir(cursorMapPath).exists() && QDir(cursor_parent_map_path).exists()) {
        QDir().mkdir(cursorMapPath);
    }
    writeRecordFile(cursorMapPath, cursor);

    QString cursorFile;
    uint line = 0;
    uint column = 0;
    cursor.location(cursorFile, line, column);
    QString translationUnitFile = cursor.translationUnitSpelling();
    if (translationUnitFile == cursorFile) {
        writeDeclaredFile(cursorMapPath, cursor);
    }
    return true;
}

void visitDefKind(const QString &storage, const ClangCursor &parent)
{
    QString parentCursorMapPath = getCursorDirPath(storage, parent);
    writeDefinitionsFile(parentCursorMapPath, parent);
}

void visitRefKind(const QString &storage, const ClangCursor &cursor)
{
    if (cursor.kindName() == "Record") {
        QString refMapPath = findRecordMapPath(storage, cursor.typeSpelling());
        if (!refMapPath.isEmpty())
            writeReferenceFile(refMapPath, cursor);
    }
}

CXChildVisitResult cursorVisitor(CXCursor _cursor, CXCursor _parent, CXClientData client_data)
{
    char *storage = static_cast<char *>(client_data);
    ClangCursor cursor(_cursor);
    ClangCursor parent(_parent);
    QString kindName = cursor.kindName();
    QString spellName = cursor.spelling();

    auto declKindFilters = declKindFilter();
    auto defKindFilters = defKindFilter();
    auto refKindFilters = refKindFilter();

    QString parentKindName = parent.kindName();

    if (declKindFilters.keys().contains(parentKindName) && declKindFilters[parentKindName].contains(kindName)) {
        if(!visitDeclKind(storage, cursor))
            return CXChildVisit_Recurse;
    }

    if (defKindFilters.keys().contains(parentKindName) && defKindFilters[parentKindName].contains(kindName)) {
        visitDefKind(storage, parent);
    }

    if (refKindFilters.keys().contains(parentKindName) && refKindFilters[parentKindName].contains(kindName)) {
        visitRefKind(storage, cursor);
    }
    return CXChildVisit_Recurse;
}
