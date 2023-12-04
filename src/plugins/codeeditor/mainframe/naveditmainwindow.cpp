// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "naveditmainwindow.h"
#include "autohidedockwidget.h"
#include "base/abstractwidget.h"
#include "base/abstractcentral.h"
#include "base/abstractconsole.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/symbol/symbolservice.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"
#include "toolbarmanager.h"

#include <DTitlebar>
#include <DTabBar>
#include <DPushButton>
#include <DToolButton>
#include <DTabBar>

#include <QSizePolicy>
#include <QDebug>
#include <QDockWidget>
#include <QEvent>
#include <QWidget>

using namespace dpfservice;
static NavEditMainWindow *ins{nullptr};
int findIndex(DTabWidget* tabWidget, const QString &text)
{
    for (int index = 0; index < tabWidget->count(); index ++) {
        if (tabWidget->tabText(index) == text) {
            return index;
        }
    }
    return -1;
}

NavEditMainWindow *NavEditMainWindow::instance()
{
    if (!ins)
        ins = new NavEditMainWindow;
    return ins;
}

NavEditMainWindow::NavEditMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : DMainWindow (parent)
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    using namespace std::placeholders;

    if (!windowService->addToolBarWidgetItem) {
        windowService->addToolBarWidgetItem = std::bind(&NavEditMainWindow::addToolBarWidgetItem, this, _1, _2, _3);
    }

    if (!windowService->removeToolBarItem) {
        windowService->removeToolBarItem = std::bind(&NavEditMainWindow::removeToolBarItem, this, _1);
    }

    if (!windowService->setToolBarItemDisable) {
        windowService->setToolBarItemDisable = std::bind(&NavEditMainWindow::setToolBarItemDisable, this, _1, _2);
    }

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSwitchContext,
                     this, &NavEditMainWindow::switchWidgetContext);

    QObject::connect(EditorCallProxy::instance(), &EditorCallProxy::toSwitchWorkspace,
                     this, &NavEditMainWindow::switchWidgetWorkspace);

    // initialize space area.
    initWorkspaceUI();
    initContextUI();

    mainToolBar = new ToolBarManager(tr("toolbar"));
    titlebar()->setFixedHeight(0);
}

NavEditMainWindow::~NavEditMainWindow()
{
    qInfo() << __FUNCTION__;
}


void NavEditMainWindow::initWorkspaceUI()
{
    qDockWidgetWorkspace = new AutoHideDockWidget(this);
    qDockWidgetWorkspace->setFeatures(DDockWidget::DockWidgetMovable);
    qDockWidgetWorkspace->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    qDockWidgetWorkspace->setTitleBarWidget(new DWidget());

    editWorkspaceWidget = new DWidget(qDockWidgetWorkspace);
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

    qDockWidgetWorkspace->setWidget(editWorkspaceWidget);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, qDockWidgetWorkspace);
    setCorner(Qt::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);

    workspaceWidgets.insert(DDockWidget::tr("Workspace"), editWorkspaceWidget);
}

void NavEditMainWindow::initContextUI()
{
    qDockWidgetContext = new AutoHideDockWidget(this);
    qDockWidgetContext->setFeatures(DDockWidget::DockWidgetMovable);
    qDockWidgetContext->setTitleBarWidget(new DWidget());
    contextWidget = new DWidget(qDockWidgetContext);
    stackContextWidget = new DStackedWidget();

    contextTabBar = new DFrame();
    DStyle::setFrameRadius(contextTabBar, 0);
    contextTabBar->setLineWidth(0);
    contextTabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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

    qDockWidgetContext->setWidget(contextWidget);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, qDockWidgetContext);
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
        qDockWidgetWorkspace->setWidget(qWidget);
        workspaceWidgets.insert(title, qWidget);
    }
}

void NavEditMainWindow::switchWorkspaceArea(const QString &title)
{
    auto widget = workspaceWidgets.value(title);
    if (widget) {
        qDockWidgetWorkspace->setWidget(widget);
    }
}

