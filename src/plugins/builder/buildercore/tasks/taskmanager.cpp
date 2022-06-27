/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "taskmanager.h"
#include "transceiver/eventsender.h"

TaskManager *TaskManager::instance()
{
    static TaskManager ins;
    return &ins;
}

QListView *TaskManager::getView() const
{
    return view;
}

void TaskManager::clearTasks()
{
    model->clearTasks();
}

TaskManager::TaskManager(QObject *parent) : QObject(parent)
{
    view = new TaskView();
    model.reset(new TaskModel());
    view->setModel(model.get());
    auto tld = new TaskDelegate;
    view->setItemDelegate(tld);

    view->setFrameStyle(QFrame::NoFrame);
    view->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(view->selectionModel(), &QItemSelectionModel::currentChanged,
            tld, &TaskDelegate::currentChanged);

    connect(view->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &TaskManager::currentChanged);
    connect(view, &QAbstractItemView::activated,
            this, &TaskManager::triggerDefaultHandler);
}

void TaskManager::slotAddTask(const Task &task, int linkedOutputLines, int skipLines)
{
    Q_UNUSED(linkedOutputLines)
    Q_UNUSED(skipLines)

    model->addTask(task);
}

void TaskManager::currentChanged(const QModelIndex &index)
{
    Q_UNUSED(index)
}

void TaskManager::triggerDefaultHandler(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    Task task(model->task(index));
    if (task.isNull())
        return;

    if (task.file.exists()) {
        EventSender::jumpTo(task.file.toString(), task.movedLine);
    }
}
