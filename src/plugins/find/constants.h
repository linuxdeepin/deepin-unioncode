// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QMap>
#include <QObject>

struct SearchParams
{
    QStringList filePathList;
    QString searchText;
    bool sensitiveFlag;
    bool wholeWordsFlag;
    QStringList patternsList;
    QStringList exPatternsList;
    QMap<QString, QString> projectInfoMap;
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
    int lineNumber;
    QString context;
};

using FindItemList = QList<FindItem>;
using ProjectInfo = QMap<QString, QString>;

Q_DECLARE_METATYPE(SearchParams)
Q_DECLARE_METATYPE(ReplaceParams)
Q_DECLARE_METATYPE(FindItem)

#endif   //CONSTANTS_H
