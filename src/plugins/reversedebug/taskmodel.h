// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "event_man.h"

#include <QAbstractItemModel>
#include <QFont>

namespace ReverseDebugger {
namespace Internal {

class Task
{
public:
    Task() = default;
    Task(const QString &description, const QString &category, const EventEntry *event);

    bool isNull() const;
    void clear();

    uint taskId = 0;
    const EventEntry *event = nullptr;
    QString category;
    QString description;

private:
    static uint s_nextId;
};

bool operator==(const Task &t1, const Task &t2);
bool operator<(const Task &a, const Task &b);
uint qHash(const Task &task);

class TaskModel : public QAbstractItemModel
{
    Q_OBJECT

    class CategoryData
    {
    public:
        CategoryData()
            : count(0) {}

        void addTask(const Task &task)
        {
            Q_UNUSED(task);
            ++count;
        }

        void removeTask(const Task &task)
        {
            Q_UNUSED(task);
            --count;
        }

        void clear()
        {
            count = 0;
        }

        QString displayName;
        int count = 0;
    };

public:
    enum Roles { Result = Qt::UserRole,
                 Description,
                 ExtraInfo,
                 Time,
                 Duration,
                 Return,
                 Tid,
                 ThreadNum,
                 Type,
                 Category,
                 Icon,
    };

    explicit TaskModel(QObject *parent = nullptr);

    void setTimelinePtr(void *timeline);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Task task(int index) const;
    Task task(const QModelIndex &index) const;

    QList<QString> categoryIds() const;
    QString categoryDisplayName(const QString &categoryId) const;
    void addCategory(const QString &categoryId, const QString &categoryName);

    QList<Task> tasks(const QString &categoryId = "") const;
    void addTask(const Task &task);
    void removeTask(const Task &task);
    void clearTasks(const QString &categoryId = "");
    int getSizeOfLineNumber(const QFont &font);

    int taskCount(const QString &categoryId);
    int rowForId(unsigned int id);
signals:

public slots:

private:
    QHash<QString, CategoryData> categories;   // category id to data
    QList<Task> allTasks;   // all tasks (in order of id)

    int sizeOfLineNumber = 0;
    QFont fileMeasurementFont;
    QFont lineMeasurementFont;
    void *timeline = nullptr;
};

}   // namespace Internal
}   // namespace ReverseDebugger
