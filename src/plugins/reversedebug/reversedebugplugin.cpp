// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reversedebugplugin.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "common/util/eventdefinitions.h"
#include "common/actionmanager/actionmanager.h"
#include "common/actionmanager/actioncontainer.h"
#include "reversedebuggermgr.h"

#include <DMenu>

constexpr char A_REVERSE_DEBUG_RECORD[] = "ReverseDebug.Action.Record";
constexpr char A_REVERSE_DEBUG_REPLAY[] = "ReverseDebug.Action.Replay";

DWIDGET_USE_NAMESPACE
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

    auto mTools = ActionManager::instance()->actionContainer(M_TOOLS);
    auto mReverseDbg = ActionManager::instance()->createContainer(M_TOOLS_REVERSEDEBUG);
    mReverseDbg->menu()->setTitle(tr("Reverse debug"));
    mTools->addMenu(mReverseDbg);

    auto actionInit = [&](QAction *action, QString actionID) {
        auto cmd = ActionManager::instance()->registerAction(action, actionID);
        mReverseDbg->addAction(cmd);
    };

    auto recoredAction = new QAction(tr("Record"), mReverseDbg);
    actionInit(recoredAction, A_REVERSE_DEBUG_RECORD);
    auto replayAction = new QAction(tr("Replay"), mReverseDbg);
    actionInit(replayAction, A_REVERSE_DEBUG_REPLAY);

    reverseDebug = new ReverseDebuggerMgr(this);
    connect(recoredAction, &QAction::triggered, reverseDebug, &ReverseDebuggerMgr::recored);
    connect(replayAction, &QAction::triggered, reverseDebug, &ReverseDebuggerMgr::replay);

    windowService->addContextWidget(tr("R&everse Debug"), new AbstractWidget(reverseDebug->getWidget()), false);

    return true;
}

dpf::Plugin::ShutdownFlag ReverseDebugPlugin::stop()
{
    return Sync;
}
