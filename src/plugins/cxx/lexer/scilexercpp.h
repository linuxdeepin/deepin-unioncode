// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCILEXERCPP_H
#define SCILEXERCPP_H

#include "base/abstractlexerproxy.h"

class SciLexerCPP : public AbstractLexerProxy
{
    Q_OBJECT
public:
    enum Style{
        //! The default.
        Default = 0,
        InactiveDefault = Default + 64,

        //! A C comment.
        Comment = 1,
        InactiveComment = Comment + 64,

        //! A C++ comment line.
        CommentLine = 2,
        InactiveCommentLine = CommentLine + 64,

        //! A JavaDoc/Doxygen style C comment.
        CommentDoc = 3,
        InactiveCommentDoc = CommentDoc + 64,

        //! A number.
        Number = 4,
        InactiveNumber = Number + 64,

        //! A keyword.
        Keyword = 5,
        InactiveKeyword = Keyword + 64,

        //! A double-quoted string.
        DoubleQuotedString = 6,
        InactiveDoubleQuotedString = DoubleQuotedString + 64,

        //! A single-quoted string.
        SingleQuotedString = 7,
        InactiveSingleQuotedString = SingleQuotedString + 64,

        //! An IDL UUID.
        UUID = 8,
        InactiveUUID = UUID + 64,

        //! A pre-processor block.
        PreProcessor = 9,
        InactivePreProcessor = PreProcessor + 64,

        //! An operator.
        Operator = 10,
        InactiveOperator = Operator + 64,

        //! An identifier
        Identifier = 11,
        InactiveIdentifier = Identifier + 64,

        //! The end of a line where a string is not closed.
        UnclosedString = 12,
        InactiveUnclosedString = UnclosedString + 64,

        //! A C# verbatim string.
        VerbatimString = 13,
        InactiveVerbatimString = VerbatimString + 64,

        //! A JavaScript regular expression.
        Regex = 14,
        InactiveRegex = Regex + 64,

        //! A JavaDoc/Doxygen style C++ comment line.
        CommentLineDoc = 15,
        InactiveCommentLineDoc = CommentLineDoc + 64,

        //! A keyword defined in keyword set number 2.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet2 = 16,
        InactiveKeywordSet2 = KeywordSet2 + 64,

        //! A JavaDoc/Doxygen keyword.
        CommentDocKeyword = 17,
        InactiveCommentDocKeyword = CommentDocKeyword + 64,

        //! A JavaDoc/Doxygen keyword error.
        CommentDocKeywordError = 18,
        InactiveCommentDocKeywordError = CommentDocKeywordError + 64,

        //! A global class or typedef defined in keyword set number 5.  The
        //! class must be sub-classed and re-implement keywords() to make use
        //! of this style.
        GlobalClass = 19,
        InactiveGlobalClass = GlobalClass + 64,

        //! A C++ raw string.
        RawString = 20,
        InactiveRawString = RawString + 64,

        //! A Vala triple-quoted verbatim string.
        TripleQuotedVerbatimString = 21,
        InactiveTripleQuotedVerbatimString = TripleQuotedVerbatimString + 64,

        //! A Pike hash-quoted string.
        HashQuotedString = 22,
        InactiveHashQuotedString = HashQuotedString + 64,

        //! A pre-processor stream comment.
        PreProcessorComment = 23,
        InactivePreProcessorComment = PreProcessorComment + 64,

        //! A JavaDoc/Doxygen style pre-processor comment.
        PreProcessorCommentLineDoc = 24,
        InactivePreProcessorCommentLineDoc = PreProcessorCommentLineDoc + 64,

        //! A user-defined literal.
        UserLiteral = 25,
        InactiveUserLiteral = UserLiteral + 64,

        //! A task marker.
        TaskMarker = 26,
        InactiveTaskMarker = TaskMarker + 64,

        //! An escape sequence.
        EscapeSequence = 27,
        InactiveEscapeSequence = EscapeSequence + 64,
    };
    Q_ENUM(Style)

    explicit SciLexerCPP(QObject *parent = nullptr);

    virtual const char *language() const override;
    virtual const char *lexer() const override;
    virtual QString description(int style) const override;
    virtual bool isSupport(const QString &fileName) const override;
    virtual QStringList autoCompletionWordSeparators() const override;
    virtual const char *blockEnd(int *style) const override;
    virtual const char *blockStart(int *style) const override;
    virtual const char *blockStartKeyword(int *style) const override;
    virtual int braceStyle() const override;
    virtual const char *wordCharacters() const override;
    virtual QColor defaultColor(int style) const override;
    virtual bool defaultEolFill(int style) const override;
    virtual QFont defaultFont(int style) const override;
    virtual QColor defaultPaper(int style) const override;
    virtual const char *keywords(int set) const override;
    virtual void refreshProperties() override;
    virtual bool caseSensitive() const override;
};

#endif   // SCILEXERCPP_H
