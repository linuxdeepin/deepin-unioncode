// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectcorereceiver.h"
#include "common/common.h"
#include "mainframe/projecttree.h"
#include "mainframe/projectkeeper.h"
#include "services/project/projectservice.h"
#include "services/window/windowelement.h"

ProjectCoreReceiver::ProjectCoreReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<ProjectCoreReceiver> ()
{

}

dpf::EventHandler::Type ProjectCoreReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ProjectCoreReceiver::topics()
{
    return {project.topic, workspace.topic}; //绑定menu 事件
}

void ProjectCoreReceiver::eventProcess(const dpf::Event &event)
{
    using namespace dpfservice;
    if (event.data() == project.activeProject.name) {
        auto infos = ProjectKeeper::instance()->treeView()->getAllProjectInfo();
        QString kitName = event.property(project.openProject.pKeys[0]).toString();
        QString language = event.property(project.openProject.pKeys[1]).toString();
        QString workspace = event.property(project.openProject.pKeys[2]).toString();
        ProjectKeeper::instance()->treeView()->activeProjectInfo(kitName, language, workspace);
    } else if (event.data() == project.openProject.name) {
        uiController.doSwitch(dpfservice::MWNA_EDIT);
        auto &ctx = dpfInstance.serviceContext();
        ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
        if (projectService) {
            // "kitName", "language", "workspace"
            QString kitName = event.property(project.openProject.pKeys[0]).toString();
            QString language = event.property(project.openProject.pKeys[1]).toString();
            QString workspace = event.property(project.openProject.pKeys[2]).toString();
            auto generator = projectService->createGenerator<ProjectGenerator>(kitName);
            if (!generator)
                return;
            QStringList supportLangs = generator->supportLanguages();
            if (supportLangs.contains(language)) {
                if (generator->canOpenProject(kitName, language, workspace)) {
                    generator->doProjectOpen(language, kitName, workspace);
                } else if (generator->isOpenedProject(kitName, language, workspace)) {
                    project.activeProject(kitName, language, workspace);
                }
            }
        }
    } else if (event.data() == workspace.expandAll.name) {
        ProjectKeeper::instance()->treeView()->expandAll();
    } else if (event.data() == workspace.foldAll.name) {
        ProjectKeeper::instance()->treeView()->collapseAll();
    }
}


