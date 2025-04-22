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
    KeywordRole,
    MatchedLengthRole,
    FilePathRole,
    ReplaceTextRole
};

enum SearchScope {
    AllProjects = 0,
    CurrentProject,
    CurrentFile
};

enum SearchFlag {
    SearchNoFlag = 0,
    SearchCaseSensitively = 1,
    SearchWholeWords = 1 << 1,
    SearchRegularExpression = 1 << 2
};
Q_DECLARE_FLAGS(SearchFlags, SearchFlag)

enum MessageType {
    Information,
    Warning
};

struct FindItem
{
    QString filePathName;
    int line = -1;
    int column = -1;
    QString keyword;
    int matchedLength;
    QStringList capturedTexts;
    QString context;

    inline bool operator==(const FindItem &other) const
    {
        return filePathName == other.filePathName && line == other.line
                && column == other.column;
    }
};

using FindItemList = QList<FindItem>;

struct SearchParams
{
    QString keyword;
    QStringList projectFileList;
    QStringList editFileList;
    QStringList includeList;
    QStringList excludeList;
    SearchFlags flags = SearchNoFlag;
    SearchScope scope = AllProjects;
};

struct ReplaceParams
{
    QStringList editFileList;
    QMap<QString, FindItemList> resultMap;
    QString replaceText;
    SearchFlags flags = SearchNoFlag;
};

class Utils
{
public:
    static QString expandRegExpReplacement(const QString &replaceText, const QStringList &capturedTexts);
};

Q_DECLARE_METATYPE(SearchParams)
Q_DECLARE_METATYPE(ReplaceParams)
Q_DECLARE_METATYPE(FindItem)

#endif   //CONSTANTS_H
