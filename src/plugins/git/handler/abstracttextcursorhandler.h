// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTTEXTCURSORHANDLER_H
#define ABSTRACTTEXTCURSORHANDLER_H

#include "gui/giteditor.h"

class AbstractTextCursorHandler : public QObject
{
    Q_OBJECT
public:
    explicit AbstractTextCursorHandler(GitEditor *editor = nullptr);

    virtual bool findContentsUnderCursor(const QTextCursor &cursor);
    virtual void highlightCurrentContents() = 0;
    virtual void handleCurrentContents() = 0;

    GitEditor *editor() const;
    QTextCursor currentCursor() const;

private:
    GitEditor *gitEditor { nullptr };
    QTextCursor curCurrsor;
};

#endif   // ABSTRACTTEXTCURSORHANDLER_H
