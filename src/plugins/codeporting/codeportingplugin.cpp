// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeportingplugin.h"
#include "codeportingmanager.h"
#include "configwidget.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "common/actionmanager/actionmanager.h"
#include "common/actionmanager/actioncontainer.h"

#include <QMenu>

constexpr char A_CODE_PORTING[] = "CodePorting.Base";

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
    auto mTools = ActionManager::instance()->actionContainer(M_TOOLS);

    QAction *action = new QAction(tr("Code Porting"), mTools);
    auto cmd = ActionManager::instance()->registerAction(action, A_CODE_PORTING);
    mTools->addAction(cmd);
    connect(action, &QAction::triggered, CodePortingManager::instance(), &CodePortingManager::slotShowConfigWidget);

    return true;
}

dpf::Plugin::ShutdownFlag CodePortingPlugin::stop()
{
    return Sync;
}
