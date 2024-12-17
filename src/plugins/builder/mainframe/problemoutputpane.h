// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROBLEMOUTPUTPANE_H
#define PROBLEMOUTPUTPANE_H

#include "common/type/task.h"
#include "tasks/taskmodel.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE

class ProblemOutputPane : public DWidget
{
    Q_OBJECT
public:
    ProblemOutputPane(QWidget *parent = nullptr);
    virtual ~ProblemOutputPane() override;

    void clearContents();
    void addTask(const Task &task, int linkedOutputLines, int skipLines);
    void showSpecificTasks(ShowType type);
};

#endif   // PROBLEMOUTPUTPANE_H
