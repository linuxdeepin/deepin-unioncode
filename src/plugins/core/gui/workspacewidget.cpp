// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"

#include <DTitlebar>
#include <DTabBar>
#include <DStyle>
#include <DStackedWidget>
#include <DPushButton>

inline constexpr int kMinimumWidth = { 100 };

WorkspaceWidget::WorkspaceWidget(QWidget *parent) : DWidget(parent)
{
    initUi();
}

void WorkspaceWidget::initUi()
{
    mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    setMinimumWidth(kMinimumWidth);

    editWorkspaceWidget = new DFrame(this);
    editWorkspaceWidget->setLineWidth(0);
    DStyle::setFrameRadius(editWorkspaceWidget, 0);
    stackEditWorkspaceWidget = new DStackedWidget(this);
    workspaceTabBar = new DFrame(this);
    workspaceTabBar->setLineWidth(0);
    DStyle::setFrameRadius(workspaceTabBar, 0);

    QHBoxLayout *workspaceTabLayout = new QHBoxLayout(workspaceTabBar);
    workspaceTabLayout->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *workspaceVLayout = new QVBoxLayout();
    workspaceVLayout->setContentsMargins(0, 0, 0, 0);
    workspaceVLayout->setSpacing(0);
    workspaceVLayout->addWidget(stackEditWorkspaceWidget);
    workspaceVLayout->addWidget(workspaceTabBar);
    editWorkspaceWidget->setLayout(workspaceVLayout);

    mainLayout->addWidget(editWorkspaceWidget);
}

void WorkspaceWidget::addWorkspaceWidget(const QString &title, AbstractWidget *treeWidget, const QString &iconName)
{
    auto qTreeWidget = static_cast<DWidget *>(treeWidget->qWidget());

    qTreeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    editWorkspaceWidgets.insert(title, qTreeWidget);
    stackEditWorkspaceWidget->addWidget(qTreeWidget);

    DToolButton *tabBtn = new DToolButton;
    tabBtn->setCheckable(true);
    tabBtn->setChecked(false);
    tabBtn->setToolTip(title);
    tabBtn->setIcon(QIcon::fromTheme(iconName));
    tabBtn->setMinimumSize(QSize(24, 24));
    tabBtn->setIconSize(QSize(16, 16));
    tabBtn->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *btnLayout = static_cast<QHBoxLayout*>(workspaceTabBar->layout());
    btnLayout->addWidget(tabBtn);

    connect(tabBtn, &DPushButton::clicked, qTreeWidget, [=]{
        switchWidgetWorkspace(title);
    });
    workspaceTabButtons.insert(title, tabBtn);
    switchWidgetWorkspace(title);
}

bool WorkspaceWidget::switchWidgetWorkspace(const QString &title)
{
    auto widget = editWorkspaceWidgets[title];
    stackEditWorkspaceWidget->setCurrentWidget(widget);
    for (auto it = workspaceTabButtons.begin(); it != workspaceTabButtons.end(); ++it) {
        it.value()->setChecked(false);
        if (it.key() == title)
            it.value()->setChecked(true);
    }

    emit expandStateChange(widget->property("canExpand").toBool());
    emit workSpaceWidgeSwitched(title);

    return false;
}

bool WorkspaceWidget::getCurrentExpandState()
{
    return stackEditWorkspaceWidget->currentWidget()->property("canExpand").toBool();
}

QString WorkspaceWidget::getCurrentTitle() const
{
    for (auto it = workspaceTabButtons.begin(); it != workspaceTabButtons.end(); ++it) {
        if (it.value()->isChecked() == true)
            return it.key();
    }
    return "";
}
