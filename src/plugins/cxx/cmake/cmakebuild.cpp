/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "cmakebuild.h"

#include "services/builder/builderservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;

class CMakeBuildPrivate
{
    friend class CMakeBuild;
};

CMakeBuild::CMakeBuild(QObject *parent)
    : QObject(parent)
    , d(new CMakeBuildPrivate())
{

}

CMakeBuild::~CMakeBuild()
{
    if (d)
        delete d;
}

QString CMakeBuild::build(const QString& kitName, const QString& projectPath)
{
    Q_UNUSED(projectPath)
    QString buildUuid;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());

    if (builderService && projectService
            && projectService->getActiveTarget) {
        auto target = projectService->getActiveTarget(TargetType::kBuildTarget);
        if (!target.buildCommand.isEmpty()) {
            QStringList args = target.buildArguments << target.buildTarget;
            BuildCommandInfo commandInfo;
            commandInfo.kitName = kitName;
            commandInfo.program = target.buildCommand;
            commandInfo.arguments = args;
            commandInfo.workingDir = target.outputPath;

            buildUuid = commandInfo.uuid;
            builderService->interface.builderCommand(commandInfo);
        }
    }

    return buildUuid;
}

QString CMakeBuild::getTargetPath()
{
    QString targetPath;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && projectService->getActiveTarget) {
        auto target = projectService->getActiveTarget(kActiveExecTarget);
        targetPath = target.outputPath + QDir::separator() + target.path + QDir::separator() + target.buildTarget;
    }

    return targetPath;
}


