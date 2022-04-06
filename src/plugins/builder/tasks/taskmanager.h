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
#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "taskmodel.h"
#include "taskdelegate.h"
#include "taskview.h"

/**
 * @brief Used to manage and display the information about build error,
 * move it to seperate plugin when more type information need be processed.
 */
class TaskManager : public QObject
{
    Q_OBJECT
public:
    static TaskManager *instance();

    QListView *getView() const;

    void clearTasks();

signals:

public slots:
    void slotAddTask(const Task &task, int linkedOutputLines, int skipLines);

     void currentChanged(const QModelIndex &index);
     void triggerDefaultHandler(const QModelIndex &index);

private:
    explicit TaskManager(QObject *parent = nullptr);

    QSharedPointer<TaskView> view;
    QSharedPointer<TaskModel> model;
};

#endif // TASKMANAGER_H
