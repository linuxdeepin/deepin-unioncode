// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gradlebuild.h"

#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

class GradleBuildPrivate
{
    friend class GradleBuild;
};

GradleBuild::GradleBuild(QObject *parent)
    : QObject(parent)
    , d(new GradleBuildPrivate())
{

}

GradleBuild::~GradleBuild()
{
    if (d)
        delete d;
}

QString GradleBuild::build(const QString& kitName, const QString& projectPath)
{
    QString buildUuid;
    auto &ctx = dpfInstance.serviceContext();
    dpfservice::BuilderService *builderService = ctx.service<dpfservice::BuilderService>(dpfservice::BuilderService::name());

    if (builderService) {
        BuildCommandInfo commandInfo;
        commandInfo.kitName = kitName;
        commandInfo.program = OptionManager::getInstance()->getGradleToolPath();
        commandInfo.arguments = QStringList("build");
        commandInfo.workingDir = projectPath;

        buildUuid = commandInfo.uuid;
        builderService->runbuilderCommand({commandInfo}, false);
    }

    return buildUuid;
}


