/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
*/
#include "symbolcppasynparser.h"

#include "services/project/projectservice.h"

#include "common/common.h"

#include <QDir>
#include <QDirIterator>
#include <QMimeDatabase>
#include <QSet>
#include <QMutex>
#include <QReadLocker>
#include <QWriteLocker>
#include <QApplication>

#include <memory>
#include <iostream>

namespace {
QSharedPointer<QReadWriteLock> rwLock {new QReadWriteLock};
static bool canRunAllThread = true;
}

std::ostream& operator<<(std::ostream& stream, const CXString& str)
{
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

QDebug operator<< (QDebug& out, const CXString& str)
{
    out << clang_getCString(str);
    clang_disposeString(str);
    return out;
}

typedef struct{
    QStandardItem *root;
    QSet<QString> files;
    // first parent, secend is child kind
    QHash<CXCursorKind, QSet<CXCursorKind>> kindFilter;
}VisitData;

auto getDefaultKindFilter()
{
    QHash<CXCursorKind, QSet<CXCursorKind>> result;
    result[CXCursor_TranslationUnit] = {
        CXCursor_StructDecl,
        CXCursor_ClassDecl,
        CXCursor_UnionDecl,
        CXCursor_TypedefDecl,
        CXCursor_FunctionDecl,
    };

    result[CXCursor_StructDecl] = {
        CXCursor_StructDecl,
        CXCursor_FieldDecl
    };

    result[CXCursor_UnionDecl] = {
        CXCursor_FieldDecl
    };

    result[CXCursor_ClassDecl] = {
        CXCursor_ClassDecl,
        CXCursor_UnionDecl,
        CXCursor_StructDecl,
        CXCursor_TypedefDecl,
        CXCursor_FieldDecl,
        CXCursor_CXXMethod,
    };

    result[CXCursor_Namespace] = {
        CXCursor_Namespace,
        CXCursor_ClassDecl,
        CXCursor_UnionDecl,
        CXCursor_StructDecl,
        CXCursor_TypedefDecl,
        CXCursor_FieldDecl,
        CXCursor_CXXMethod,
    };

    return result;
}

struct QStr : public QString
{
    QStr(CXString cxStr) : QString(clang_getCString(cxStr)){}
    QStr(CXCursorKind cxKind) : QStr(clang_getCursorKindSpelling(cxKind)){}
    QStr(CXType cxType) : QStr(clang_getTypeSpelling(cxType)){}
    QStr(CXCursor cxCursor) : QStr(clang_getCursorSpelling(cxCursor)){}
    QStr(CXFile cxFile) : QStr(clang_getFileName(cxFile)){}
};

libClang::KindInfo clang_getCursorCustomKindInfo(CXCursor cursor)
{
    return {
        QStr(clang_getCursorKind(cursor)),
                QStr(clang_getCursorType(cursor)),
                QStr(cursor)
    };
}

libClang::Location clang_getCursorCustomLocation(CXCursor cursor)
{
    CXFile file;
    unsigned line, column, offset;
    auto location = clang_getCursorLocation(cursor);
    clang_getSpellingLocation(location, &file, &line, &column, &offset);
    // Location(const QString &filePath, int line, int column, int offset)
    return {
        QStr(clang_getFileName(file)),
                (int)line,
                (int)column,
                (int)offset
    };
}

dpfservice::SymbolInfo clang_getCursorCustomSymbolInfo(CXCursor cursor)
{
    return {
        clang_getCursorCustomKindInfo(cursor),
        { clang_getCursorCustomLocation(cursor) }
    };
}

QString clang_getCursorLocationSeplling(CXCursor cursor) {
    CXFile file;
    unsigned line, column, offset;
    auto location = clang_getCursorLocation(cursor);
    clang_getSpellingLocation(location, &file, &line, &column, &offset);
    QString result;
    result += "{\n";
    result += "  fileName: " + QStr(clang_getFileName(file)) + "\n";
    result += "  line: " + QString::number(line) + "\n";
    result += "  column: " + QString::number(column) + "\n";
    result += "  offset: " + QString::number(offset) + "\n";
    result += "}";
    return result;
}

QString clang_getCursorLocationFileNameSeplling(CXCursor cursor) {
    CXFile file;
    unsigned line, column, offset;
    auto location = clang_getCursorLocation(cursor);
    clang_getSpellingLocation(location, &file, &line, &column, &offset);
    QString result(clang_getCString(clang_getFileName(file)));
    return result;
}

QStandardItem *findItem(QStandardItem *root, const dpfservice::SymbolInfo &findSymInfo)
{
    if (!root)
        return nullptr;

    using namespace dpfservice;
    using namespace libClang;
    if (SymbolInfo::get(root).getKindInfo() == findSymInfo.getKindInfo())
        return root;

    for (int row = 0; row < root->rowCount(); row ++) {
        auto childItem = root->child(row);
        if (!childItem) {
            return nullptr;
        } else if (SymbolInfo::get(childItem).getKindInfo() == findSymInfo.getKindInfo()) {
            return childItem;
        } else if (childItem->hasChildren()) { // find children
            auto item = findItem(childItem, findSymInfo);
            if (item)
                return item;
        } else {
            continue;
        }
    }

    return nullptr;
}

CXChildVisitResult parseVisitChildren(CXCursor current, CXCursor parent, CXClientData client_data)
{
    if (!canRunAllThread) {
        return CXChildVisit_Break;
    }

    using namespace dpfservice;
    using namespace libClang;

    VisitData *iRoot = static_cast<VisitData*>(client_data);

    auto rootItem = iRoot->root;
    if (!iRoot->files.contains(clang_getCursorLocationFileNameSeplling(parent))) {
        return CXChildVisit_Recurse;
    }

    auto parentKind = clang_getCursorKind(parent);
    auto currentKind = clang_getCursorKind(current);
    if (iRoot->kindFilter.keys().contains(parentKind) &&
            iRoot->kindFilter[parentKind].contains(currentKind)) {

        auto parentSymInfo = clang_getCursorCustomSymbolInfo(parent);

        // read lock
        rwLock->lockForRead();
        auto parentItem = findItem(rootItem, parentSymInfo);
        rwLock->unlock();
        // read unlock


        // write lock
        rwLock->lockForWrite();
        if (!parentItem) {
            parentItem = new QStandardItem(parentSymInfo.getValue() + " " + parentSymInfo.getType());
            SymbolInfo::set(parentItem, parentSymInfo);

            int row = 0;
            int rowCount = rootItem->rowCount();
            for ( ; row < rowCount; row ++) {
                if (parentSymInfo.getValue().toUpper() < SymbolInfo::get(rootItem->child(row)).getValue().toUpper()) {
                    rootItem->insertRow(row, parentItem);
                    break;
                }
            }
            if (row >= rowCount) {
                rootItem->appendRow(parentItem);
            }

        } else {
            using namespace dpfservice;
            SymbolInfo::merge(parentItem, parentSymInfo);
        }

        QString parentTooltip;
        parentTooltip += "Kind: " + parentSymInfo.getKind() + "\n";
        parentTooltip += "Type: " + parentSymInfo.getType() + "\n";
        parentTooltip += "value: " + parentSymInfo.getValue() + "\n";
        parentTooltip += "Location Count:" +
                QString::number(SymbolInfo::get(parentItem).getLocations().size());
        parentItem->setToolTip(parentTooltip);
        rwLock->unlock();
        // write unlock


        auto currentSymInfo = clang_getCursorCustomSymbolInfo(current);


        // read lock
        rwLock->lockForRead();
        auto currentItem = findItem(parentItem, currentSymInfo);
        rwLock->unlock();
        // read unlock


        // write lock
        rwLock->lockForWrite();
        if (!currentItem) {
            currentItem = new QStandardItem(currentSymInfo.getValue()
                                            + " " + currentSymInfo.getType());
            SymbolInfo::set(currentItem, currentSymInfo);
            parentItem->appendRow(currentItem);
        } else {
            SymbolInfo::merge(currentItem, currentSymInfo);
        }

        QString currentTooltip;
        currentTooltip += "Kind: " + currentSymInfo.getKind() + "\n";
        currentTooltip += "Type: " + currentSymInfo.getType() + "\n";
        currentTooltip += "value: " + currentSymInfo.getValue() + "\n";
        currentTooltip += "Location Count:" +
                QString::number(SymbolInfo::get(currentItem).getLocations().size());
        currentItem->setToolTip(currentTooltip);
        rwLock->unlock();
        // write unlock
    }

    return CXChildVisit_Recurse;
};

SymbolCppAsynParser::SymbolCppAsynParser()
{
    QObject::connect(qApp, &QApplication::destroyed, [=](){
        ::canRunAllThread = true;
    });
}

SymbolCppAsynParser::~SymbolCppAsynParser()
{
    qDebug() << __FUNCTION__;
}

void SymbolCppAsynParser::setGlobalRunFlags(bool canRun)
{
    canRunAllThread = canRun;
}

bool SymbolCppAsynParser::globalRunFlags()
{
    return canRunAllThread;
}

void SymbolCppAsynParser::doParserOne(QStandardItem *item, const QString &file,
                                      const QSet<QString> &srcFiles)
{

    CXIndex index = clang_createIndex(0, 0);

    CXTranslationUnit unit ;
    CXErrorCode err = clang_parseTranslationUnit2(index, file.toLatin1(),
                                                  nullptr, 0,
                                                  nullptr, 0,
                                                  CXTranslationUnit_None, // CXTranslationUnit_SingleFileParse
                                                  &unit);
    if (err != CXErrorCode::CXError_Success) {
        qCritical() << "Filaed, clang_parseTranslationUnit2 craete CXTranslationUnit: "
                    << file;
        parserEnd(false);
        return;
    }

    // std::cout << "\nParser FileName: " << file.toStdString();
    VisitData data {item, srcFiles, getDefaultKindFilter()};
    clang_visitChildren(clang_getTranslationUnitCursor(unit), parseVisitChildren, &data);
    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    parserEnd(true);
}

void SymbolCppAsynParser::doParser(QStandardItem *item, const QSet<QString> &srcFiles)
{
    qInfo() << __FUNCTION__;

    for (auto filePath : srcFiles) {
        doParserOne(item, filePath, srcFiles);
    }
    parserEnd(true);
}
