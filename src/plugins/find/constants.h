// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QMap>
#include <QObject>

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
    QStringList projectFileList;
    QStringList openedFileList;
    QString keyword;
    SearchScope scope;
};

struct SearchParams
{
    BaseParams baseParams;
    QStringList includeList;
    QStringList excludeList;
    SearchFlags flags = SearchNoFlag;
};

struct ReplaceParams
{
    QStringList filePathList;
    QString keyword;
    QString replaceText;
};

struct FindItem
{
    QString filePathName;
    int lineNumber = -1;
    QString context;

    inline bool operator==(const FindItem &other)
    {
        return filePathName == other.filePathName && lineNumber == other.lineNumber;
    }
};

using FindItemList = QList<FindItem>;

Q_DECLARE_METATYPE(SearchParams)
Q_DECLARE_METATYPE(ReplaceParams)
Q_DECLARE_METATYPE(FindItem)

#endif   //CONSTANTS_H
