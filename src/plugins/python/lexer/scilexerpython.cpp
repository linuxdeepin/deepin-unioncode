// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scilexerpython.h"

#include <DGuiApplicationHelper>

#include <QColor>
#include <QFont>
#include <QFileInfo>

DGUI_USE_NAMESPACE

SciLexerPython::SciLexerPython(QObject *parent)
    : AbstractLexerProxy(parent)
{
}

const char *SciLexerPython::language() const
{
    return "Python";
}

const char *SciLexerPython::lexer() const
{
    return "python";
}

QString SciLexerPython::description(int style) const
{
    switch (style) {
    case Default:
        return tr("Default");

    case Comment:
        return tr("Comment");

    case Number:
        return tr("Number");

    case DoubleQuotedString:
        return tr("Double-quoted string");

    case SingleQuotedString:
        return tr("Single-quoted string");

    case Keyword:
        return tr("Keyword");

    case TripleSingleQuotedString:
        return tr("Triple single-quoted string");

    case TripleDoubleQuotedString:
        return tr("Triple double-quoted string");

    case ClassName:
        return tr("Class name");

    case FunctionMethodName:
        return tr("Function or method name");

    case Operator:
        return tr("Operator");

    case Identifier:
        return tr("Identifier");

    case CommentBlock:
        return tr("Comment block");

    case UnclosedString:
        return tr("Unclosed string");

    case HighlightedIdentifier:
        return tr("Highlighted identifier");

    case Decorator:
        return tr("Decorator");

    case DoubleQuotedFString:
        return tr("Double-quoted f-string");

    case SingleQuotedFString:
        return tr("Single-quoted f-string");

    case TripleSingleQuotedFString:
        return tr("Triple single-quoted f-string");

    case TripleDoubleQuotedFString:
        return tr("Triple double-quoted f-string");
    }

    return QString();
}

bool SciLexerPython::isSupport(const QString &file) const
{
    QFileInfo info(file);
    return info.suffix().compare("py", Qt::CaseInsensitive) == 0;
}

int SciLexerPython::blockLookback() const
{
    return 0;
}

const char *SciLexerPython::blockStart(int *style) const
{
    if (style)
        *style = Operator;

    return ":";
}

int SciLexerPython::braceStyle() const
{
    return Operator;
}

QColor SciLexerPython::defaultColor(int style) const
{
    bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    switch (style) {
    case Default:
        return isDarkTheme ? QColor("#d6cf9a") : QColor("#000000");

    case Comment:
    case CommentBlock:
        return isDarkTheme ? QColor("#a8abb0") : QColor("#008000");

    case Number:
        return isDarkTheme ? QColor("#8a602c") : QColor("#000080");

    case UnclosedString:
    case DoubleQuotedString:
    case SingleQuotedString:
    case DoubleQuotedFString:
    case SingleQuotedFString:
    case TripleSingleQuotedString:
    case TripleDoubleQuotedString:
    case TripleSingleQuotedFString:
    case TripleDoubleQuotedFString:
        return isDarkTheme ? QColor("#d69545") : QColor("#008000");

    case Keyword:
        return isDarkTheme ? QColor("#ff8080") : QColor("#800080");

    case ClassName:
        return isDarkTheme ? QColor("#bcf0ff") : QColor("#000000");

    case FunctionMethodName:
        return isDarkTheme ? QColor("#56a8f5") : QColor("#00627a");

    case Operator:
    case Identifier:
        break;

    case HighlightedIdentifier:
        return isDarkTheme ? QColor("#aa4926") : QColor("#660099");

    case Decorator:
        return isDarkTheme ? QColor("#b3ae60") : QColor("#9e880d");
    }

    return AbstractLexerProxy::defaultColor(style);
}

bool SciLexerPython::defaultEolFill(int style) const
{
    if (style == UnclosedString)
        return true;

    return AbstractLexerProxy::defaultEolFill(style);
}

QFont SciLexerPython::defaultFont(int style) const
{
    return AbstractLexerProxy::defaultFont(style);
}

QColor SciLexerPython::defaultPaper(int style) const
{
    if (style == UnclosedString) {
        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
        return isDarkTheme ? QColor("#fa6675") : QColor("#ffcccc");
    }

    return AbstractLexerProxy::defaultPaper(style);
}

const char *SciLexerPython::keywords(int set) const
{
    if (set != 1)
        return nullptr;

    return "and as assert break class continue def del elif else except exec "
           "finally for from global if import in is lambda None not or pass "
           "print raise return try while with yield";
}

void SciLexerPython::refreshProperties()
{
    emit propertyChanged("fold.comment.python", "0");
    emit propertyChanged("fold.compact", "1");
    emit propertyChanged("fold.quotes.python", "0");
    emit propertyChanged("tab.timmy.whinge.level", "0");
    emit propertyChanged("lexer.python.strings.over.newline", "0");
    emit propertyChanged("lexer.python.strings.u", "1");
    emit propertyChanged("lexer.python.literals.binary", "1");
    emit propertyChanged("lexer.python.strings.b", "1");
    emit propertyChanged("lexer.python.keywords2.no.sub.identifiers", "0");
}
