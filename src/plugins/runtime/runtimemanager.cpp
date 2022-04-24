/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "runtimemanager.h"
#include "runtimewidget.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"
#include "runtimewidget.h"
#include <QThread>

using namespace dpfservice;
RuntimeManager *RuntimeManager::instance()
{
    static RuntimeManager instance;
    return &instance;
}

RuntimeManager::RuntimeManager(QObject *parent)
    : QObject(parent), runtimeWidget(new RuntimeWidget)
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        using namespace std::placeholders;
        if (!projectService->showConfigureProjDlg) {
            projectService->showConfigureProjDlg = std::bind(&RuntimeManager::showConfigureProjDlg, this, _1);
        }
    }
}

RuntimeWidget *RuntimeManager::getRuntimeWidget() const
{
    return runtimeWidget;
}

void RuntimeManager::showConfigureProjDlg(QString &projectPath)
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        if (windowService->addCentralNavigation)
            windowService->switchWidgetNavigation(MWNA_RUNTIME);
    }

    runtimeWidget->showPane(RuntimeWidget::kConfigurePane, projectPath);
    QEventLoop eventLoop;
    connect(runtimeWidget, &RuntimeWidget::configureDone, [&]() {
        eventLoop.quit();
    });
    eventLoop.exec();
}
