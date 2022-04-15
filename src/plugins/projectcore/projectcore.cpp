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
#include "mainframe/projectkeeper.h"
#include "mainframe/projecttreeview.h"
#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "services/workspace/workspaceservice.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
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
    toolchains::cxx::generatGlobalFile();
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        if (windowService->addWidgetWorkspace) {
            windowService->addWidgetWorkspace(MWCWT_PROJECTS,
                                              new AbstractWidget(ProjectKeeper::instance()->treeView()));
        }
    }

    using namespace std::placeholders;
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        if (!projectService->addProjectRootItem) {
            projectService->addProjectRootItem = std::bind(&ProjectTreeView::appendRootItem,
                                                           ProjectKeeper::instance()->treeView(), _1);
        }
        if (!projectService->expandedProjectDepth) {
            projectService->expandedProjectDepth = std::bind(&ProjectTreeView::expandedProjectDepth,
                              ProjectKeeper::instance()->treeView(), _1, _2);
        }
        if (!projectService->expandedProjectAll) {
            projectService->expandedProjectAll = std::bind(&ProjectTreeView::expandedProjectAll,
                                                           ProjectKeeper::instance()->treeView(), _1);
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag ProjectCore::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}
