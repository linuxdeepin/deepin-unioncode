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
#include "debuggerglobals.h"
#include "debugmanager.h"

#include "services/window/windowservice.h"
#include "services/debugger/debuggerservice.h"

#include "base/abstractnav.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"
#include "base/abstractmainwindow.h"
#include "base/abstractwidget.h"

#include <QMenu>

using namespace dpfservice;

void DebuggerPlugin::initialize()
{
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(dpfservice::DebuggerService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool DebuggerPlugin::start()
{
    auto &ctx = dpfInstance.serviceContext();
    windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    DebuggerService *debuggerService = ctx.service<DebuggerService>(DebuggerService::name());
    if (!debuggerService) {
        qCritical() << "Failed, can't found debugger service";
        abort();
    }

    debugManager->initialize(windowService, debuggerService);

    // instert output pane to window.
    emit windowService->addContextWidget(tr("&Application Output"), new AbstractWidget(debugManager->getOutputPane()));
    emit windowService->addContextWidget(tr("Stac&kFrame"), new AbstractWidget(debugManager->getStackPane()));
    emit windowService->setWidgetWatch(new AbstractWidget(debugManager->getLocalsPane()));
    emit windowService->addContextWidget(tr("Break&points"), new AbstractWidget(debugManager->getBreakpointPane()));

    connect(debugManager, &DebugManager::debugStarted, this, &DebuggerPlugin::slotDebugStarted);

    return true;
}

dpf::Plugin::ShutdownFlag DebuggerPlugin::stop()
{
    QProcess::execute("killall -9 debugadapter");
    return Sync;
}

void DebuggerPlugin::slotDebugStarted()
{
    if (windowService) {
        emit windowService->switchWidgetContext("&Application Output");
    }
}
