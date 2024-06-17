// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTLEXERPROXY_H
#define ABSTRACTLEXERPROXY_H

#include <QObject>
#include <QStringList>
#include <QColor>

class AbstractLexerProxy : public QObject
{
    Q_OBJECT
public:
    explicit AbstractLexerProxy(QObject* parent = nullptr);
    virtual ~AbstractLexerProxy() = default;  // Virtual destructor for proper cleanup

    // Pure virtual methods (must be implemented in derived classes)
    virtual QString language() const = 0; 
    virtual QString description(int style) const = 0;

    // Virtual methods (can be overridden in derived classes)
    virtual QString lexer() const; // Changed return type from const char* to QString
    virtual bool isSupport(const QString& fileName) const;
    virtual QStringList autoCompletionWordSeparators() const;

    // Virtual methods for block indentation (using const char* for compatibility with Scintilla)
    virtual const char* blockEnd(int* style = nullptr) const;
    virtual int blockLookback() const;
    virtual const char* blockStart(int* style = nullptr) const;
    virtual const char* blockStartKeyword(int* style = nullptr) const;
    virtual int braceStyle() const;

    // Virtual methods for word characters and styling
    virtual const char* wordCharacters() const;
    virtual QColor defaultColor(int style) const;
    virtual bool defaultEolFill(int style) const;
    virtual QColor defaultPaper(int style) const;

    // Virtual methods for keywords
    virtual const char* keywords(int set) const;

    // Virtual method for refreshing properties
    virtual void refreshProperties();
    virtual bool caseSensitive() const;

signals:
    // Signal for property changes
    void propertyChanged(const char* prop, const char* val); 

private:
    class AbstractLexerProxyPrivate; // Forward declare the private class
    std::unique_ptr<AbstractLexerProxyPrivate> d; // Use unique_ptr for automatic memory management
};

#endif  // ABSTRACTLEXERPROXY_H

