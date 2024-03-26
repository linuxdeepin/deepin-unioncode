// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractlexerproxy.h"

#include <DGuiApplicationHelper>

#include <QFont>

DGUI_USE_NAMESPACE

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

QStringList AbstractLexerProxy::autoCompletionWordSeparators() const
{
    return {};
}

const char *AbstractLexerProxy::blockEnd(int *style) const
{
    Q_UNUSED(style)
    return nullptr;
}

int AbstractLexerProxy::blockLookback() const
{
    return 20;
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
    bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    return isDarkTheme ? QColor("#d6cf9a") : QColor("#000000");
}

bool AbstractLexerProxy::defaultEolFill(int style) const
{
    Q_UNUSED(style)
    return false;
}

QColor AbstractLexerProxy::defaultPaper(int style) const
{
    Q_UNUSED(style)
    bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    return isDarkTheme ? QColor("#2e2f30") : QColor("#F8F8F8");
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
