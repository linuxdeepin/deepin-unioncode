// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    QAction *action = new QAction(tr("Code Porting"));
    auto inputAction = new AbstractAction(action, this);
    inputAction->setShortCutInfo("Tool.CodePorting", action->text(),QKeySequence());
    connect(action, &QAction::triggered, CodePortingManager::instance(), &CodePortingManager::slotShowConfigWidget);

    windowService->addAction(MWM_TOOLS, inputAction);

    return true;
}

dpf::Plugin::ShutdownFlag CodePortingPlugin::stop()
{
    return Sync;
}
