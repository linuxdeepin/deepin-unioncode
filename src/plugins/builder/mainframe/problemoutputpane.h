// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROBLEMOUTPUTPANE_H
#define PROBLEMOUTPUTPANE_H

#include "common/type/task.h"

#include <DFrame>
#include <DMenu>

DWIDGET_USE_NAMESPACE

class ProblemOutputPane : public DFrame
{
    Q_OBJECT
public:
    ProblemOutputPane(QWidget *parent = nullptr);
    virtual ~ProblemOutputPane() override;

    void clearContents();
    void addTask(const Task &task, int linkedOutputLines, int skipLines);

protected:
    void contextMenuEvent(QContextMenuEvent * event) override;

private:
    QList<QAction*> actionFactory();
    DMenu *menu = nullptr;
};

#endif // PROBLEMOUTPUTPANE_H
