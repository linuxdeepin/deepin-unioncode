// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytools.h"
#include "mainframe/binarytoolsdialog.h"

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
        ActionManager::getInstance()->registerAction(action, "Tools.Binary", action->text(), QKeySequence());
        windowService->addAction(MWM_TOOLS, new AbstractAction(action));

        QObject::connect(action, &QAction::triggered, [=](){
           BinaryToolsDialog dlg;
           dlg.exec();
        });
    }

    return true;
}

dpf::Plugin::ShutdownFlag BinaryTools::stop()
{
    return Sync;
}
