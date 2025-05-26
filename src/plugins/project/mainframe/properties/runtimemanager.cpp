// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
//    auto &ctx = dpfInstance.serviceContext();
//    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
//    if (projectService) {
//        using namespace std::placeholders;
//        if (!projectService->showConfigureProjDlg) {
//            projectService->showConfigureProjDlg = std::bind(&RuntimeManager::showConfigureProjDlg, this, _1);
//        }
//    }
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
    static QEventLoop eventLoop;
    connect(runtimeWidget, &RuntimeWidget::configureDone, [=]() {
        eventLoop.quit();
    });
    eventLoop.exec();
}
