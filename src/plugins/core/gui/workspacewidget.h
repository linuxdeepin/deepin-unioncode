// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEWIDGET_H
#define WORKSPACEWIDGET_H

#include "base/abstractwidget.h"

#include <DWidget>
#include <DFrame>
#include <DToolButton>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class WorkspaceWidget : public DWidget
{
    Q_OBJECT
public:
    explicit WorkspaceWidget(QWidget *parent = nullptr);

    void addWorkspaceWidget(const QString &title, AbstractWidget *treeWidget, const QString &iconName);
    bool switchWidgetWorkspace(const QString &title);

private:
    void initUi();

    QVBoxLayout *mainLayout { nullptr };

    QMap<QString, DWidget *> workspaceWidgets;
    DWidget *editWorkspaceWidget { nullptr };

    QMap<QString, DWidget *> editWorkspaceWidgets;
    QMap<QString, DToolButton *> workspaceTabButtons;
    DStackedWidget *stackEditWorkspaceWidget { nullptr };
    DFrame *workspaceTabBar { nullptr };
};

#endif   // WORKSPACEWIDGET_H
