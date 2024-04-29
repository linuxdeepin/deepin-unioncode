// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitblamehighlighter.h"

#include <DGuiApplicationHelper>

#include <QSet>
#include <QTextCharFormat>

DGUI_USE_NAMESPACE

GitBlameHighlighter::GitBlameHighlighter(const QSet<QString> &changes, QTextDocument *doc)
    : BaseHighlighter(doc)
{
    setChanges(changes);
}

void GitBlameHighlighter::setChanges(const QSet<QString> &changes)
{
    changeMap.clear();
    const int changeNumberCount = changes.size();
    if (changeNumberCount == 0)
        return;

    QColor background = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType ? Qt::black : Qt::white;
    const QList<QColor> colors = generateColors(changeNumberCount, background);
    int m = 0;
    const int cstep = colors.count() / changeNumberCount;
    const auto cend = changes.constEnd();
    for (auto it = changes.constBegin(); it != cend; ++it) {
        QTextCharFormat format;
        format.setForeground(colors.at(m));
        changeMap.insert(*it, format);
        m += cstep;
    }
}

void GitBlameHighlighter::highlightBlock(const QString &text)
{
    if (text.isEmpty() || changeMap.empty())
        return;

    const QString change = changeNumber(text);
    const auto it = changeMap.constFind(change);
    if (it != changeMap.constEnd())
        setFormatWithSpaces(text, 0, text.length(), it.value());
}

QString GitBlameHighlighter::changeNumber(const QString &block) const
{
    const int pos = block.indexOf(' ', 4);
    return pos > 1 ? block.left(pos) : QString();
}
