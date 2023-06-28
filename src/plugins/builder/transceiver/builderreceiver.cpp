// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "builderreceiver.h"
#include "mainframe/buildmanager.h"
#include "common/common.h"
#include "services/project/projectservice.h"

BuilderReceiver::BuilderReceiver(QObject *parent)
    : dpf::EventHandler(parent)
{

}

dpf::EventHandler::Type BuilderReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList BuilderReceiver::topics()
{
    return {project.topic, T_DEBUGGER, symbol.topic};
}

void BuilderReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.activedProject.name) {
        QVariant proInfoVar = event.property(project.activedProject.pKeys[0]);
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        BuildManager::instance()->setActivedProjectInfo(projectInfo.kitName(), projectInfo.workspaceFolder());
    } else if (event.data() == project.createdProject.name) {
        QVariant proInfoVar = event.property(project.createdProject.pKeys[0]);
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        BuildManager::instance()->setActivedProjectInfo(projectInfo.kitName(), projectInfo.workspaceFolder());
    } else if (event.data() == project.deletedProject.name) {
        QVariant proInfoVar = event.property(project.deletedProject.pKeys[0]);
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        BuildManager::instance()->clearActivedProjectInfo();
    } else if (event.data() == symbol.parseDone.name) {
        bool bSuccess = event.property("success").toBool();
        if(!bSuccess) {
            QString workspace = event.property("workspace").toString();
            QString language = event.property("language").toString();
            const QString &storage = workspace;

            QString errMsg = QDialog::tr(
                        "Error parsing project symbol\n"
                        "workspace: %0\n"
                        "language: %1\n"
                        "storage: %2\n")
                    .arg(workspace)
                    .arg(language)
                    .arg(storage);

            BuildManager::instance()->addOutput(errMsg, OutputPane::ErrorMessage);
        }
    }
}

