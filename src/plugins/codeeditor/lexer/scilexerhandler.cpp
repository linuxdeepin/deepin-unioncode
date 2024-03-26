// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scilexerhandler.h"

SciLexerHandler::SciLexerHandler(QObject *parent)
    : QsciLexer(parent)
{
}

void SciLexerHandler::setProxy(AbstractLexerProxy *proxy)
{
    this->proxy = proxy;
    connect(proxy, &AbstractLexerProxy::propertyChanged, this, &QsciLexer::propertyChanged);
}

const char *SciLexerHandler::language() const
{
    return proxy->language();
}

const char *SciLexerHandler::lexer() const
{
    return proxy->lexer();
}

QString SciLexerHandler::description(int style) const
{
    return proxy->description(style);
}

QStringList SciLexerHandler::autoCompletionWordSeparators() const
{
    return proxy->autoCompletionWordSeparators();
}

const char *SciLexerHandler::blockEnd(int *style) const
{
    return proxy->blockEnd(style);
}

int SciLexerHandler::blockLookback() const
{
    return proxy->blockLookback();
}

const char *SciLexerHandler::blockStart(int *style) const
{
    return proxy->blockStart(style);
}

const char *SciLexerHandler::blockStartKeyword(int *style) const
{
    return proxy->blockStartKeyword(style);
}

int SciLexerHandler::braceStyle() const
{
    return proxy->braceStyle();
}

bool SciLexerHandler::caseSensitive() const
{
    return proxy->caseSensitive();
}

const char *SciLexerHandler::keywords(int set) const
{
    return proxy->keywords(set);
}

QColor SciLexerHandler::defaultColor(int style) const
{
    return proxy->defaultColor(style);
}

bool SciLexerHandler::defaultEolFill(int style) const
{
    return proxy->defaultEolFill(style);
}

QColor SciLexerHandler::defaultPaper(int style) const
{
    return proxy->defaultPaper(style);
}

void SciLexerHandler::refreshProperties()
{
    proxy->refreshProperties();
}

const char *SciLexerHandler::wordCharacters() const
{
    return proxy->wordCharacters();
}
