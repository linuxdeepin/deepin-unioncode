// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "naveditmainwindow.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/symbol/symbolservice.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"

#include <DTitlebar>
#include <DTabBar>
#include <DPushButton>
#include <DToolButton>
#include <DTabBar>

#include <QSplitter>
#include <QSizePolicy>
#include <QDebug>
#include <QDockWidget>
#include <QEvent>
#include <QWidget>

inline constexpr int kMinimumWidth = { 100 };

using namespace dpfservice;
static NavEditMainWindow *ins{nullptr};

NavEditMainWindow *NavEditMainWindow::instance()
{
    if (!ins)
        ins = new NavEditMainWindow;
    return ins;
}

NavEditMainWindow::NavEditMainWindow(QWidget *parent)
    : DWidget (parent)
{
    initUI();
    initConnect();
}

NavEditMainWindow::~NavEditMainWindow()
{
    qInfo() << __FUNCTION__;
}

void NavEditMainWindow::initUI()
{
    workspaceWidget = new DStackedWidget();
    workspaceWidget->setMinimumWidth(kMinimumWidth);
    contextWidget = new DWidget();
    editWidget = new DWidget();
    watchWidget = new DWidget();
    editWatchSplitter = new QSplitter(Qt::Horizontal);
    editWatchSplitter->setChildrenCollapsible(false);

    contextSpliter = new QSplitter(Qt::Vertical);
    contextSpliter->setMinimumWidth(kMinimumWidth);
    contextSpliter->setChildrenCollapsible(false);
    contextSpliter->addWidget(editWatchSplitter);
    contextSpliter->addWidget(contextWidget);
    contextSpliter->setStretchFactor(0, 5);
    contextSpliter->setStretchFactor(1, 1);

    QSplitter *workspaceSpliter = new QSplitter(Qt::Horizontal);
    workspaceSpliter->setChildrenCollapsible(false);
    workspaceSpliter->addWidget(workspaceWidget);

    //保证左右拉伸的时候，查找窗口与交互区窗口可以完全显示，不被遮挡
    contextSpliter->setMinimumWidth(800);

    workspaceSpliter->addWidget(contextSpliter);
    workspaceSpliter->setStretchFactor(0, 1);
    workspaceSpliter->setStretchFactor(1, 3);

    mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(workspaceSpliter);
    setLayout(mainLayout);

    initWorkspaceUI();
    initContextUI();
}

void NavEditMainWindow::initConnect()
{
    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSwitchContext,
                     this, &NavEditMainWindow::switchWidgetContext);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSwitchWorkspace,
                     this, &NavEditMainWindow::switchWidgetWorkspace);
}

void NavEditMainWindow::initWorkspaceUI()
{
    editWorkspaceWidget = new DWidget();
    stackEditWorkspaceWidget = new DStackedWidget();
    workspaceTabBar = new DFrame();
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

    workspaceWidget->addWidget(editWorkspaceWidget);
    workspaceWidgets.insert(DDockWidget::tr("Workspace"), editWorkspaceWidget);
}

void NavEditMainWindow::initContextUI()
{
    stackContextWidget = new DStackedWidget();
    contextTabBar = new DFrame();
    DStyle::setFrameRadius(contextTabBar, 0);
    contextTabBar->setLineWidth(0);
    QHBoxLayout *contextTabLayout = new QHBoxLayout(contextTabBar);
    contextTabLayout->setAlignment(Qt::AlignLeft);

    QVBoxLayout *contextVLayout = new QVBoxLayout();
    contextVLayout->setContentsMargins(0, 0, 0, 0);
    contextVLayout->setSpacing(0);
    contextVLayout->addWidget(new DHorizontalLine);
    contextVLayout->addWidget(contextTabBar);
    contextVLayout->addWidget(new DHorizontalLine);
    contextVLayout->addWidget(stackContextWidget);
    contextWidget->setLayout(contextVLayout);
}

void NavEditMainWindow::addWidgetWorkspace(const QString &title, AbstractWidget *treeWidget, const QString &iconName)
{
    auto qTreeWidget = static_cast<DWidget*>(treeWidget->qWidget());

    qTreeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    editWorkspaceWidgets.insert(title, qTreeWidget);
    stackEditWorkspaceWidget->addWidget(qTreeWidget);

    DToolButton *tabBtn = new DToolButton;
    tabBtn->setCheckable(true);
    tabBtn->setChecked(true);
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
}

