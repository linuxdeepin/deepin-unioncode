// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "taskmodel.h"
#include "taskdelegate.h"
#include "taskview.h"
#include "taskfilterproxymodel.h"

namespace dpfservice {
class AiService;
class WindowService;
}

/**
 * @brief Used to manage and display the information about build error,
 * move it to seperate plugin when more type information need be processed.
 */
class TaskManager : public QObject
{
    Q_OBJECT
public:
    static TaskManager *instance();

    DTK_WIDGET_NAMESPACE::DListView *getView() const;

    void clearTasks();

signals:

public slots:
    void slotAddTask(const Task &task, int linkedOutputLines, int skipLines);

    void currentChanged(const QModelIndex &index);
    void triggerDefaultHandler(const QModelIndex &index);

    void showSpecificTasks(ShowType type);
    void showContextMenu(const QPoint &pos);
    void fixIssueWithAi(const QModelIndex &index);

private:
    explicit TaskManager(QObject *parent = nullptr);

    QString readContext(const QString &path, int codeLine);

    TaskView *view = nullptr;
    QSharedPointer<TaskModel> model;
    QSharedPointer<TaskFilterProxyModel> filterModel;
    dpfservice::AiService *aiSrv = nullptr;
    dpfservice::WindowService *winSrv = nullptr;
};

#endif   // TASKMANAGER_H
