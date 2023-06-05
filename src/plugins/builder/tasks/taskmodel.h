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
#ifndef TASKMODEL_H
#define TASKMODEL_H

#include "services/builder/task.h"

#include <QAbstractItemModel>
#include <QFont>

class TaskModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles { File = Qt::UserRole, Line, MovedLine, Description, FileNotFound, Type, Category, Icon, Task_t };

    explicit TaskModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Task task(const QModelIndex &index) const;

    QList<Task> getTasks() const;
    void addTask(const Task &task);
    void removeTask(const Task &task);
    void clearTasks();

    int taskCount();

    int sizeOfFile(const QFont &font);
    int getSizeOfLineNumber(const QFont &font);
signals:

public slots:
private:

    QFont fileMeasurementFont;
    QFont lineMeasurementFont;
    int maxSizeOfFileName = 0;
    int lastMaxSizeIndex = 0;
    int sizeOfLineNumber = 0;

    QList<Task> tasks;
};

#endif // TASKMODEL_H
