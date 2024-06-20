// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTLEXERPROXY_H
#define ABSTRACTLEXERPROXY_H

#include <QObject>

class AbstractLexerProxy : public QObject
{
    Q_OBJECT
public:
    explicit AbstractLexerProxy(QObject *parent = nullptr);

    //! Returns the name of the language.
    virtual const char *language() const = 0;

    //! Returns the name of the lexer.  If 0 is returned then the lexer's
    //! numeric identifier is used.  The default implementation returns 0.
    virtual const char *lexer() const;

    //! Returns the descriptive name for style number \a style.  For a valid
    //! style number for this language a non-empty QString must be returned.
    //! If the style number is invalid then an empty QString must be returned.
    //! This is intended to be used in user preference dialogs.
    virtual QString description(int style) const = 0;

    //! Return true if support this `fileName`, otherwise return false
    virtual bool isSupport(const QString &fileName) const;

    //! \internal Returns the character sequences that can separate
    //! auto-completion words.
    virtual QStringList autoCompletionWordSeparators() const;

    //! \internal Returns a space separated list of words or characters in
    //! a particular style that define the end of a block for
    //! auto-indentation.  The styles is returned via \a style.
    virtual const char *blockEnd(int *style = nullptr) const;

    //! Returns the number of lines prior to the current one when determining
    //! the scope of a block when auto-indenting.
    virtual int blockLookback() const;

    //! \internal Returns a space separated list of words or characters in
    //! a particular style that define the start of a block for
    //! auto-indentation.  The styles is returned via \a style.
    virtual const char *blockStart(int *style = nullptr) const;

    //! \internal Returns a space separated list of keywords in a
    //! particular style that define the start of a block for
    //! auto-indentation.  The styles is returned via \a style.
    virtual const char *blockStartKeyword(int *style = nullptr) const;

    //! \internal Returns the style used for braces for brace matching.
    virtual int braceStyle() const;

    //! Returns the string of characters that comprise a word.
    virtual const char *wordCharacters() const;

    //! Returns the foreground colour of the text for style number \a style.
    //!
    //! \sa defaultPaper()
    virtual QColor defaultColor(int style) const;

    //! Returns the end-of-line fill for style number \a style.
    virtual bool defaultEolFill(int style) const;

    //! Returns the font for style number \a style.
    virtual QFont defaultFont(int style) const;

    //! Returns the background colour of the text for style number \a style.
    //!
    //! \sa defaultColor()
    virtual QColor defaultPaper(int style) const;

    //! Returns the set of keywords for the keyword set \a set recognised
    //! by the lexer as a space separated string.  Set 1 is normally used for
    //! primary keywords and identifiers.  Set 2 is normally used for secondary
    //! keywords and identifiers.  Set 3 is normally used for documentation
    //! comment keywords.  Set 4 is normally used for global classes and
    //! typedefs.
    virtual const char *keywords(int set) const;

    //! Causes all properties to be refreshed by emitting the
    //! propertyChanged() signal as required.
    virtual void refreshProperties();

    //! Returns true if the language is case sensitive.  The default is false.
    virtual bool caseSensitive() const;

signals:
    //! This signal is emitted when the value of the lexer property \a prop
    //! needs to be changed.  The new value is \a val.
    void propertyChanged(const char *prop, const char *val);
};

#endif   // ABSTRACTLEXERPROXY_H
