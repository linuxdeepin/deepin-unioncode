// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKMODEL_H
#define TASKMODEL_H

#include "common/type/task.h"

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
