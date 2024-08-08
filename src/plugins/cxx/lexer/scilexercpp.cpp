// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scilexercpp.h"
#include "common/common.h"

#include <DGuiApplicationHelper>

#include <QColor>
#include <QFont>

DGUI_USE_NAMESPACE

SciLexerCPP::SciLexerCPP(QObject *parent)
    : AbstractLexerProxy(parent)
{
}

const char *SciLexerCPP::language() const
{
    return "cpp";
}

const char *SciLexerCPP::lexer() const
{
    return "cpp";
}

QString SciLexerCPP::description(int style) const
{
    switch (style) {
    case Default:
        return tr("Default");

    case InactiveDefault:
        return tr("Inactive default");

    case Comment:
        return tr("C comment");

    case InactiveComment:
        return tr("Inactive C comment");

    case CommentLine:
        return tr("C++ comment");

    case InactiveCommentLine:
        return tr("Inactive C++ comment");

    case CommentDoc:
        return tr("JavaDoc style C comment");

    case InactiveCommentDoc:
        return tr("Inactive JavaDoc style C comment");

    case Number:
        return tr("Number");

    case InactiveNumber:
        return tr("Inactive number");

    case Keyword:
        return tr("Keyword");

    case InactiveKeyword:
        return tr("Inactive keyword");

    case DoubleQuotedString:
        return tr("Double-quoted string");

    case InactiveDoubleQuotedString:
        return tr("Inactive double-quoted string");

    case SingleQuotedString:
        return tr("Single-quoted string");

    case InactiveSingleQuotedString:
        return tr("Inactive single-quoted string");

    case UUID:
        return tr("IDL UUID");

    case InactiveUUID:
        return tr("Inactive IDL UUID");

    case PreProcessor:
        return tr("Pre-processor block");

    case InactivePreProcessor:
        return tr("Inactive pre-processor block");

    case Operator:
        return tr("Operator");

    case InactiveOperator:
        return tr("Inactive operator");

    case Identifier:
        return tr("Identifier");

    case InactiveIdentifier:
        return tr("Inactive identifier");

    case UnclosedString:
        return tr("Unclosed string");

    case InactiveUnclosedString:
        return tr("Inactive unclosed string");

    case VerbatimString:
        return tr("C# verbatim string");

    case InactiveVerbatimString:
        return tr("Inactive C# verbatim string");

    case Regex:
        return tr("JavaScript regular expression");

    case InactiveRegex:
        return tr("Inactive JavaScript regular expression");

    case CommentLineDoc:
        return tr("JavaDoc style C++ comment");

    case InactiveCommentLineDoc:
        return tr("Inactive JavaDoc style C++ comment");

    case KeywordSet2:
        return tr("Secondary keywords and identifiers");

    case InactiveKeywordSet2:
        return tr("Inactive secondary keywords and identifiers");

    case CommentDocKeyword:
        return tr("JavaDoc keyword");

    case InactiveCommentDocKeyword:
        return tr("Inactive JavaDoc keyword");

    case CommentDocKeywordError:
        return tr("JavaDoc keyword error");

    case InactiveCommentDocKeywordError:
        return tr("Inactive JavaDoc keyword error");

    case GlobalClass:
        return tr("Global classes and typedefs");

    case InactiveGlobalClass:
        return tr("Inactive global classes and typedefs");

    case RawString:
        return tr("C++ raw string");

    case InactiveRawString:
        return tr("Inactive C++ raw string");

    case TripleQuotedVerbatimString:
        return tr("Vala triple-quoted verbatim string");

    case InactiveTripleQuotedVerbatimString:
        return tr("Inactive Vala triple-quoted verbatim string");

    case HashQuotedString:
        return tr("Pike hash-quoted string");

    case InactiveHashQuotedString:
        return tr("Inactive Pike hash-quoted string");

    case PreProcessorComment:
        return tr("Pre-processor C comment");

    case InactivePreProcessorComment:
        return tr("Inactive pre-processor C comment");

    case PreProcessorCommentLineDoc:
        return tr("JavaDoc style pre-processor comment");

    case InactivePreProcessorCommentLineDoc:
        return tr("Inactive JavaDoc style pre-processor comment");

    case UserLiteral:
        return tr("User-defined literal");

    case InactiveUserLiteral:
        return tr("Inactive user-defined literal");

    case TaskMarker:
        return tr("Task marker");

    case InactiveTaskMarker:
        return tr("Inactive task marker");

    case EscapeSequence:
        return tr("Escape sequence");

    case InactiveEscapeSequence:
        return tr("Inactive escape sequence");
    }

    return QString();
}

bool SciLexerCPP::isSupport(const QString &fileName) const
{
    using namespace support_file;
    auto id = Language::id(fileName);

    return id.compare(language()) == 0;
}

QStringList SciLexerCPP::autoCompletionWordSeparators() const
{
    QStringList wl;
    wl << "::"
       << "->"
       << ".";

    return wl;
}

const char *SciLexerCPP::blockEnd(int *style) const
{
    if (style)
        *style = Operator;

    return "}";
}

const char *SciLexerCPP::blockStart(int *style) const
{
    if (style)
        *style = Operator;

    return "{";
}

const char *SciLexerCPP::blockStartKeyword(int *style) const
{
    if (style)
        *style = Keyword;

    return "case catch class default do else finally for if private "
           "protected public struct try union while";
}

int SciLexerCPP::braceStyle() const
{
    return Operator;
}

const char *SciLexerCPP::wordCharacters() const
{
    return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#";
}

