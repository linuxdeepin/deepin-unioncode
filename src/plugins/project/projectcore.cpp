// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectcore.h"
#include "transceiver/sendevents.h"
#include "mainframe/projectkeeper.h"
#include "mainframe/projecttree.h"
#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include <QProcess>
#include <QAction>
#include <QLabel>
#include <QTreeView>

using namespace dpfservice;
void ProjectCore::initialize()
{
    qInfo() << __FUNCTION__;
    // 发布工程服务
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(ProjectService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool ProjectCore::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        if (windowService->addWidgetWorkspace) {
            auto view = new AbstractWidget(ProjectKeeper::instance()->treeView());
            windowService->addWidgetWorkspace(MWCWT_PROJECTS, view, "edit-project");
        }
    }

    if (windowService && windowService->addCentralNavigation) {
        //windowService->addCentralNavigation(MWNA_RUNTIME, new AbstractCentral(RuntimeManager::instance()->getRuntimeWidget()));
    }

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                     this, &ProjectCore::pluginsStartedMain, Qt::DirectConnection);


    using namespace std::placeholders;
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        ProjectTree *treeView = ProjectKeeper::instance()->treeView();
        if (!projectService->projectView.addRootItem) {
            projectService->projectView.addRootItem
                    = std::bind(&ProjectTree::appendRootItem, treeView, _1);
        }
        if (!projectService->projectView.removeRootItem) {
            projectService->projectView.removeRootItem
                    = std::bind(&ProjectTree::removeRootItem, treeView, _1);
        }
        if (!projectService->projectView.expandedDepth) {
            projectService->projectView.expandedDepth
                    = std::bind(&ProjectTree::expandedProjectDepth, treeView, _1, _2);
        }
        if (!projectService->projectView.expandedAll) {
            projectService->projectView.expandedAll
                    = std::bind(&ProjectTree::expandedProjectAll, treeView, _1);
        }
        if (!projectService->getAllProjectInfo) {
            projectService->getAllProjectInfo
                    = std::bind(&ProjectTree::getAllProjectInfo, treeView);
        }
        if (!projectService->getProjectInfo) {
            projectService->getProjectInfo
                    = std::bind(&ProjectTree::getProjectInfo, treeView, _1, _2);
        }
        if (!projectService->getActiveProjectInfo) {
            projectService->getActiveProjectInfo
                    = std::bind(&ProjectTree::getActiveProjectInfo, treeView);
        }
        if (!projectService->hasProjectInfo) {
            projectService->hasProjectInfo
                    = std::bind(&ProjectTree::hasProjectInfo, treeView, _1);
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag ProjectCore::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

void ProjectCore::pluginsStartedMain()
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (projectService && windowService) {
        QStringList kitNames = projectService->supportGeneratorName<ProjectGenerator>();
        for (auto kitName : kitNames) {
            auto generator = projectService->createGenerator<ProjectGenerator>(kitName);
            if (generator) {
                for(auto lang : generator->supportLanguages()) {
                    auto action = generator->openProjectAction(lang, kitName);
                    if (action)
                        windowService->addOpenProjectAction(lang, new AbstractAction(action));
                }
                QObject::connect(generator, &ProjectGenerator::itemChanged,
                                 ProjectKeeper::instance()->treeView(),
                                 &ProjectTree::itemModified, Qt::UniqueConnection);
            }
        }
    }
}