DWidget *NavEditMainWindow::setWidgetEdit(AbstractCentral *editWidget)
{
    DWidget *oldWidget = qWidgetEdit;
    qWidgetEdit = static_cast<DWidget*>(editWidget->qWidget());
    // avoid be deleted when setCentralWidget.
    if (oldWidget) {
        oldWidget->setParent(nullptr);
    }
    setCentralWidget(qWidgetEdit);
    if (oldWidget) {
        oldWidget->setParent(this);
    }
    return oldWidget;
}

DWidget *NavEditMainWindow::setWidgetWatch(AbstractWidget *watchWidget)
{
    if (!qDockWidgetWatch) {
        qDockWidgetWatch = new AutoHideDockWidget(DDockWidget::tr("Watcher"), this);
        qDockWidgetWatch->setTitleBarWidget(new QWidget());
        qDockWidgetWatch->setFeatures(DDockWidget::AllDockWidgetFeatures);
        qDockWidgetWatch->hide();
        addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, qDockWidgetWatch);
    }
    DWidget *oldWidget = qWidgetWatch;
    qWidgetWatch = static_cast<DWidget*>(watchWidget->qWidget());
    qWidgetWatch->setParent(qDockWidgetWatch);
    // avoid be deleted when setWidget.
    if (oldWidget) {
        oldWidget->setParent(nullptr);
    }
    qDockWidgetWatch->setWidget(qWidgetWatch);
    if (oldWidget) {
        oldWidget->setParent(qDockWidgetWatch);
    }
    qDockWidgetWatch->hide();
    qWidgetWatch->hide();
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

bool NavEditMainWindow::switchWidgetTools(const QString &title)
{
    for (int i = 0; i < qTabWidgetTools->count(); i++) {
        if (qTabWidgetTools->tabText(i) == title) {
            qTabWidgetTools->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}

void NavEditMainWindow::addFindToolBar(AbstractWidget *findToolbar)
{
    if (!findToolbar)
        return;

    if (!qDockWidgetFindToolBar) {
        qDockWidgetFindToolBar = new AutoHideDockWidget(DDockWidget::tr("Find ToolBar"), this);
        addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, qDockWidgetFindToolBar);
        DWidget *widget = static_cast<DWidget*>(findToolbar->qWidget());
        qDockWidgetFindToolBar->setWidget(widget);
        qDockWidgetFindToolBar->hide();
    }
}

void NavEditMainWindow::showFindToolBar()
{
    if (qDockWidgetFindToolBar) {
        if (qDockWidgetFindToolBar->isVisible()) {
            qDockWidgetFindToolBar->hide();
        } else {
            qDockWidgetFindToolBar->show();
        }
    }
}

void NavEditMainWindow::addValgrindBar(AbstractWidget *valgrindbar)
{
    if (!valgrindbar)
        return;

    if (!qDockWidgetValgrindBar) {
        qDockWidgetValgrindBar = new AutoHideDockWidget(DDockWidget::tr("Valgrind"), this);
        addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, qDockWidgetValgrindBar);
        DWidget *widget = static_cast<DWidget*>(valgrindbar->qWidget());
        qDockWidgetValgrindBar->setWidget(widget);
        qDockWidgetValgrindBar->hide();
    }
}

void NavEditMainWindow::showValgrindBar()
{
    if (qDockWidgetValgrindBar) {
        if (qDockWidgetValgrindBar->isVisible()) {
            qDockWidgetValgrindBar->hide();
        } else {
            qDockWidgetValgrindBar->show();
        }
    }
}

bool NavEditMainWindow::addToolBarWidgetItem(const QString &id, AbstractWidget *widget, const QString &group)
{
    QMutexLocker locker(&mutex);
    if (!mainToolBar)
        return false;

    return mainToolBar->addWidgetItem(id, static_cast<DWidget*>(widget->qWidget()), group);
}

void NavEditMainWindow::removeToolBarItem(const QString &id)
{
    if (mainToolBar)
        mainToolBar->removeItem(id);
}

void NavEditMainWindow::setToolBarItemDisable(const QString &id, bool disable)
{
    if (mainToolBar)
        mainToolBar->disableItem(id, disable);
}
