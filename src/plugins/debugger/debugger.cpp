/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "debugger.h"
#include "base/abstractnav.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"
#include "base/abstractmainwindow.h"
#include "services/window/windowservice.h"

#include <QMenu>

using namespace dpfservice;

void Debugger::initialize()
{

}

bool Debugger::start()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    QAction *startDebugging = new QAction("Start Debugging");
    AbstractAction * actionImpl = new AbstractAction(startDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *detachDebugger = new QAction("Detach Debugger");
    actionImpl = new AbstractAction(detachDebugger);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *interrupt = new QAction("Interrupt");
    actionImpl = new AbstractAction(interrupt);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *continueAction = new QAction("Continue");
    actionImpl = new AbstractAction(continueAction);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *abortDebugging = new QAction("Abort Debugging");
    actionImpl = new AbstractAction(abortDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *restartDebugging = new QAction("Restart Debugging");
    actionImpl = new AbstractAction(restartDebugging);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *setpOver = new QAction("Setp Over");
    actionImpl = new AbstractAction(setpOver);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *setpInto = new QAction("Setp Into");
    actionImpl = new AbstractAction(setpInto);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    QAction *setpOut = new QAction("Setp Out");
    actionImpl = new AbstractAction(setpOut);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    return true;
}

dpf::Plugin::ShutdownFlag Debugger::stop()
{
    return Sync;
}
