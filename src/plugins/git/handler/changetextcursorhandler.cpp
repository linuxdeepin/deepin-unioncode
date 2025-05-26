// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "changetextcursorhandler.h"
#include "client/gitclient.h"

ChangeTextCursorHandler::ChangeTextCursorHandler(GitEditor *editor)
    : AbstractTextCursorHandler(editor)
{
}

bool ChangeTextCursorHandler::findContentsUnderCursor(const QTextCursor &cursor)
{
    AbstractTextCursorHandler::findContentsUnderCursor(cursor);
    currentChange = editor()->changeUnderCursor(cursor);
    return !currentChange.isEmpty();
}

void ChangeTextCursorHandler::highlightCurrentContents()
{
    QTextEdit::ExtraSelection sel;
    sel.cursor = currentCursor();
    sel.cursor.select(QTextCursor::WordUnderCursor);
    sel.format.setFontUnderline(true);
    sel.format.setProperty(QTextFormat::UserProperty, currentChange);
    editor()->setExtraSelections(QList<QTextEdit::ExtraSelection>() << sel);
}

void ChangeTextCursorHandler::handleCurrentContents()
{
    GitClient::instance()->show(editor()->sourceFile(), currentChange);
}
