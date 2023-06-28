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

#include <QProcess>
#include <QAction>
#include <QLabel>
#include <QTreeView>

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
        ActionManager::getInstance()->registerAction(action, "Analyze.UsrActionAnalyze", action->text(), QKeySequence());
        action->setCheckable(true);
        action->setChecked(Configure::enabled());
        QObject::connect(action, &QAction::toggled, Configure::setEnabled);
        windowService->addAction(MWM_TOOLS, new AbstractAction(action));
    }

    return true;
}

dpf::Plugin::ShutdownFlag ActionAnalyse::stop()
{
    return Sync;
}
