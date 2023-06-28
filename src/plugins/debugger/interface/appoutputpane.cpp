// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appoutputpane.h"
#include "common/type/constants.h"

#include <QScrollBar>
#include <QMenu>
#include <QDebug>

class OutputWindowPrivate
{
public:
    explicit OutputWindowPrivate(QTextDocument *document)
        : cursor(document)
    {
    }

    ~OutputWindowPrivate()
    {
    }

    QTextCursor cursor;
    QMenu *menu = nullptr;
    int maxCharCount = default_max_char_count();
    bool enforceNewline = false;
    bool scrollToBottom = true;
};

AppOutputPane::AppOutputPane()
    : d(new OutputWindowPrivate(document()))
{
    setReadOnly(true);
}

AppOutputPane::~AppOutputPane()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void AppOutputPane::clearContents()
{
    clear();
}

QString normalizeNewlines(const QString &text)
{
    QString res = text;
    res.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    return res;
}

bool AppOutputPane::isScrollbarAtBottom() const
{
    return verticalScrollBar()->value() == verticalScrollBar()->maximum();
}

QString AppOutputPane::doNewlineEnforcement(const QString &out)
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

void AppOutputPane::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    // QPlainTextEdit destroys the first calls value in case of multiline
    // text, so make sure that the scroll bar actually gets the value set.
    // Is a noop if the first call succeeded.
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void AppOutputPane::appendText(const QString &textIn, const QTextCharFormat &format)
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

void AppOutputPane::appendText(const QString &text, OutputFormat format)
{
    QTextCharFormat textFormat;
    switch (format) {
    case OutputFormat::StdOut:
        textFormat.setForeground(kTextColorNormal);
        textFormat.setFontWeight(QFont::Normal);
        break;
    case OutputFormat::StdErr:
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
    default:
        // nothing to do.
        break;
    }

    appendText(text, textFormat);
}

void AppOutputPane::contextMenuEvent(QContextMenuEvent * event)
{
    if (nullptr == d->menu) {
        d->menu = new QMenu(this);
        d->menu->setParent(this);
        d->menu->addActions(actionFactory());
    }

    d->menu->move(event->globalX(), event->globalY());
    d->menu->show();
}

QList<QAction*> AppOutputPane::actionFactory()
{
    QList<QAction*> list;

    {
        auto action = new QAction(this);
        action->setText(tr("Copy"));
        connect(action, &QAction::triggered, [this](){
            if (!document()->toPlainText().isEmpty())
                copy();
        });
        list.append(action);
    }

    {
        auto action = new QAction(this);
        action->setText(tr("Clear"));
        connect(action, &QAction::triggered, [this](){
            if (!document()->toPlainText().isEmpty())
                clear();
        });
        list.append(action);
    }

    {
        auto action = new QAction(this);
        action->setText(tr("Select All"));
        connect(action, &QAction::triggered, [this](){
            if (!document()->toPlainText().isEmpty())
                selectAll();
        });
        list.append(action);
    }

    return list;
}
