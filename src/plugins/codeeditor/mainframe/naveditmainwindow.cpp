/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "naveditmainwindow.h"
#include "autohidedockwidget.h"
#include "base/abstractwidget.h"
#include "base/abstractcentral.h"
#include "base/abstractconsole.h"
#include "services/window/windowservice.h"
#include "transceiver/codeeditorreceiver.h"
#include "common/common.h"
#include "toolbarmanager.h"

#include <QDebug>
#include <QDockWidget>
#include <QEvent>
#include <QWidget>

using namespace dpfservice;
static NavEditMainWindow *ins{nullptr};
int findIndex(QTabWidget* tabWidget, const QString &text)
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
    : QMainWindow (parent, flags)
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

    if (!windowService->addToolBarSeparator) {
        windowService->addToolBarSeparator = std::bind(&NavEditMainWindow::addToolBarSeparator, this, _1);
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

    qDockWidgetContext = new AutoHideDockWidget(QDockWidget::tr("Context"), this);
    qDockWidgetContext->setFeatures(QDockWidget::DockWidgetMovable);
    qTabWidgetContext = new QTabWidget(qDockWidgetContext);
    qTabWidgetContext->setMinimumHeight(100);
    qDockWidgetContext->setWidget(qTabWidgetContext);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, qDockWidgetContext);
    mainToolBar = new ToolBarManager(tr("toolbar"));
    addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar->getToolBar());
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

void NavEditMainWindow::setConsole(AbstractConsole *console)
{
    QWidget *qConsoleWidget = static_cast<QWidget*>(console->qWidget());
    if (!qConsoleWidget || !qTabWidgetContext) {
        return;
    }

    int consoleIndex = findIndex(qTabWidgetContext, dpfservice::CONSOLE_TAB_TEXT);
    if (consoleIndex >= 0) {
        qTabWidgetContext->removeTab(consoleIndex);
        qTabWidgetContext->insertTab(consoleIndex, qConsoleWidget, dpfservice::CONSOLE_TAB_TEXT);
        return;
    }
    qConsoleWidget->setParent(qTabWidgetContext);
    qTabWidgetContext->insertTab(0, qConsoleWidget, dpfservice::CONSOLE_TAB_TEXT);
}

void NavEditMainWindow::addWidgetWorkspace(const QString &title, AbstractWidget *treeWidget)
{
    if (!qDockWidgetWorkspace) {
        qTabWidgetWorkspace = new QTabWidget();
        QFont font = qTabWidgetWorkspace->font();
        font.setItalic(true);
        qTabWidgetWorkspace->setFont(font);
        qTabWidgetWorkspace->setMinimumHeight(300);
        qTabWidgetWorkspace->setTabPosition(QTabWidget::West);
        qDockWidgetWorkspace = new AutoHideDockWidget(QDockWidget::tr("Workspace"), this);
        qDockWidgetWorkspace->setFeatures(QDockWidget::DockWidgetMovable);
        qDockWidgetWorkspace->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, qDockWidgetWorkspace);
        qDockWidgetWorkspace->setWidget(qTabWidgetWorkspace);
    }

    if (qTabWidgetWorkspace) {
        auto qTreeWidget = (QWidget*)treeWidget->qWidget();
        qTabWidgetWorkspace->addTab(qTreeWidget, title);
    }
}

void NavEditMainWindow::setWidgetEdit(AbstractCentral *editWidget)
{
    if (centralWidget()) {
        delete qWidgetEdit;
        qWidgetEdit = nullptr;
    }
    qWidgetEdit = (QWidget*)editWidget->qWidget();
    setCentralWidget(qWidgetEdit);
}

void NavEditMainWindow::setWidgetWatch(AbstractWidget *watchWidget)
{
    if (!qDockWidgetWatch) {
        qDockWidgetWatch = new AutoHideDockWidget(QDockWidget::tr("Watcher"), this);
        qDockWidgetWatch->setFeatures(QDockWidget::AllDockWidgetFeatures);
        qDockWidgetWatch->hide();
        addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, qDockWidgetWatch);
    }
    if (qDockWidgetWatch) {
        if (qWidgetWatch) {
            delete qWidgetWatch;
            qWidgetWatch = nullptr;
        }
        qWidgetWatch = static_cast<QWidget*>(watchWidget->qWidget());
        qWidgetWatch->setParent(qDockWidgetWatch);
        qDockWidgetWatch->setWidget(qWidgetWatch);
        qDockWidgetWatch->hide();
        qWidgetWatch->hide();
    }
}

void NavEditMainWindow::addWidgetContext(const QString &title, AbstractWidget *contextWidget, const QString &group)
{
    QWidget *qWidget = static_cast<QWidget*>(contextWidget->qWidget());
    if (!qWidget || !qTabWidgetContext) {
        return;
    }

    if (group.isEmpty()) {
        qTabWidgetContext->insertTab(1, qWidget, title);
        return;
    }

    int index = 0;
    if (!contextList.count(group)) {
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
    qTabWidgetContext->insertTab(index + 1, qWidget, title);
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

void NavEditMainWindow::addFindToolBar(AbstractWidget *findToolbar)
{
    if (!findToolbar)
        return;

    if (!qDockWidgetFindToolBar) {
        qDockWidgetFindToolBar = new AutoHideDockWidget(QDockWidget::tr("Find ToolBar"), this);
        addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, qDockWidgetFindToolBar);
        QWidget *widget = static_cast<QWidget*>(findToolbar->qWidget());
        qDockWidgetFindToolBar->setWidget(widget);
        qDockWidgetFindToolBar->hide();
    }
}

void NavEditMainWindow::showFindToolBar()
{
     if (qDockWidgetFindToolBar) {
         qDockWidgetFindToolBar->show();
     }
}

bool NavEditMainWindow::addToolBarActionItem(const QString &id, QAction *action, const QString &group)
{
    if (!mainToolBar)
        return false;

    return mainToolBar->addActionItem(id, action, group);
}

bool NavEditMainWindow::addToolBarWidgetItem(const QString &id, AbstractWidget *widget, const QString &group)
{
    if (!mainToolBar)
        return false;

    return mainToolBar->addWidgetItem(id, static_cast<QWidget*>(widget->qWidget()), group);
}

void NavEditMainWindow::addToolBarSeparator(const QString &group)
{
    if (!mainToolBar)
        return;
    mainToolBar->addSeparator(group);
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
