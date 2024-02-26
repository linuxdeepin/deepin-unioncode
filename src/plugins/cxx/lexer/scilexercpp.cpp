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
    switch (style) {
    case Default:
        return QColor(0x80, 0x80, 0x80);

    case Comment:
    case CommentLine:
        return QColor(0x00, 0x7f, 0x00);

    case CommentDoc:
    case CommentLineDoc:
    case PreProcessorCommentLineDoc:
        return QColor(0x3f, 0x70, 0x3f);

    case Number:
        return QColor(0x00, 0x7f, 0x7f);

    case Keyword:
        return QColor(0x00, 0x00, 0x7f);

    case DoubleQuotedString:
    case SingleQuotedString:
    case RawString:
        return QColor(0x7f, 0x00, 0x7f);

    case PreProcessor:
        return QColor(0x7f, 0x7f, 0x00);

    case Operator:
    case UnclosedString:
        return QColor(0x00, 0x00, 0x00);

    case VerbatimString:
    case TripleQuotedVerbatimString:
    case HashQuotedString:
        return QColor(0x00, 0x7f, 0x00);

    case Regex:
        return QColor(0x3f, 0x7f, 0x3f);

    case CommentDocKeyword:
        return QColor(0x30, 0x60, 0xa0);

    case CommentDocKeywordError:
        return QColor(0x80, 0x40, 0x20);

    case PreProcessorComment:
        return QColor(0x65, 0x99, 0x00);

    case InactiveDefault:
    case InactiveUUID:
    case InactiveCommentLineDoc:
    case InactiveKeywordSet2:
    case InactiveCommentDocKeyword:
    case InactiveCommentDocKeywordError:
    case InactivePreProcessorCommentLineDoc:
        return QColor(0xc0, 0xc0, 0xc0);

    case InactiveComment:
    case InactiveCommentLine:
    case InactiveNumber:
    case InactiveVerbatimString:
    case InactiveTripleQuotedVerbatimString:
    case InactiveHashQuotedString:
        return QColor(0x90, 0xb0, 0x90);

    case InactiveCommentDoc:
        return QColor(0xd0, 0xd0, 0xd0);

    case InactiveKeyword:
        return QColor(0x90, 0x90, 0xb0);

    case InactiveDoubleQuotedString:
    case InactiveSingleQuotedString:
    case InactiveRawString:
        return QColor(0xb0, 0x90, 0xb0);

    case InactivePreProcessor:
        return QColor(0xb0, 0xb0, 0x90);

    case InactiveOperator:
    case InactiveIdentifier:
    case InactiveGlobalClass:
        return QColor(0xb0, 0xb0, 0xb0);

    case InactiveUnclosedString:
        return QColor(0x00, 0x00, 0x00);

    case InactiveRegex:
        return QColor(0x7f, 0xaf, 0x7f);

    case InactivePreProcessorComment:
        return QColor(0xa0, 0xc0, 0x90);

    case UserLiteral:
        return QColor(0xc0, 0x60, 0x00);

    case InactiveUserLiteral:
        return QColor(0xd7, 0xa0, 0x90);

    case TaskMarker:
        return QColor(0xbe, 0x07, 0xff);

    case InactiveTaskMarker:
        return QColor(0xc3, 0xa1, 0xcf);
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

QFont SciLexerCPP::defaultFont(int style) const
{
    QFont f;

    switch (style) {
    case Comment:
    case InactiveComment:
    case CommentLine:
    case InactiveCommentLine:
    case CommentDoc:
    case InactiveCommentDoc:
    case CommentLineDoc:
    case InactiveCommentLineDoc:
    case CommentDocKeyword:
    case InactiveCommentDocKeyword:
    case CommentDocKeywordError:
    case InactiveCommentDocKeywordError:
    case TaskMarker:
    case InactiveTaskMarker:
        f = QFont("Courier New", 10);
        break;

    case Keyword:
    case InactiveKeyword:
    case Operator:
    case InactiveOperator:
        f = AbstractLexerProxy::defaultFont(style);
        f.setBold(true);
        break;

    case DoubleQuotedString:
    case InactiveDoubleQuotedString:
    case SingleQuotedString:
    case InactiveSingleQuotedString:
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
        f = QFont("Courier New", 10);
        break;

    default:
        f = AbstractLexerProxy::defaultFont(style);
    }

    return f;
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
        return "and and_eq asm auto bitand bitor bool break case "
               "catch char class compl const const_cast continue "
               "default delete do double dynamic_cast else enum "
               "explicit export extern false float for friend goto if "
               "inline int long mutable namespace new not not_eq "
               "operator or or_eq private protected public register "
               "reinterpret_cast return short signed sizeof static "
               "static_cast struct switch template this throw true "
               "try typedef typeid typename union unsigned using "
               "virtual void volatile wchar_t while xor xor_eq";

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
