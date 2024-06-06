// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QMap>
#include <QObject>

enum ResultRole {
    LineRole = Qt::UserRole + 1,
    ColumnRole,
    KeywordRole
};

enum SearchScope {
    AllProjects = 0,
    CurrentProject,
    CurrentFile
};

enum SearchFlag {
    SearchNoFlag = 0,
    SearchCaseSensitive = 1,
    SearchWholeWord = 1 << 1
};
Q_DECLARE_FLAGS(SearchFlags, SearchFlag)

struct BaseParams
{
    QStringList baseFileList;
    QStringList openedFileList;
    QString keyword;
};

struct SearchParams
{
    BaseParams baseParams;
    QStringList includeList;
    QStringList excludeList;
    SearchFlags flags = SearchNoFlag;
    SearchScope scope = AllProjects;
};

struct ReplaceParams
{
    BaseParams baseParams;
    QString replaceText;
    SearchFlags flags = SearchNoFlag;
};

struct FindItem
{
    QString filePathName;
    int line = -1;
    int column = -1;
    QString keyword;
    QString context;

    inline bool operator==(const FindItem &other)
    {
        return filePathName == other.filePathName && line == other.line
                && column == other.column;
    }
};

using FindItemList = QList<FindItem>;

Q_DECLARE_METATYPE(SearchParams)
Q_DECLARE_METATYPE(ReplaceParams)
Q_DECLARE_METATYPE(FindItem)

#endif   //CONSTANTS_H
