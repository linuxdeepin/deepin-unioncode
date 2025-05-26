// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstracttextcursorhandler.h"

AbstractTextCursorHandler::AbstractTextCursorHandler(GitEditor *editor)
    : QObject(editor),
      gitEditor(editor)
{
}

bool AbstractTextCursorHandler::findContentsUnderCursor(const QTextCursor &cursor)
{
    curCurrsor = cursor;
    return false;
}

GitEditor *AbstractTextCursorHandler::editor() const
{
    return gitEditor;
}

QTextCursor AbstractTextCursorHandler::currentCursor() const
{
    return curCurrsor;
}
