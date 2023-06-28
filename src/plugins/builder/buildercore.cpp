// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buildercore.h"
#include "mainframe/buildmanager.h"

#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"

#include "base/abstractwidget.h"

using namespace dpfservice;

void BuilderCore::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(BuilderService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool BuilderCore::start()
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());
    auto builderService = ctx.service<BuilderService>(BuilderService::name());
    if (!windowService || !builderService) {
        qCritical() << "Failed, can't found window service or build service";
        abort();
    }

    windowService->addContextWidget(tr("Co&mpile Output"), new AbstractWidget(BuildManager::instance()->getCompileOutputPane()), "Compile");
    windowService->addContextWidget(tr("&Issues"), new AbstractWidget(BuildManager::instance()->getProblemOutputPane()), "Compile");

    using namespace std::placeholders;
    builderService->interface.builderCommand = std::bind(&BuildManager::handleCommand, BuildManager::instance(), _1, _2);
    return true;
}

dpf::Plugin::ShutdownFlag BuilderCore::stop()
{
    return Sync;
}

