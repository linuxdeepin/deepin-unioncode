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

    windowService->addContextWidget(tr("Reverse Debug"), new AbstractWidget(reverseDebug->getWidget()));

    return true;
}

dpf::Plugin::ShutdownFlag ReverseDebugPlugin::stop()
{
    return Sync;
}
