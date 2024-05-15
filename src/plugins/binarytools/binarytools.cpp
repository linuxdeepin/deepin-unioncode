// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytools.h"
#include "mainframe/binarytoolsdialog.h"
#include "configure/binarytoolsmanager.h"

#include "common/common.h"
#include "base/abstractaction.h"
#include "services/window/windowservice.h"

void BinaryTools::initialize()
{
}

bool BinaryTools::start()
{
    qInfo() << __FUNCTION__;
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        auto action = new QAction(MWMTA_BINARY_TOOLS);
        auto inputAction = new AbstractAction(action, this);
        inputAction->setShortCutInfo("Tools.Binary", action->text());
        windowService->addAction(MWM_TOOLS, inputAction);

        toolMenu = new QMenu();
        action->setMenu(toolMenu);
        BinaryToolsManager::instance()->setToolMenu(toolMenu);
    }

    const auto &tools = BinaryToolsManager::instance()->tools();
    BinaryToolsManager::instance()->checkAndAddToToolbar(tools);
    BinaryToolsManager::instance()->updateToolMenu(tools);

    return true;
}

dpf::Plugin::ShutdownFlag BinaryTools::stop()
{
    if (toolMenu)
        toolMenu->deleteLater();

    return Sync;
}
