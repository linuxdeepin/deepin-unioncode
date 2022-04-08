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
#include "workspace.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "workspacedata.h"
#include "workspacecmake.h"
#include "services/workspace/workspaceservice.h"
#include "processdialog.h"
#include <QAction>
#include <QLabel>

using namespace dpfservice;
namespace  {
static WorkspaceData *data = nullptr;
}

void Workspace::initialize()
{
    if (!data)
        data = WorkspaceData::globalInstance();

    WorkspaceObjectFactory::regClass<WorkspaceCMake>(WorkspaceCMake::buildSystemName());

    QString errStr;

    // 发布窗口服务
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(WorkspaceService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }

    using namespace std::placeholders;
    WorkspaceService *workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());

    if (workspaceService) {
        QObject::connect(workspaceService, &WorkspaceService::addWorkspace,
                         data, &WorkspaceData::addWorkspace, Qt::UniqueConnection);

        QObject::connect(workspaceService, &WorkspaceService::delWorkspace,
                         data, &WorkspaceData::delWorkspace, Qt::UniqueConnection);

        if (!workspaceService->findWorkspace) {
            workspaceService->findWorkspace = std::bind(&WorkspaceData::findWorkspace, data, _1);
        }
        if (!workspaceService->targetPath) {
            workspaceService->targetPath = std::bind(&WorkspaceData::targetPath, data, _1);
        }
    }
}

bool Workspace::start()
{
    qInfo() << __FUNCTION__;
    return true;
}

dpf::Plugin::ShutdownFlag Workspace::stop()
{
    return Sync;
}
