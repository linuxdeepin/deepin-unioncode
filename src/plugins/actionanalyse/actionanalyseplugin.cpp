// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionanalyseplugin.h"
#include "mainframe/configure.h"
#include "mainframe/analysekeeper.h"

#include "common/common.h"
#include "base/abstractaction.h"
#include "services/window/windowservice.h"
#include "services/window/windowelement.h"

#include <QAction>

void ActionAnalyse::initialize()
{

}

bool ActionAnalyse::start()
{
    qInfo() << __FUNCTION__;
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        auto action = new QAction(MWMTA_USR_ACTION_ANALYZE);
        action->setCheckable(true);
        action->setChecked(Configure::enabled());
        QObject::connect(action, &QAction::toggled, Configure::setEnabled);

        auto inputAction = new AbstractAction(action);
        inputAction->setShortCutInfo("Analyze.UsrActionAnalyze", action->text());
        windowService->addAction(MWM_TOOLS, inputAction);
    }

    return true;
}

dpf::Plugin::ShutdownFlag ActionAnalyse::stop()
{
    return Sync;
}
