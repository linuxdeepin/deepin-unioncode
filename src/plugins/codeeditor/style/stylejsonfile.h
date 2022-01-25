#ifndef STYLEJSONFILE_H
#define STYLEJSONFILE_H

#include <QObject>
#include <QHash>
#include <QString>

#include "common/type/menuext.h"

/*!
 * \brief The StyleJsonFile class
 * 该类线程安全
 */
class StyleJsonFilePrivate;
class StyleJsonFile : public QObject
{
    Q_OBJECT
    StyleJsonFilePrivate *const d;
public:
    enum_def(Key, QString)
    {
        E BackgroundSelf = "BackgroundSelf";
        E Cursor = "Cursor";
        E Namespace = "Namespace";
        E Type = "Type";
        E Class = "class";
        E Variable = "Variable";
        E Property = "Property";
        E Text = "Text";
        E Link = "Link";
        E Selection = "Selection";
        E LineNumber = "Line Number";
        E Keyword = "Keyword";
        E Punctuation = "Punctuation";
        E Operators = "Operators";
        E OverloadedOperators = "Overloaded Operators";
        E Preprocessor = "Preprocessor";
        E SearchResult = "Search Result";
        E SearchScope = "Search Scope";
        E Parentheses = "Parentheses";
        E MismatchedParentheses = "Mismatched Parentheses";
        E AutoComplete = "Auto Complete";
        E CurrentLine = "Current Line";
        E CurrentLineNumber = "Current Line Number";
        E Occurrences = "Occurrences";
        E UnusedOccurrences = "Unused Occurrences";
        E RenamingOccurrences = "Renaming Occurrences";
        E Number = "Number";
        E String = "String";
        E PrimitiveType = "Primitive Type";
        E Local = "Local";
        E Field = "Field";
        E Global = "Global";
        E Enumeration = "Enumeration";
        E Function = "Function";
        E FunctionDeclaration = "Function Declaration";
        E FunctionDefinition = "Function Definition";
        E VirtualFunction = "Virtual Function";
        E Comment = "Comment";
        E ErrorContext = "Error Context";
        E Warning = "Warning";
        E WarningContext = "Warning Context";
    };

    StyleJsonFile();
    virtual ~StyleJsonFile();
    bool setLanguage(const QString &languageID);
    QStringList themes();
    bool setTheme(const QString &theme);
    QJsonValue value(const QString &Key) const;
};

#endif // STYLEJSONFILE_H
