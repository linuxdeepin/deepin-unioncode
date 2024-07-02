// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "outputdocumentfind.h"
#include "widget/outputpane.h"

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QRegularExpression>

class OutputDocumentFindPrivate
{
public:
    explicit OutputDocumentFindPrivate();

    QTextCursor textCursor() const;
    QTextDocument *document() const;
    void setTextCursor(const QTextCursor &cursor);
    void find(const QString &txt, QTextCursor start, bool isBackward);

public:
    QPlainTextEdit *textEdit { nullptr };
};

OutputDocumentFindPrivate::OutputDocumentFindPrivate()
{
}

QTextCursor OutputDocumentFindPrivate::textCursor() const
{
    Q_ASSERT(textEdit);
    return textEdit->textCursor();
}

QTextDocument *OutputDocumentFindPrivate::document() const
{
    Q_ASSERT(textEdit);
    return textEdit->document();
}

void OutputDocumentFindPrivate::setTextCursor(const QTextCursor &cursor)
{
    Q_ASSERT(textEdit);
    textEdit->setTextCursor(cursor);
}

void OutputDocumentFindPrivate::find(const QString &txt, QTextCursor start, bool isBackward)
{
    if (txt.isEmpty()) {
        setTextCursor(start);
        return;
    }

    QTextDocument::FindFlags textDocFlags;
    if (isBackward)
        textDocFlags |= QTextDocument::FindBackward;

    QRegularExpression regexp(txt, QRegularExpression::CaseInsensitiveOption);
    QTextCursor found = document()->find(regexp, start, textDocFlags);

    if (found.isNull()) {
        if (!isBackward)
            start.movePosition(QTextCursor::Start);
        else
            start.movePosition(QTextCursor::End);

        found = document()->find(regexp, start, textDocFlags);
        if (found.isNull())
            return;
    }
    setTextCursor(found);
}

OutputDocumentFind::OutputDocumentFind(OutputPane *parent)
    : AbstractDocumentFind(parent),
      d(new OutputDocumentFindPrivate())
{
    d->textEdit = parent->edit();
}

OutputDocumentFind::~OutputDocumentFind()
{
    delete d;
}

QString OutputDocumentFind::findString() const
{
    QTextCursor cursor = d->textCursor();
    if (cursor.hasSelection() && cursor.block() != cursor.document()->findBlock(cursor.anchor()))
        return QString();   // multi block selection

    if (cursor.hasSelection())
        return cursor.selectedText();

    if (!cursor.atBlockEnd() && !cursor.hasSelection()) {
        cursor.movePosition(QTextCursor::StartOfWord);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        QString s = cursor.selectedText();
        for (const QChar c : s) {
            if (!c.isLetterOrNumber() && c != QLatin1Char('_')) {
                s.clear();
                break;
            }
        }
        return s;
    }

    return QString();
}

void OutputDocumentFind::findNext(const QString &txt)
{
    d->find(txt, d->textCursor(), false);
}

void OutputDocumentFind::findPrevious(const QString &txt)
{
    d->find(txt, d->textCursor(), true);
}

bool OutputDocumentFind::supportsReplace() const
{
    return false;
}
