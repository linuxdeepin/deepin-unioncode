// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reversedebugplugin.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "common/actionmanager/actionmanager.h"
#include "reversedebuggermgr.h"

#include <QMenu>

using namespace dpfservice;
using namespace ReverseDebugger::Internal;

void ReverseDebugPlugin::initialize()
{
}

bool ReverseDebugPlugin::start()
{
    auto &ctx = dpfInstance.serviceContext();
    windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    auto actionInit = [&](QAction *action, QString actionID, QKeySequence key, QString iconFileName){
        ActionManager::getInstance()->registerAction(action, actionID, action->text(), key, iconFileName);
        AbstractAction *actionImpl = new AbstractAction(action);
        windowService->addAction(dpfservice::MWM_TOOLS, actionImpl);
    };

    auto reverseDbgAction = new QAction(tr("Reverse debug"));
    QMenu *menu = new QMenu();
    reverseDbgAction->setMenu(menu);
    actionInit(reverseDbgAction, "Tool.Reverse", {}, "");

    auto recoredAction = new QAction(tr("Record"));
    auto replayAction = new QAction(tr("Replay"));

    menu->addAction(recoredAction);
    menu->addAction(replayAction);

    reverseDebug = new ReverseDebuggerMgr(this);
    connect(recoredAction, &QAction::triggered, reverseDebug, &ReverseDebuggerMgr::recored);
    connect(replayAction, &QAction::triggered, reverseDebug, &ReverseDebuggerMgr::replay);

    windowService->addContextWidget(tr("R&everse Debug"), new AbstractWidget(reverseDebug->getWidget()), "Reverse");

    return true;
}

dpf::Plugin::ShutdownFlag ReverseDebugPlugin::stop()
{
    return Sync;
}
