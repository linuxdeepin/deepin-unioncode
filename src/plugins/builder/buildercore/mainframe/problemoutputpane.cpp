/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "problemoutputpane.h"
#include "tasks/taskmanager.h"
#include "services/builder/task.h"


#include "common/common.h"

#include <QVBoxLayout>
#include <QMenu>

ProblemOutputPane::ProblemOutputPane(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(TaskManager::instance()->getView());
    setLayout(layout);
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


void ProblemOutputPane::contextMenuEvent(QContextMenuEvent * event)
{
    static QMenu *menu = nullptr;
    if (nullptr == menu) {
        menu = new QMenu(this);
        menu->setParent(this);
        menu->addActions(actionFactory());
    }

    menu->move(event->globalX(), event->globalY());
    menu->show();
}

QList<QAction*> ProblemOutputPane::actionFactory()
{
    QList<QAction*> list;
    auto action = new QAction(this);
    action->setText(tr("Clear"));
    connect(action, &QAction::triggered, [this](){
        clearContents();
    });
    list.append(action);
    return list;
}


