// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debuggerplugin.h"
#include "debuggerglobals.h"
#include "debugmanager.h"

#include "services/window/windowservice.h"
#include "services/debugger/debuggerservice.h"

#include "base/abstractaction.h"
#include "base/abstractmenu.h"
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

    if (windowService->addNavigationItem) {
        QAction *action = new QAction(MWNA_DEBUG, this);
        action->setIcon(QIcon::fromTheme("debug-navigation"));
        windowService->addNavigationItem(new AbstractAction(action));
        windowService->registerWidgetToMode("debugMainWindow", new AbstractWidget(debugManager->getDebugMainPane()), CM_DEBUG, Position::Left, true, true);
        windowService->registerWidgetToMode("debuggerWatcher", new AbstractWidget(debugManager->getLocalsPane()), CM_DEBUG, Position::Right, true, false);
    }

    connect(debugManager, &DebugManager::debugStarted, this, &DebuggerPlugin::slotDebugStarted);

    return true;
}

dpf::Plugin::ShutdownFlag DebuggerPlugin::stop()
{
    delete debugManager;
    QProcess::execute("killall -9 debugadapter");
    return Sync;
}

void DebuggerPlugin::slotDebugStarted()
{
    uiController.doSwitch(MWNA_DEBUG);
    uiController.switchContext(tr("&Application Output"));
}
