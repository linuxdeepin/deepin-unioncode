// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QMap>
#include <QObject>

enum SearchScope {
    AllProjects,
    CurrentProject,
    CurrentDocument
};

struct SearchParams
{
    QStringList filePathList;
    QString searchText;
    bool sensitiveFlag;
    bool wholeWordsFlag;
    QStringList patternsList;
    QStringList exPatternsList;
};

struct ReplaceParams
{
    QStringList filePathList;
    QString searchText;
    QString replaceText;
};

struct FindItem
{
    QString filePathName;
    int lineNumber = -1;
    QString context;
};

using FindItemList = QList<FindItem>;

Q_DECLARE_METATYPE(SearchParams)
Q_DECLARE_METATYPE(ReplaceParams)
Q_DECLARE_METATYPE(FindItem)

#endif   //CONSTANTS_H
