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
        auto inputAction = new AbstractAction(action);
        inputAction->setShortCutInfo("Tools.Binary", action->text());
        windowService->addAction(MWM_TOOLS, inputAction);

        QObject::connect(action, &QAction::triggered, [=](){
           static BinaryToolsDialog dlg;
           dlg.exec();
        });
    }

    return true;
}

dpf::Plugin::ShutdownFlag BinaryTools::stop()
{
    return Sync;
}
