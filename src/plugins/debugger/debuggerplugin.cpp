/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<huangyub@uniontech.com>
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
#include "debuggerplugin.h"
#include "base/abstractnav.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"
#include "base/abstractmainwindow.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "debuggerglobals.h"
#include "debugmanager.h"

#include <QMenu>

using namespace dpfservice;

void DebuggerPlugin::initialize()
{

}

bool DebuggerPlugin::start()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    InitUI(windowService);

    debugManager->initialize();

    // instert output pane to window.
    emit windowService->addContextWidget("AppOutput", new AbstractWidget(debugManager->getOutputPane()));
    emit windowService->addContextWidget("StackFrame", new AbstractWidget(debugManager->getStackPane()));
    emit windowService->addContextWidget("Locals", new AbstractWidget(debugManager->getLocalsPane()));
    emit windowService->addContextWidget("Breakpoints", new AbstractWidget(debugManager->getBreakpointPane()));

    return true;
}

dpf::Plugin::ShutdownFlag DebuggerPlugin::stop()
{
    return Sync;
}

bool DebuggerPlugin::InitUI(WindowService *windowService)
{
    QAction *startDebugging = new QAction("Start Debugging");
    startDebugging->setShortcut(QKeySequence(Qt::Key::Key_F5));
    connect(startDebugging, &QAction::triggered, debugManager, &DebugManager::startDebug);
    AbstractAction *actionImpl = new AbstractAction(startDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *detachDebugger = new QAction("Detach Debugger");
    connect(detachDebugger, &QAction::triggered, debugManager, &DebugManager::detachDebug);
    actionImpl = new AbstractAction(detachDebugger);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *interrupt = new QAction("Interrupt");
    connect(interrupt, &QAction::triggered, debugManager, &DebugManager::interruptDebug);
    actionImpl = new AbstractAction(interrupt);
    interrupt->setEnabled(false);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *continueDebugging = new QAction("Continue");
    connect(continueDebugging, &QAction::triggered, debugManager, &DebugManager::continueDebug);
    actionImpl = new AbstractAction(continueDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *abortDebugging = new QAction("Abort Debugging");
    connect(abortDebugging, &QAction::triggered, debugManager, &DebugManager::abortDebug);
    actionImpl = new AbstractAction(abortDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *restartDebugging = new QAction("Restart Debugging");
    connect(restartDebugging, &QAction::triggered, debugManager, &DebugManager::restartDebug);
    actionImpl = new AbstractAction(restartDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *stepOver = new QAction("Step Over");
    stepOver->setShortcut(QKeySequence(Qt::Key::Key_F10));
    connect(stepOver, &QAction::triggered, debugManager, &DebugManager::stepOver);
    actionImpl = new AbstractAction(stepOver);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *stepIn = new QAction("Step In");
    stepIn->setShortcut(QKeySequence(Qt::Key::Key_F11));
    connect(stepIn, &QAction::triggered, debugManager, &DebugManager::stepIn);
    actionImpl = new AbstractAction(stepIn);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *stepOut = new QAction("Step Out");
    stepOut->setShortcut(QKeySequence(Qt::Modifier::SHIFT | Qt::Key::Key_F11));
    connect(stepOut, &QAction::triggered, debugManager, &DebugManager::stepOut);
    actionImpl = new AbstractAction(stepOut);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    return true;
}
