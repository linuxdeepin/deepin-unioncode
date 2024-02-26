// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractlexerproxy.h"

#include <QFont>
#include <QApplication>
#include <QPalette>

AbstractLexerProxy::AbstractLexerProxy(QObject *parent)
    : QObject(parent)
{
}

const char *AbstractLexerProxy::lexer() const
{
    return nullptr;
}

bool AbstractLexerProxy::isSupport(const QString &fileName) const
{
    Q_UNUSED(fileName)
    return false;
}

const char *AbstractLexerProxy::blockEnd(int *style) const
{
    Q_UNUSED(style)
    return nullptr;
}

const char *AbstractLexerProxy::blockStart(int *style) const
{
    Q_UNUSED(style)
    return nullptr;
}

const char *AbstractLexerProxy::blockStartKeyword(int *style) const
{
    Q_UNUSED(style)
    return nullptr;
}

int AbstractLexerProxy::braceStyle() const
{
    return -1;
}

const char *AbstractLexerProxy::wordCharacters() const
{
    return nullptr;
}

QColor AbstractLexerProxy::defaultColor(int style) const
{
    Q_UNUSED(style)
    QPalette pal = QApplication::palette();
    return pal.text().color();
}

bool AbstractLexerProxy::defaultEolFill(int style) const
{
    Q_UNUSED(style)
    return false;
}

QFont AbstractLexerProxy::defaultFont(int style) const
{
    Q_UNUSED(style)
    QFont font("Courier New", 10);
    return font;
}

QColor AbstractLexerProxy::defaultPaper(int style) const
{
    Q_UNUSED(style)
    QPalette pal = QApplication::palette();
    return pal.base().color();
}

const char *AbstractLexerProxy::keywords(int set) const
{
    Q_UNUSED(set)
    return nullptr;
}

void AbstractLexerProxy::refreshProperties()
{
}

bool AbstractLexerProxy::caseSensitive() const
{
    return false;
}
