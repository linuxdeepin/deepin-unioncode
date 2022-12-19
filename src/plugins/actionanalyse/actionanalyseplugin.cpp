/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
        auto action = new QAction(QAction::tr("User Action Analyse"));
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
