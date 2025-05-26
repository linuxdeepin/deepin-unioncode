// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "problemoutputpane.h"
#include "tasks/taskmanager.h"
#include "common/type/task.h"

#include "common/common.h"

#include <QVBoxLayout>

ProblemOutputPane::ProblemOutputPane(QWidget *parent)
    : DWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(TaskManager::instance()->getView());
}

ProblemOutputPane::~ProblemOutputPane()
{
}

void ProblemOutputPane::clearContents()
{
    TaskManager::instance()->clearTasks();
}

void ProblemOutputPane::addTask(const Task &task, int linkedOutputLines, int skipLines)
{
    TaskManager::instance()->slotAddTask(task, linkedOutputLines, skipLines);
}

void ProblemOutputPane::showSpecificTasks(ShowType type)
{
    TaskManager::instance()->showSpecificTasks(type);
}
