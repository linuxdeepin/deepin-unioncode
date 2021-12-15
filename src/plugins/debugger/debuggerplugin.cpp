/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "services/window/windowservice.h"

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

    return true;
}

dpf::Plugin::ShutdownFlag DebuggerPlugin::stop()
{
    return Sync;
}

bool DebuggerPlugin::InitUI(WindowService *windowService)
{
    QAction *startDebugging = new QAction("Start Debugging");
    AbstractAction * actionImpl = new AbstractAction(startDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *detachDebugger = new QAction("Detach Debugger");
    actionImpl = new AbstractAction(detachDebugger);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *interrupt = new QAction("Interrupt");
    actionImpl = new AbstractAction(interrupt);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *continueDebugging = new QAction("Continue");
    actionImpl = new AbstractAction(continueDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *abortDebugging = new QAction("Abort Debugging");
    actionImpl = new AbstractAction(abortDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *restartDebugging = new QAction("Restart Debugging");
    actionImpl = new AbstractAction(restartDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *stepOver = new QAction("Step Over");
    actionImpl = new AbstractAction(stepOver);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *stepIn = new QAction("Step In");
    actionImpl = new AbstractAction(stepIn);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *stepOut = new QAction("Step Out");
    actionImpl = new AbstractAction(stepOut);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    return true;
}