QColor SciLexerCPP::defaultColor(int style) const
{
    bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    switch (style) {
    case Default:
    case InactiveDefault:
        return isDarkTheme ? QColor("#d6cf9a") : QColor("#000000");

    case Comment:
    case CommentLine:
    case InactiveComment:
    case InactiveCommentLine:
    case CommentDoc:
    case InactiveCommentDoc:
    case CommentLineDoc:
    case InactiveCommentLineDoc:
    case PreProcessorCommentLineDoc:
    case InactivePreProcessorCommentLineDoc:
    case CommentDocKeyword:
    case CommentDocKeywordError:
    case InactiveCommentDocKeyword:
    case InactiveCommentDocKeywordError:
    case PreProcessorComment:
    case InactivePreProcessorComment:
        return isDarkTheme ? QColor("#a8abb0") : QColor("#008000");

    case Number:
    case InactiveNumber:
        return isDarkTheme ? QColor("#8a602c") : QColor("#000080");

    case Keyword:
    case InactiveKeyword:
        return isDarkTheme ? QColor("#45c6d6") : QColor("#808000");

    case DoubleQuotedString:
    case SingleQuotedString:
    case RawString:
    case InactiveDoubleQuotedString:
    case InactiveSingleQuotedString:
    case InactiveRawString:
        return isDarkTheme ? QColor("#d69545") : QColor("#008000");

    case PreProcessor:
    case InactivePreProcessor:
        return isDarkTheme ? QColor("#ff6aad") : QColor("#000080");

    case Operator:
    case UnclosedString:
    case InactiveUnclosedString:
    case InactiveOperator:
        return isDarkTheme ? QColor("#d6cf9a") : QColor("#000000");

    case VerbatimString:
    case TripleQuotedVerbatimString:
    case InactiveVerbatimString:
    case InactiveTripleQuotedVerbatimString:
    case HashQuotedString:
    case InactiveHashQuotedString:
        return isDarkTheme ? QColor("#d69545") : QColor("#008000");

    case Regex:
    case InactiveRegex:
        return isDarkTheme ? QColor("#45c6d6") : QColor("#3f7f3f");

    case UserLiteral:
    case InactiveUserLiteral:
        return isDarkTheme ? QColor("#d6cf9a") : QColor("#c06000");

    case TaskMarker:
    case InactiveTaskMarker:
        return isDarkTheme ? QColor("#ff6aad") : QColor("#be07ff");
    }

    return AbstractLexerProxy::defaultColor(style);
}

bool SciLexerCPP::defaultEolFill(int style) const
{
    switch (style) {
    case UnclosedString:
    case InactiveUnclosedString:
    case VerbatimString:
    case InactiveVerbatimString:
    case Regex:
    case InactiveRegex:
    case TripleQuotedVerbatimString:
    case InactiveTripleQuotedVerbatimString:
    case HashQuotedString:
    case InactiveHashQuotedString:
        return true;
    }

    return AbstractLexerProxy::defaultEolFill(style);
}

QColor SciLexerCPP::defaultPaper(int style) const
{
    switch (style) {
    case UnclosedString:
    case InactiveUnclosedString:
        return QColor(0xe0, 0xc0, 0xe0);

    case VerbatimString:
    case InactiveVerbatimString:
    case TripleQuotedVerbatimString:
    case InactiveTripleQuotedVerbatimString:
        return QColor(0xe0, 0xff, 0xe0);

    case Regex:
    case InactiveRegex:
        return QColor(0xe0, 0xf0, 0xe0);

    case RawString:
    case InactiveRawString:
        return QColor(0xff, 0xf3, 0xff);

    case HashQuotedString:
    case InactiveHashQuotedString:
        return QColor(0xe7, 0xff, 0xd7);
    }

    return AbstractLexerProxy::defaultPaper(style);
}

const char *SciLexerCPP::keywords(int set) const
{
    if (set == 1)
        return "and and_eq asm auto bitand bitor bool break case catch "
               "char class compl const constexpr const_cast continue default "
               "delete do double dynamic_cast else enum explicit export "
               "extern false farcall final float for friend goto if inline int "
               "long mutable namespace new not not_eq nullptr null out "
               "operator or or_eq override private protected public println register "
               "reinterpret_cast return short signed sizeof static system "
               "static_cast struct switch template this throw true try typedef "
               "typeid typename union unsigned using virtual void volatile "
               "wchar_t while xor xor_eq";

    if (set == 3)
        return "a addindex addtogroup anchor arg attention author b "
               "brief bug c class code date def defgroup deprecated "
               "dontinclude e em endcode endhtmlonly endif "
               "endlatexonly endlink endverbatim enum example "
               "exception f$ f[ f] file fn hideinitializer "
               "htmlinclude htmlonly if image include ingroup "
               "internal invariant interface latexonly li line link "
               "mainpage name namespace nosubgrouping note overload "
               "p page par param post pre ref relates remarks return "
               "retval sa section see showinitializer since skip "
               "skipline struct subsection test throw todo typedef "
               "union until var verbatim verbinclude version warning "
               "weakgroup $ @ \\ & < > # { }";

    return nullptr;
}

void SciLexerCPP::refreshProperties()
{
    emit propertyChanged("fold.at.else", "0");
    emit propertyChanged("fold.comment", "0");
    emit propertyChanged("fold.compact", "1");
    emit propertyChanged("fold.preprocessor", "1");
    emit propertyChanged("styling.within.preprocessor", "0");
    emit propertyChanged("lexer.cpp.allow.dollars", "1");
    emit propertyChanged("lexer.cpp.triplequoted.strings", "0");
    emit propertyChanged("lexer.cpp.hashquoted.strings", "0");
    emit propertyChanged("lexer.cpp.backquoted.strings", "0");
    emit propertyChanged("lexer.cpp.escape.sequence", "0");
    emit propertyChanged("lexer.cpp.verbatim.strings.allow.escapes", "0");
}

bool SciLexerCPP::caseSensitive() const
{
    return false;
}
