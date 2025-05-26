// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
            args.removeAll("");
            BuildCommandInfo commandInfo;
            commandInfo.kitName = kitName;
            commandInfo.program = target.buildCommand;
            commandInfo.arguments = args;
            commandInfo.workingDir = target.workingDir;

            buildUuid = commandInfo.uuid;
            builderService->runbuilderCommand({commandInfo}, false);
        }
    }

    return buildUuid;
}



