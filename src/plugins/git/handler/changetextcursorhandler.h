// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHANGETEXTCURSORHANDLER_H
#define CHANGETEXTCURSORHANDLER_H

#include "abstracttextcursorhandler.h"

class ChangeTextCursorHandler : public AbstractTextCursorHandler
{
    Q_OBJECT
public:
    explicit ChangeTextCursorHandler(GitEditor *editor = nullptr);

    bool findContentsUnderCursor(const QTextCursor &cursor) override;
    void highlightCurrentContents() override;
    void handleCurrentContents() override;

private:
    QString currentChange;
};

#endif // CHANGETEXTCURSORHANDLER_H
