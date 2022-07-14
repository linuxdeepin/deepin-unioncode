/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

    windowService->addContextWidget("Co&mpile Output", new AbstractWidget(BuildManager::instance()->getCompileOutputPane()));
    windowService->addContextWidget("&Issues", new AbstractWidget(BuildManager::instance()->getProblemOutputPane()));

    using namespace std::placeholders;
    builderService->interface.builderCommand = std::bind(&BuildManager::handleCommand, BuildManager::instance(), _1, _2);
    return true;
}

dpf::Plugin::ShutdownFlag BuilderCore::stop()
{
    return Sync;
}

