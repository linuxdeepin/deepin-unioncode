// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
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

constexpr char A_EVENT_RECORDER_RECORD[] = "EventRecorder.Action.Record";
constexpr char A_EVENT_RECORDER_REPLAY[] = "EventRecorder.Action.Replay";

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
    auto mEventRecorder = ActionManager::instance()->createContainer(M_TOOLS_EVENTRECORDER);
    mEventRecorder->menu()->setTitle(tr("Event recorder"));
    mTools->addMenu(mEventRecorder);

    auto actionInit = [&](QAction *action, QString actionID) {
        auto cmd = ActionManager::instance()->registerAction(action, actionID);
        mEventRecorder->addAction(cmd);
    };

    auto recoredAction = new QAction(tr("Record"), mEventRecorder);
    actionInit(recoredAction, A_EVENT_RECORDER_RECORD);
    auto replayAction = new QAction(tr("Replay"), mEventRecorder);
    actionInit(replayAction, A_EVENT_RECORDER_REPLAY);

    reverseDebug = new ReverseDebuggerMgr(this);
    connect(recoredAction, &QAction::triggered, reverseDebug, &ReverseDebuggerMgr::recored);
    connect(replayAction, &QAction::triggered, reverseDebug, &ReverseDebuggerMgr::replay);

    windowService->addContextWidget(tr("Ev&ent Recorder"), new AbstractWidget(reverseDebug->getWidget()), false);

    return true;
}

dpf::Plugin::ShutdownFlag ReverseDebugPlugin::stop()
{
    return Sync;
}
