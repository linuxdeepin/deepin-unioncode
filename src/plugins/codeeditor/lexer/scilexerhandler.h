// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCILEXERHANDLER_H
#define SCILEXERHANDLER_H

#include "base/abstractlexerproxy.h"

#include <Qsci/qscilexer.h>

class SciLexerHandler : public QsciLexer
{
    Q_OBJECT
public:
    explicit SciLexerHandler(QObject *parent = nullptr);

    void setProxy(AbstractLexerProxy *proxy);

    virtual const char *language() const override;
    virtual const char *lexer() const override;
    virtual QString description(int style) const override;
    virtual const char *blockEnd(int *style) const override;
    virtual const char *blockStart(int *style) const override;
    virtual const char *blockStartKeyword(int *style) const override;
    virtual int braceStyle() const override;
    virtual bool caseSensitive() const override;
    virtual const char *keywords(int set) const override;
    virtual QColor defaultColor(int style) const override;
    virtual bool defaultEolFill(int style) const override;
    virtual QFont defaultFont(int style) const override;
    virtual QColor defaultPaper(int style) const override;
    virtual void refreshProperties() override;
    virtual const char *wordCharacters() const override;

private:
    AbstractLexerProxy *proxy { nullptr };
};

#endif   // SCILEXERHANDLER_H
