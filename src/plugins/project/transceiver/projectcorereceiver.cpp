// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectcorereceiver.h"
#include "common/common.h"
#include "mainframe/projecttree.h"
#include "mainframe/projectkeeper.h"
#include "services/project/projectservice.h"
#include "services/window/windowelement.h"
#include "services/session/sessionservice.h"

using namespace dpfservice;

ProjectCoreReceiver::ProjectCoreReceiver(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<ProjectCoreReceiver>()
{
    using namespace std::placeholders;
    eventHandleMap.insert(project.activeProject.name, std::bind(&ProjectCoreReceiver::processActiveProjectEvent, this, _1));
    eventHandleMap.insert(project.openProject.name, std::bind(&ProjectCoreReceiver::processOpenProjectEvent, this, _1));
    eventHandleMap.insert(project.activatedProject.name, std::bind(&ProjectCoreReceiver::processActivatedProjectEvent, this, _1));
    eventHandleMap.insert(project.openProjectByPath.name, std::bind(&ProjectCoreReceiver::processOpenProjectByPathEvent, this, _1));
    eventHandleMap.insert(workspace.expandAll.name, std::bind(&ProjectCoreReceiver::processExpandAllEvent, this, _1));
    eventHandleMap.insert(workspace.foldAll.name, std::bind(&ProjectCoreReceiver::processFoldAllEvent, this, _1));
    eventHandleMap.insert(editor.switchedFile.name, std::bind(&ProjectCoreReceiver::processSwitchedFileEvent, this, _1));
    eventHandleMap.insert(uiController.modeRaised.name, std::bind(&ProjectCoreReceiver::processModeRaisedEvent, this, _1));
    eventHandleMap.insert(session.sessionLoaded.name, std::bind(&ProjectCoreReceiver::processSessionLoadedEvent, this, _1));
    eventHandleMap.insert(session.readyToSaveSession.name, std::bind(&ProjectCoreReceiver::processReadyToSaveSessionEvent, this, _1));
}

dpf::EventHandler::Type ProjectCoreReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ProjectCoreReceiver::topics()
{
    return { project.topic, workspace.topic, editor.topic, uiController.topic, session.topic };   //绑定menu 事件
}

void ProjectCoreReceiver::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void ProjectCoreReceiver::processActiveProjectEvent(const dpf::Event &event)
{
    QString workspace = event.property("workspace").toString();
    ProjectKeeper::instance()->treeView()->activeProjectInfo(workspace);
}

void ProjectCoreReceiver::processOpenProjectEvent(const dpf::Event &event)
{
    auto sessionSrv = dpfGetService(SessionService);
    sessionSrv->markSessionFileDirty();
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
                generator->doProjectOpen(kitName, language, workspace);
            } else if (generator->isOpenedProject(kitName, language, workspace)) {
                project.activeProject(kitName, language, workspace);
            }
        }
    }
}

void ProjectCoreReceiver::processExpandAllEvent(const dpf::Event &event)
{
    ProjectKeeper::instance()->treeView()->expandAll();
}

void ProjectCoreReceiver::processFoldAllEvent(const dpf::Event &event)
{
    ProjectKeeper::instance()->treeView()->collapseAll();
}

void ProjectCoreReceiver::processActivatedProjectEvent(const dpf::Event &event)
{
    QVariant proInfoVar = event.property("projectInfo");
    dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);

    emit ProjectProxy::instance()->projectActivated(projectInfo);
}

void ProjectCoreReceiver::processSwitchedFileEvent(const dpf::Event &event)
{
    emit ProjectProxy::instance()->switchedFile(event.property("fileName").toString());
    ProjectKeeper::instance()->treeView()->selectProjectFile(event.property("fileName").toString());
}

void ProjectCoreReceiver::processModeRaisedEvent(const dpf::Event &event)
{
    auto mode = event.property("mode").toString();
    emit ProjectProxy::instance()->modeRaised(mode);
}

void ProjectCoreReceiver::processOpenProjectByPathEvent(const dpf::Event &event)
{
    auto directory = event.property("directory").toString();
    emit ProjectProxy::instance()->openProject(directory);
}

void ProjectCoreReceiver::processSessionLoadedEvent(const dpf::Event &event)
{
    uiController.doSwitch(dpfservice::MWNA_EDIT);
    auto sessionSrv = dpfGetService(SessionService);
    auto view = ProjectKeeper::instance()->treeView();
    view->closeAllProjects();

    auto prjList = sessionSrv->value("ProjectList").toList();
    for (const auto &info : prjList) {
        auto map = info.toMap();
        dpf::Event e(project.topic);
        e.setProperty("kitName", map.value("KitName"));
        e.setProperty("language", map.value("Language"));
        e.setProperty("workspace", map.value("Workspace"));

        processOpenProjectEvent(e);
    }

    if (prjList.size() > 1) {
        auto activePrj = sessionSrv->value("ActiveProject").toString();
        view->activeProjectInfo(activePrj);
    }
}

void ProjectCoreReceiver::processReadyToSaveSessionEvent(const dpf::Event &event)
{
    auto sessionSrv = dpfGetService(SessionService);
    Q_ASSERT(sessionSrv);

    QVariantList infoList;
    auto view = ProjectKeeper::instance()->treeView();
    const auto &prjList = view->getAllProjectInfo();
    for (const auto &prj : prjList) {
        QVariantMap map;
        map.insert("KitName", prj.kitName());
        map.insert("Workspace", prj.workspaceFolder());
        map.insert("Language", prj.language());

        infoList << map;
    }
    sessionSrv->setValue("ProjectList", infoList);
    sessionSrv->setValue("ActiveProject", view->getActiveProjectInfo().workspaceFolder());
}
