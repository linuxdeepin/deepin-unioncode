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
    return {project.topic}; //绑定menu 事件
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
        navigation.doSwitch(dpfservice::MWNA_EDIT);
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
    }
}


