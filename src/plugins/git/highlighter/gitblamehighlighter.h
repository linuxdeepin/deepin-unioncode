// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITBLAMEHIGHLIGHTER_H
#define GITBLAMEHIGHLIGHTER_H

#include "basehighlighter.h"

#include <QMap>

class GitBlameHighlighter : public BaseHighlighter
{
    Q_OBJECT
public:
    explicit GitBlameHighlighter(const QSet<QString> &changes, QTextDocument *doc = nullptr);

    void setChanges(const QSet<QString> &changes);
    void highlightBlock(const QString &text) override;

private:
    QString changeNumber(const QString &block) const;

private:
    using ChangeFormatMap = QMap<QString, QTextCharFormat>;
    ChangeFormatMap changeMap;
};

#endif   // GITBLAMEHIGHLIGHTER_H
