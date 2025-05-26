// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKVIEW_H
#define TASKVIEW_H

#include <DListView>

class TaskView : public DTK_WIDGET_NAMESPACE::DListView
{
    Q_OBJECT
public:
    explicit TaskView(QWidget *parent = nullptr);
    ~TaskView() override;
    void resizeEvent(QResizeEvent *e) override;

signals:
    void sigFixIssue(const QModelIndex &index);
};

#endif   // TASKVIEW_H
