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
        builderService->interface.builderCommand({commandInfo}, false);
    }

    return buildUuid;
}


