/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<huangyub@uniontech.com>
 *             hongjinchuan<hongjinchuan@uniontech.com>
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
#include "codeportingplugin.h"
#include "codeportingmanager.h"
#include "configwidget.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "common/actionmanager/actionmanager.h"

#include <QMenu>

using namespace dpfservice;

void CodePortingPlugin::initialize()
{
}

bool CodePortingPlugin::start()
{
    auto &ctx = dpfInstance.serviceContext();
    windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    // Add code porting item in tool menu.
    QAction *action = new QAction(tr("&Code Porting"));
    ActionManager::getInstance()->registerAction(action, "Tool.CodePorting", action->text(),QKeySequence(), "code_porting.png");
    connect(action, &QAction::triggered, CodePortingManager::instance(), &CodePortingManager::slotShowConfigWidget);

    AbstractAction *actionImpl = new AbstractAction(action);
    windowService->addAction(MWM_TOOLS, actionImpl);

    // Add output pane
    windowService->addContextWidget(tr("Code Porting"), new AbstractWidget(CodePortingManager::instance()->getOutputPane()));

    // Add report pane
    windowService->addContextWidget(tr("Porting &Report"), new AbstractWidget(CodePortingManager::instance()->getReportPane()));

    return true;
}

dpf::Plugin::ShutdownFlag CodePortingPlugin::stop()
{
    return Sync;
}
