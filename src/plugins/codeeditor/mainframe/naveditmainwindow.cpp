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
    if (!windowService->addToolBarActionItem) {
        windowService->addToolBarActionItem = std::bind(&NavEditMainWindow::addToolBarActionItem, this, _1, _2, _3);
    }

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

    initToolbar();

    qDockWidgetContext = new AutoHideDockWidget(DDockWidget::tr("Context"), this);
    qDockWidgetContext->setFeatures(DDockWidget::DockWidgetMovable);
    qTabWidgetContext = new DTabWidget(qDockWidgetContext);
    qTabWidgetContext->setMinimumHeight(100);
    qDockWidgetContext->setWidget(qTabWidgetContext);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, qDockWidgetContext);
    mainToolBar = new ToolBarManager(tr("toolbar"));

    titlebar()->setFixedHeight(0);    

    // initialize space area.
    qDockWidgetWorkspace = new AutoHideDockWidget(this);
    qDockWidgetWorkspace->setFeatures(DDockWidget::DockWidgetMovable);
    qDockWidgetWorkspace->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    qDockWidgetWorkspace->setTitleBarWidget(new DWidget());
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, qDockWidgetWorkspace);
    setCorner(Qt::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);

    qTabWidgetWorkspace = new DTabWidget();
    qTabWidgetWorkspace->setMinimumHeight(300);
    qTabWidgetWorkspace->setTabPosition(DTabWidget::West);
    workspaceWidgets.insert(DDockWidget::tr("Workspace"), qTabWidgetWorkspace);

    qDockWidgetWorkspace->setWidget(qTabWidgetWorkspace);
}

NavEditMainWindow::~NavEditMainWindow()
{
    qTabWidgetContext->removeTab(findIndex(qTabWidgetContext, dpfservice::CONSOLE_TAB_TEXT));
    qInfo() << __FUNCTION__;
}

QStringList NavEditMainWindow::contextWidgetTitles() const
{
    QStringList result;
    for (int index = 0; index < qTabWidgetContext->count(); index ++) {
        result<< qTabWidgetContext->tabText(index);
    }
    return result;
}

void NavEditMainWindow::addWidgetWorkspace(const QString &title, AbstractWidget *treeWidget)
{
    auto qTreeWidget = static_cast<DWidget*>(treeWidget->qWidget());
    qTabWidgetWorkspace->addTab(qTreeWidget, title);

    adjustWorkspaceItemOrder();
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

void NavEditMainWindow::addContextWidget(const QString &title, AbstractWidget *contextWidget, const QString &group)
{
    QMutexLocker locker(&mutex);
    DWidget *qWidget = static_cast<DWidget*>(contextWidget->qWidget());
    if (!qWidget || !qTabWidgetContext || group.isEmpty()) {
        return;
    }

    int index = 0;
    if (!contextList.contains(group)) {
        for (int i = 0; i < contextList.size(); i++) {
            const QString &temp = contextList.at(i);
            if (QString::compare(group, temp) < 0) {
                break;
            } else {
                index++;
            }
        }
    } else {
        index = contextList.indexOf(group, 0);
        for (; index < contextList.size(); index++) {
            if (contextList.at(index) != group) {
                break;
            }
        }
    }
    contextList.insert(index, group);
    qTabWidgetContext->insertTab(index, qWidget, title);
}

bool NavEditMainWindow::hasContextWidget(const QString &title)
{
    QMutexLocker locker(&mutex);
    int count = qTabWidgetContext->count();
    for (int i = 0; i < count; ++i) {
        if (qTabWidgetContext->tabText(i) == title)
            return true;
    }
    return false;
}

void NavEditMainWindow::removeContextWidget(AbstractWidget *contextWidget)
{
    DWidget *qWidget = static_cast<DWidget*>(contextWidget->qWidget());
    if (!qWidget || !qTabWidgetContext)
        return;

    int index = qTabWidgetContext->indexOf(qWidget);
    qTabWidgetContext->removeTab(index);
}

bool NavEditMainWindow::switchWidgetWorkspace(const QString &title)
{
    for (int i = 0; i < qTabWidgetWorkspace->count(); i++) {
        if (qTabWidgetWorkspace->tabText(i) == title) {
            qTabWidgetWorkspace->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}

bool NavEditMainWindow::switchWidgetContext(const QString &title)
{
    for (int i = 0; i < qTabWidgetContext->count(); i++){
        if (qTabWidgetContext->tabText(i) == title) {
            qTabWidgetContext->setCurrentIndex(i);
            return true;
        }
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

bool NavEditMainWindow::addToolBarActionItem(const QString &id, QAction *action, const QString &group)
{
    QMutexLocker locker(&mutex);

    if (!mainToolBar)
        return false;

    if (group == "Search") {
        searchAction = action;
        return true;
    }

    addTopToolBar(id, action, group);

    return true;
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

void NavEditMainWindow::adjustWorkspaceItemOrder()
{
    auto tabBar = qTabWidgetWorkspace->tabBar();
    for (int i = 0; i < tabBar->count(); i++) {
        if(tabBar->tabText(i) == MWCWT_PROJECTS)
            tabBar->moveTab(i, 0);
        if (tabBar->tabText(i) == MWCWT_SYMBOL)
            tabBar->moveTab(i, 1);
    }
}

void NavEditMainWindow::initToolbar()
{
    topToolBarWidget.insert(MWNA_EDIT, new DWidget());
    topToolBarWidget.insert(MWNA_DEBUG, new DWidget());
    QHBoxLayout *hLayout = new QHBoxLayout(topToolBarWidget[MWNA_EDIT]);
    hLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    hLayout->setContentsMargins(20, 0, 0, 0);
    QHBoxLayout *hLayout1 = new QHBoxLayout(topToolBarWidget[MWNA_DEBUG]);
    hLayout1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    hLayout1->setContentsMargins(20, 0, 0, 0);
}

DWidget *NavEditMainWindow::getTopToolBarWidget(const QString &navName)
{
    return topToolBarWidget[navName];
}

bool NavEditMainWindow::addTopToolBar(const QString &id, QAction *action, const QString &group)
{
    if (!action || id.isEmpty())
        return false;

    QHBoxLayout *toolBarLayout = static_cast<QHBoxLayout*>(topToolBarWidget[group]->layout());
    toolBarLayout->addWidget(addIconButton(action));
    if (id == "Restart.Debugging")
        toolBarLayout->addSpacing(20);
    if (id == "Step.Out" || id == "Running") {
        toolBarLayout->addSpacing(20);
        toolBarLayout->insertWidget(-1, addIconButton(searchAction));
    }

    return true;
}

DIconButton *NavEditMainWindow::addIconButton(QAction *action)
{
    if (!action)
        return {};

    DIconButton *iconBtn = new DIconButton();
    iconBtn->setToolTip(action->text());
    iconBtn->setIcon(action->icon());
    iconBtn->setMinimumSize(QSize(36, 36));
    iconBtn->setIconSize(QSize(20, 20));
    iconBtn->setShortcut(action->shortcut());
    connect(iconBtn, &DIconButton::clicked, action, &QAction::triggered);

    return iconBtn;
}
