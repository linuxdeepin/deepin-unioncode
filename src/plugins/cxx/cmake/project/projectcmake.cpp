// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectcmake.h"
#include "mainframe/cmakegenerator.h"
#include "mainframe/properties/targetsmanager.h"

#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"

#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include <QAction>
#include <QLabel>

using namespace dpfservice;

void ProjectCMake::initialize()
{
}

bool ProjectCMake::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        QString errorString;
        projectService->implGenerator<CmakeGenerator>(CmakeGenerator::toolKitName(), &errorString);
    }

    if (projectService && !projectService->getActiveTarget) {
        using namespace std::placeholders;
        projectService->getActiveTarget = std::bind(&TargetsManager::getActivedTargetByTargetType, TargetsManager::instance(), _1);
    }

    return true;
}

dpf::Plugin::ShutdownFlag ProjectCMake::stop()
{
    return Sync;
}
