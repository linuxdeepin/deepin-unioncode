// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FUZZYMATCHER_H
#define FUZZYMATCHER_H
#include <QVector>

QT_BEGIN_NAMESPACE
class QRegularExpression;
class QRegularExpressionMatch;
class QString;
QT_END_NAMESPACE

class FuzzyMatcher
{
public:
    enum class CaseSensitivity {
        CaseInsensitive,
        CaseSensitive,
        FirstLetterCaseSensitive
    };

    class HighlightingPositions {
    public:
        QVector<int> starts;
        QVector<int> lengths;
    };

    static QRegularExpression createRegExp(const QString &pattern,
            CaseSensitivity caseSensitivity = CaseSensitivity::CaseInsensitive);
    static HighlightingPositions highlightingPositions(const QRegularExpressionMatch &match);
};


#endif // FUZZYMATCHER_H
