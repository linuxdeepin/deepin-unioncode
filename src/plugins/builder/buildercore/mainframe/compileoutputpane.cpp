/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "compileoutputpane.h"
#include "common/common.h"

#include <QScrollBar>
#include <QDebug>


class CompileOutputPanePrivate
{
public:
    explicit CompileOutputPanePrivate(QTextDocument *document)
        : cursor(document)
    {
    }

    ~CompileOutputPanePrivate()
    {
    }

    bool enforceNewline = false;
    bool scrollToBottom = true;
    int maxCharCount = K_DEFAULT_MAX_CHAR_COUNT;
    QTextCursor cursor;
};

CompileOutputPane::CompileOutputPane(QWidget *parent)
    : QPlainTextEdit(parent)
    , d(new CompileOutputPanePrivate(document()))
{
    setReadOnly(true);
}

CompileOutputPane::~CompileOutputPane()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void CompileOutputPane::clearContents()
{
    clear();
}

QString CompileOutputPane::normalizeNewlines(const QString &text)
{
    QString res = text;
    res.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    return res;
}

bool CompileOutputPane::isScrollbarAtBottom() const
{
    return verticalScrollBar()->value() == verticalScrollBar()->maximum();
}

QString CompileOutputPane::doNewlineEnforcement(const QString &out)
{
    d->scrollToBottom = true;
    QString s = out;
    if (d->enforceNewline) {
        s.prepend(QLatin1Char('\n'));
        d->enforceNewline = false;
    }

    if (s.endsWith(QLatin1Char('\n'))) {
        d->enforceNewline = true; // make appendOutputInline put in a newline next time
        s.chop(1);
    }

    return s;
}

void CompileOutputPane::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void CompileOutputPane::appendText(const QString &textIn, const QTextCharFormat &format)
{
    const QString text = normalizeNewlines(textIn);
    if (d->maxCharCount > 0 && document()->characterCount() >= d->maxCharCount) {
        qDebug() << "Maximum limit exceeded : " << d->maxCharCount;
        return;
    }
    const bool atBottom = isScrollbarAtBottom();
    if (!d->cursor.atEnd())
        d->cursor.movePosition(QTextCursor::End);
    d->cursor.beginEditBlock();
    d->cursor.insertText(doNewlineEnforcement(text), format);

    if (d->maxCharCount > 0 && document()->characterCount() >= d->maxCharCount) {
        QTextCharFormat tmp;
        tmp.setFontWeight(QFont::Bold);
        d->cursor.insertText(doNewlineEnforcement(tr("Additional output omitted") + QLatin1Char('\n')), tmp);
    }

    //d->cursor.endEditBlock(); //todo
    if (atBottom)
        scrollToBottom();
}

void CompileOutputPane::appendText(const QString &text, OutputFormat format)
{
    QTextCharFormat textFormat;
    switch (format) {
    case OutputFormat::Stdout:
        textFormat.setForeground(kTextColorNormal);
        textFormat.setFontWeight(QFont::Normal);
        break;
    case OutputFormat::Stderr:
        textFormat.setForeground(kErrorMessageTextColor);
        textFormat.setFontWeight(QFont::Normal);
        break;
    case OutputFormat::NormalMessage:
        textFormat.setForeground(kMessageOutput);
        break;
    case OutputFormat::ErrorMessage:
        textFormat.setForeground(kErrorMessageTextColor);
        textFormat.setFontWeight(QFont::Bold);
        break;
    }

    appendText(text, textFormat);
}
