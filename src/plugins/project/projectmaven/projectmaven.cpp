/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "projectmaven.h"
#include "mainframe/mavenopenhandler.h"
#include "mainframe/mavengenerator.h"
#include "transceiver/sendevents.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"

#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include <QAction>
#include <QLabel>

using namespace dpfservice;

void ProjectMaven::initialize()
{
}

bool ProjectMaven::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        QString errorString;
        projectService->implGenerator<MavenGenerator>(MavenGenerator::toolKitName(), &errorString);
    }

    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        if (windowService->addOpenProjectAction) {
            auto action = new AbstractAction(CmakeOpenHandler::instance()->openAction());
            windowService->addOpenProjectAction(MWMFA_JAVA, action);
        }
    }

    return true;
}

dpf::Plugin::ShutdownFlag ProjectMaven::stop()
{
    return Sync;
}
