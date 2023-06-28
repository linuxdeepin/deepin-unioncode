// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROBLEMOUTPUTPANE_H
#define PROBLEMOUTPUTPANE_H

#include "services/builder/task.h"

#include <QWidget>
#include <QMenu>

class ProblemOutputPane : public QWidget
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
    QMenu *menu = nullptr;
};

#endif // PROBLEMOUTPUTPANE_H
