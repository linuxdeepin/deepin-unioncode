// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectcmakereceiver.h"
#include "cmakeprojectgenerator.h"
#include "targetsmanager.h"
#include "common/common.h"

#include "services/project/projectservice.h"
#include "services/window/windowelement.h"
#include "services/project/projectgenerator.h"
#include "services/builder/builderglobals.h"

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
        builderEvent(event);
    }

    if (event.data() == project.activatedProject.name) {
        QVariant proInfoVar = event.property(project.activatedProject.pKeys[0]);
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        TargetsManager::instance()->readTargets(projectInfo.buildFolder(), projectInfo.workspaceFolder());
        emit ProjectCmakeProxy::instance()->projectedChanged(projectInfo);
    }

    if (event.data() == project.fileDeleted.name) {
        QVariant varKit = event.property("kit");

        if (varKit == "cmake") {
            QVariant varFilePath = event.property("filePath");
            QString filePath = varFilePath.toString();
            emit ProjectCmakeProxy::instance()->fileDeleted(filePath);
        }
    }

    if (event.data() == project.projectUpdated.name) {
        QVariant proInfoVar = event.property("projectInfo");
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        if (projectInfo.kitName() != CmakeProjectGenerator::toolKitName())
            return;

        auto *param = config::ConfigUtil::instance()->getConfigureParamPointer();
        auto iter = param->buildTypeConfigures.begin();
        for (; iter != param->buildTypeConfigures.end(); ++iter) {
            if (param->defaultType == iter->type)
                iter->runConfigure.defaultTargetName = projectInfo.currentProgram();
        }
        config::ConfigUtil::instance()->updateProjectInfo(projectInfo, param);
        dpfGetService(dpfservice::ProjectService)->updateProjectInfo(projectInfo);
    }

    if (event.data() == project.openProjectPropertys.name) {
        QVariant proInfoVar = event.property("projectInfo");
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);

        emit ProjectCmakeProxy::instance()->openProjectPropertys(projectInfo);
    }

    if (event.data() == project.projectNodeExpanded.name) {
        auto index = event.property("modelIndex").value<QModelIndex>();
        auto filePath = index.data(Qt::ToolTipRole).toString();
        emit ProjectCmakeProxy::instance()->nodeExpanded(filePath);
    }

    if (event.data() == project.projectNodeCollapsed.name) {
        auto index = event.property("modelIndex").value<QModelIndex>();
        auto filePath = index.data(Qt::ToolTipRole).toString();
        emit ProjectCmakeProxy::instance()->nodeCollapsed(filePath);
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
