// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavenbuild.h"

#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

class MavenBuildPrivate
{
    friend class MavenBuild;
};

MavenBuild::MavenBuild(QObject *parent)
    : QObject(parent)
    , d(new MavenBuildPrivate())
{

}

MavenBuild::~MavenBuild()
{
    if (d)
        delete d;
}

QString MavenBuild::build(const QString& kitName, const QString& projectPath)
{
    QString buildUuid;
    auto &ctx = dpfInstance.serviceContext();
    dpfservice::BuilderService *builderService = ctx.service<dpfservice::BuilderService>(dpfservice::BuilderService::name());

    if (builderService) {
        BuildCommandInfo commandInfo;
        commandInfo.kitName = kitName;
        commandInfo.program = OptionManager::getInstance()->getMavenToolPath();
        commandInfo.arguments = QStringList("compile");
        commandInfo.workingDir = projectPath;

        buildUuid = commandInfo.uuid;
        builderService->runbuilderCommand({commandInfo}, false);
    }

    return buildUuid;
}


