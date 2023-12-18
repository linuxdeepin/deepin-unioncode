// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectcmakereceiver.h"
#include "cmake/project/mainframe/cmakeprojectgenerator.h"
#include "cmake/project/mainframe/properties/targetsmanager.h"

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
        TargetsManager::instance()->readTargets(projectInfo.buildFolder(), projectInfo.workspaceFolder());
    }
}

void ProjectCmakeReceiver::builderEvent(const dpf::Event &event)
{
    if (event.data() == D_BUILD_STATE) {
        BuildCommandInfo commandInfo = qvariant_cast<BuildCommandInfo>(event.property(P_ORIGINCMD));
        if (ProjectCmakeProxy::instance()->getBuildCommandUuid() == commandInfo.uuid) {
            int endStatus = event.property(P_STATE).toInt();
            if (0 != endStatus) {
                qWarning() << "Build failed with state:" << endStatus;
            }
            emit ProjectCmakeProxy::instance()->buildExecuteEnd(commandInfo);
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
