/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "projectcmakereceiver.h"
#include "mainframe/cmakegenerator.h"
#include "mainframe/properties/targetsmanager.h"

#include "services/project/projectinfo.h"
#include "services/project/projectservice.h"
#include "services/window/windowelement.h"
#include "services/project/projectgenerator.h"
#include "services/builder/builderglobals.h"
#include "common/common.h"

ProjectCmakeReceiver::ProjectCmakeReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<ProjectCmakeReceiver> ()
{

}

dpf::EventHandler::Type ProjectCmakeReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ProjectCmakeReceiver::topics()
{
    return { T_BUILDER, project.topic};
}

void ProjectCmakeReceiver::eventProcess(const dpf::Event &event)
{
    if (event.topic() == T_BUILDER) {
        navigation.doSwitch(dpfservice::MWNA_EDIT);
        builderEvent(event);
    }

    if (event.data() == project.activedProject.name) {
        QVariant proInfoVar = event.property(project.activedProject.pKeys[0]);
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        TargetsManager::instance()->initialize(projectInfo.buildFolder());
    }
}

void ProjectCmakeReceiver::builderEvent(const dpf::Event &event)
{
    if (event.data() == D_BUILD_STATE) {
        BuildCommandInfo commandInfo = qvariant_cast<BuildCommandInfo>(event.property(P_ORIGINCMD));
        if (ProjectCmakeProxy::instance()->getBuildCommandUuid() == commandInfo.uuid) {
            int endStatus = event.property(P_STATE).toInt();
            if (0 == endStatus) {
                emit ProjectCmakeProxy::instance()->buildExecuteEnd(commandInfo);
            } else {
                ContextDialog::ok(QDialog::tr("Failed open project, with build step."));
            }
        }
    }
}

ProjectCmakeProxy *ProjectCmakeProxy::instance()
{
    static ProjectCmakeProxy ins;
    return &ins;
}

void ProjectCmakeProxy::setBuildCommandUuid(QString buildCommandUuid)
{
    ProjectCmakeProxy::buildCommandUuid = buildCommandUuid;
}

QString ProjectCmakeProxy::getBuildCommandUuid()
{
    return buildCommandUuid;
}
