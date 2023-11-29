// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
};

#endif // TASKVIEW_H
