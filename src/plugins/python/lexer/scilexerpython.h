// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCILEXERPYTHON_H
#define SCILEXERPYTHON_H

#include "base/abstractlexerproxy.h"

class SciLexerPython : public AbstractLexerProxy
{
    Q_OBJECT
public:
    //! This enum defines the meanings of the different styles used by the
    //! Python lexer.
    enum {
        //! The default.
        Default = 0,

        //! A comment.
        Comment = 1,

        //! A number.
        Number = 2,

        //! A double-quoted string.
        DoubleQuotedString = 3,

        //! A single-quoted string.
        SingleQuotedString = 4,

        //! A keyword.
        Keyword = 5,

        //! A triple single-quoted string.
        TripleSingleQuotedString = 6,

        //! A triple double-quoted string.
        TripleDoubleQuotedString = 7,

        //! The name of a class.
        ClassName = 8,

        //! The name of a function or method.
        FunctionMethodName = 9,

        //! An operator.
        Operator = 10,

        //! An identifier
        Identifier = 11,

        //! A comment block.
        CommentBlock = 12,

        //! The end of a line where a string is not closed.
        UnclosedString = 13,

        //! A highlighted identifier.  These are defined by keyword set
        //! 2.  Reimplement keywords() to define keyword set 2.
        HighlightedIdentifier = 14,

        //! A decorator.
        Decorator = 15,

        //! A double-quoted f-string.
        DoubleQuotedFString = 16,

        //! A single-quoted f-string.
        SingleQuotedFString = 17,

        //! A triple single-quoted f-string.
        TripleSingleQuotedFString = 18,

        //! A triple double-quoted f-string.
        TripleDoubleQuotedFString = 19,
    };

    explicit SciLexerPython(QObject *parent = nullptr);

    virtual const char *language() const override;
    virtual const char *lexer() const override;
    virtual QString description(int style) const override;
    virtual bool isSupport(const QString &file) const override;
    virtual int blockLookback() const override;
    virtual const char *blockStart(int *style) const override;
    virtual int braceStyle() const override;
    virtual QColor defaultColor(int style) const override;
    virtual bool defaultEolFill(int style) const override;
    virtual QFont defaultFont(int style) const override;
    virtual QColor defaultPaper(int style) const override;
    virtual const char *keywords(int set) const override;
    virtual void refreshProperties() override;
};

#endif   // SCILEXERPYTHON_H
