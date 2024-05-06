// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskmanager.h"
#include "transceiver/buildersender.h"
#include "common/common.h"

TaskManager *TaskManager::instance()
{
    static TaskManager ins;
    return &ins;
}

DListView *TaskManager::getView() const
{
    return view;
}

void TaskManager::clearTasks()
{
    tasks.clear();
    model->clearTasks();
}

TaskManager::TaskManager(QObject *parent) : QObject(parent)
{
    view = new TaskView();
    model.reset(new TaskModel());
    view->setModel(model.get());
    auto tld = new TaskDelegate(view);
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
    tasks.push_back(task);
}

void TaskManager::showSpecificTasks(ShowType type)
{
    model->clearTasks();
    if (type != ShowType::All) {
        QList<Task> specificTasks;
        for (const Task& task : tasks) {
            if (task.type == type) {
                specificTasks.push_back(task);
            }
        }
        model->addTasks(specificTasks);
    } else {
        model->addTasks(tasks);
    }
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
        editor.gotoLine(task.file.toString(), task.movedLine);
    }
}
