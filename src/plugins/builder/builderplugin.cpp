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
#include "base/abstractmenu.h"
#include "base/abstractmainwindow.h"
#include "base/abstractwidget.h"

#include "services/window/windowservice.h"
#include "service/pluginservicecontext.h"
#include "services/project/projectservice.h"

#include "buildoutputpane.h"
#include "buildmanager.h"
#include "tasks/taskmanager.h"

#include <QMenu>

using namespace dpfservice;

void BuilderPlugin::initialize()
{

}

bool BuilderPlugin::start()
{
    // get window service.
    auto &ctx = dpfInstance.serviceContext();
    windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    BuildManager::instance()->initialize(windowService);

    // instert output pane to window.
    emit windowService->addContextWidget("Co&mpile Output", new AbstractWidget(BuildManager::instance()->getOutputPane()));
    emit windowService->addContextWidget("&Issues", new AbstractWidget(TaskManager::instance()->getView()));

    connect(BuildManager::instance(), &BuildManager::buildStarted, this, &BuilderPlugin::slotBuildStarted);

    // Project tree right menu triggered.
    auto projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        connect(projectService, &ProjectService::targetCommand, this, &BuilderPlugin::slotProjectTreeMenu);
    }

    return true;
}

dpf::Plugin::ShutdownFlag BuilderPlugin::stop()
{
    return Sync;
}

void BuilderPlugin::slotBuildStarted()
{
    // get window service.
    if (windowService) {
        emit windowService->switchWidgetContext("Co&mpile Output");
    }
}

void BuilderPlugin::slotProjectTreeMenu(const QString &program, const QStringList &arguments)
{
    auto buildstep = BuildManager::instance()->makeCommandStep(program, arguments);
    BuildManager::instance()->buildList({buildstep}, program + " " + arguments.join(" "));
}