bool NavEditMainWindow::switchWidgetWorkspace(const QString &title)
{
    stackEditWorkspaceWidget->setCurrentWidget(editWorkspaceWidgets[title]);
    for (auto it = workspaceTabButtons.begin(); it != workspaceTabButtons.end(); ++it) {
        it.value()->setChecked(false);
        if (it.key() == title)
            it.value()->setChecked(true);
    }

    return false;
}

void NavEditMainWindow::addWorkspaceArea(const QString &title, AbstractWidget *widget)
{
    auto qWidget = static_cast<DWidget*>(widget->qWidget());
    if (qWidget) {
        workspaceWidget->addWidget(qWidget);
        workspaceWidgets.insert(title, qWidget);
    }
}

void NavEditMainWindow::switchWorkspaceArea(const QString &title)
{
    auto widget = workspaceWidgets.value(title);
    if (widget) {
        workspaceWidget->setCurrentWidget(widget);
    }
}

DWidget *NavEditMainWindow::setWidgetEdit(AbstractWidget *edit)
{
    DWidget *oldWidget = editWidget;
    editWidget = static_cast<DWidget*>(edit->qWidget());
    // avoid be deleted when setCentralWidget.
    if (oldWidget) {
        oldWidget->setParent(nullptr);
    }
    editWatchSplitter->addWidget(editWidget);
    return oldWidget;
}

DWidget *NavEditMainWindow::setWidgetWatch(AbstractWidget *watch)
{
    DWidget *oldWidget = watchWidget;
    watchWidget = static_cast<DWidget*>(watch->qWidget());

    if (oldWidget) {
        oldWidget->setParent(nullptr);
    }
    editWatchSplitter->addWidget(watchWidget);
    editWatchSplitter->setStretchFactor(0, 5);
    editWatchSplitter->setStretchFactor(1, 1);
    watchWidget->hide();
    return oldWidget;
}

void NavEditMainWindow::addContextWidget(const QString &title, AbstractWidget *contextWidget, const QString &group, bool isVisible)
{
    QMutexLocker locker(&mutex);
    DWidget *qWidget = static_cast<DWidget*>(contextWidget->qWidget());
    if (!qWidget || group.isNull()) {
        return;
    }
    contextWidgets.insert(title, qWidget);

    stackContextWidget->addWidget(qWidget);
    DPushButton *tabBtn = new DPushButton(title);
    tabBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tabBtn->setCheckable(true);
    tabBtn->setFlat(true);
    tabBtn->setFocusPolicy(Qt::NoFocus);

    if (!isVisible)
        tabBtn->hide();

    QHBoxLayout *btnLayout = static_cast<QHBoxLayout*>(contextTabBar->layout());
    btnLayout->addWidget(tabBtn);

    connect(tabBtn, &DPushButton::clicked, qWidget, [=]{
        switchWidgetContext(title);
    });

    tabButtons.insert(title, tabBtn);
}

bool NavEditMainWindow::hasContextWidget(const QString &title)
{
    QMutexLocker locker(&mutex);

    if (tabButtons.contains(title))
        return true;

    return false;
}

void NavEditMainWindow::removeContextWidget(AbstractWidget *contextWidget)
{
    DWidget *qWidget = static_cast<DWidget*>(contextWidget->qWidget());
    if (!qWidget)
        return;
}

bool NavEditMainWindow::switchWidgetContext(const QString &title)
{
    stackContextWidget->setCurrentWidget(contextWidgets[title]);
    if (tabButtons.contains(title))
        tabButtons[title]->show();

    for (auto it = tabButtons.begin(); it != tabButtons.end(); ++it) {
        it.value()->setChecked(false);
        if (it.key() == title)
            it.value()->setChecked(true);
    }

    return false;
}

void NavEditMainWindow::addFindToolBar(AbstractWidget *findToolbar)
{
    if (!findToolbar)
        return;

    findWidget = static_cast<DWidget*>(findToolbar->qWidget());
    contextSpliter->insertWidget(0, findWidget);
    findWidget->hide();
}

void NavEditMainWindow::showFindToolBar()
{
    if (findWidget) {
        if (findWidget->isVisible()) {
            findWidget->hide();
        } else {
            findWidget->show();
        }
    }
}
