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
#include "builderplugin.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"
#include "base/abstractmainwindow.h"
#include "base/abstractwidget.h"

#include "services/window/windowservice.h"
#include "services/window/windowservice.h"
#include "service/pluginservicecontext.h"

#include "project.h"
#include "buildoutputpane.h"
#include "buildtarget.h"
#include "buildmanager.h"
#include "buildersignals.h"
#include "builderglobals.h"


#include <QMenu>

using namespace dpfservice;

void BuilderPlugin::initialize()
{
}

bool BuilderPlugin::start()
{
    // get window service.
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    // insert build action.
    QAction *action = new QAction("build");
    AbstractAction *actionImpl = new AbstractAction(action);
    windowService->addAction(QString::fromStdString(MENU_BUILD), actionImpl);
    // triggered by top menu.
    connect(action, &QAction::triggered, this, &BuilderPlugin::buildProjects, Qt::DirectConnection);
    // triggerd by right menu which tree item.
    connect(builderSignals, &BuilderSignals::buildTriggered, this, &BuilderPlugin::buildProjects, Qt::DirectConnection);

    // instert output pane to window.
    emit windowService->addContextWidget("Output", new AbstractWidget(BuildManager::instance()->getOutputPane()));

    project.reset(new Project(this));

    return true;
}

dpf::Plugin::ShutdownFlag BuilderPlugin::stop()
{
    return Sync;
}

void BuilderPlugin::buildProjects()
{
    // TODO(mozart):steps should get from other place.
    auto buildList = project->activeTarget()->getbuildSteps();
    if (buildList.size() > 0)
        BuildManager::instance()->buildList(buildList);
    else {
        BuildManager::instance()->getOutputPane()->appendText("Nothing to do.");
    }
}
