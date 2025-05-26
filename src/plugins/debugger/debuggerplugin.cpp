// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
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
inline constexpr char mainWindow[] = "debugMainWindow";

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
        auto *actionImpl = new AbstractAction(action);
        windowService->addNavigationItem(actionImpl, Priority::high);
        windowService->registerWidgetToMode(mainWindow, new AbstractWidget(debugManager->getDebugMainPane()), CM_DEBUG, Position::Left, true, true);
        windowService->bindWidgetToNavigation(mainWindow, actionImpl);
        windowService->setDockHeaderName(mainWindow, tr("debug"));
        auto localsPaneImpl =  new AbstractWidget(debugManager->getLocalsPane());
        QString variablesPane = tr("Variables Watcher");
        windowService->addWidgetRightspace(variablesPane, localsPaneImpl, "");
        connect(action, &QAction::triggered, this, [=]() {
            if (debugManager->getRunState() != AbstractDebugger::kNoRun)
                windowService->showWidgetAtRightspace(variablesPane);
        }, Qt::DirectConnection);
        connect(debugManager, &DebugManager::debugStarted, this, [=](){
            uiController.doSwitch(MWNA_DEBUG);
            windowService->showWidgetAtRightspace(variablesPane);
            uiController.switchContext(tr("&Application Output"));
        }, Qt::DirectConnection);
    }

    return true;
}

dpf::Plugin::ShutdownFlag DebuggerPlugin::stop()
{
    return Sync;
}
