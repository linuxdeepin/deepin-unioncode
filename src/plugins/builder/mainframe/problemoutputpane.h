// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROBLEMOUTPUTPANE_H
#define PROBLEMOUTPUTPANE_H

#include "services/builder/task.h"

#include <DWidget>
#include <DMenu>

DWIDGET_USE_NAMESPACE

class ProblemOutputPane : public DWidget
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
