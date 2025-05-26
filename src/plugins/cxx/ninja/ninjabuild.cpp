// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjabuild.h"

#include "services/builder/builderservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;

class NinjaBuildPrivate
{
    friend class NinjaBuild;
};

NinjaBuild::NinjaBuild(QObject *parent)
    : QObject(parent)
    , d(new NinjaBuildPrivate())
{

}

NinjaBuild::~NinjaBuild()
{
    if (d)
        delete d;
}

QString NinjaBuild::build(const QString& kitName, const QString& projectPath)
{
    Q_UNUSED(projectPath)
    QString buildUuid;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());

    if (builderService) {
        BuildCommandInfo commandInfo;
        commandInfo.kitName = kitName;
        commandInfo.program = OptionManager::getInstance()->getNinjaToolPath();
        commandInfo.arguments = QStringList("all");
        commandInfo.workingDir = projectPath;

        buildUuid = commandInfo.uuid;
        builderService->runbuilderCommand({commandInfo}, false);
    }

    return buildUuid;
}



