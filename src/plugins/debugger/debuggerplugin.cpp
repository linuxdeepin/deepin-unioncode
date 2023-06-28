// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    windowService->addContextWidget(tr("Stac&kFrame"), new AbstractWidget(debugManager->getStackPane()), "Application");
    windowService->setWidgetWatch(new AbstractWidget(debugManager->getLocalsPane()));
    windowService->addContextWidget(tr("Break&points"), new AbstractWidget(debugManager->getBreakpointPane()), "Application");

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
    editor.switchContext(tr("&Application Output"));
}
