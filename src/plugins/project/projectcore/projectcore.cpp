/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
            windowService->addWidgetWorkspace(MWCWT_PROJECTS, view);
        }
    }

    if (windowService && windowService->addCentralNavigation) {
        //windowService->addCentralNavigation(MWNA_RUNTIME, new AbstractCentral(RuntimeManager::instance()->getRuntimeWidget()));
    }

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                     this, &ProjectCore::pluginsStartedMain);


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
        if (!projectService->projectView.getAllProjectInfo) {
            projectService->projectView.getAllProjectInfo
                    = std::bind(&ProjectTree::getAllProjectInfo, treeView);
        }
        if (!projectService->projectView.getProjectInfo) {
            projectService->projectView.getProjectInfo
                    = std::bind(&ProjectTree::getProjectInfo, treeView, _1, _2);
        }
        if (!projectService->projectView.getActiveProjectInfo) {
            projectService->projectView.getActiveProjectInfo
                    = std::bind(&ProjectTree::getActiveProjectInfo, treeView);
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
