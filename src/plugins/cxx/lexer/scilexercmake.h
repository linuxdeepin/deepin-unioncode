// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCILEXERCMAKE_H
#define SCILEXERCMAKE_H

#include "base/abstractlexerproxy.h"

class SciLexerCMake : public AbstractLexerProxy
{
    Q_OBJECT
public:
    //! This enum defines the meanings of the different styles used by the
    //! CMake lexer.
    enum {
        //! The default.
        Default = 0,

        //! A comment.
        Comment = 1,

        //! A string.
        String = 2,

        //! A left quoted string.
        StringLeftQuote = 3,

        //! A right quoted string.
        StringRightQuote = 4,

        //! A function.  (Defined by keyword set number 1.)
        Function = 5,

        //! A variable. (Defined by keyword set number 2.)
        Variable = 6,

        //! A label.
        Label = 7,

        //! A keyword defined in keyword set number 3.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet3 = 8,

        //! A WHILE block.
        BlockWhile = 9,

        //! A FOREACH block.
        BlockForeach = 10,

        //! An IF block.
        BlockIf = 11,

        //! A MACRO block.
        BlockMacro = 12,

        //! A variable within a string.
        StringVariable = 13,

        //! A number.
        Number = 14
    };

    explicit SciLexerCMake(QObject *parent = nullptr);

    virtual const char *language() const override;
    virtual const char *lexer() const override;
    virtual QString description(int style) const override;
    virtual bool isSupport(const QString &file) const override;
    virtual QColor defaultColor(int style) const override;
    virtual QColor defaultPaper(int style) const override;
    virtual const char *keywords(int set) const override;
    virtual void refreshProperties() override;
};

#endif   // SCILEXERCMAKE_H
