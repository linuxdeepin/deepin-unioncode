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
#include "outputpane.h"
#include "common/common.h"

#include <QScrollBar>
#include <QDebug>

/**
 * @brief Output text color.
 */
const QColor kTextColorNormal(150, 150, 150);
const QColor kErrorMessageTextColor(255, 108, 108);
const QColor kMessageOutput(0, 135, 135);

class OutputPanePrivate
{
public:
    explicit OutputPanePrivate(QTextDocument *document)
        : cursor(document)
    {
    }

    ~OutputPanePrivate()
    {
    }

    bool enforceNewline = false;
    bool scrollToBottom = true;
    int maxCharCount = K_DEFAULT_MAX_CHAR_COUNT;
    QTextCursor cursor;
};

OutputPane::OutputPane(QWidget *parent)
    : QPlainTextEdit(parent)
    , d(new OutputPanePrivate(document()))
{
    setReadOnly(true);
}

OutputPane::~OutputPane()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void OutputPane::clearContents()
{
    clear();
}

QString OutputPane::normalizeNewlines(const QString &text)
{
    QString res = text;
    res.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    return res;
}

bool OutputPane::isScrollbarAtBottom() const
{
    return verticalScrollBar()->value() == verticalScrollBar()->maximum();
}

QString OutputPane::doNewlineEnforcement(const QString &out)
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

void OutputPane::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void OutputPane::appendCustomText(const QString &textIn, const QTextCharFormat &format)
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

    d->cursor.endEditBlock();
    if (atBottom)
        scrollToBottom();
}

void OutputPane::appendText(const QString &text, OutputFormat format)
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

    appendCustomText(text, textFormat);
}
