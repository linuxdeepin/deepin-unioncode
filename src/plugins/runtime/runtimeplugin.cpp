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
#include "runtimeplugin.h"
#include "runtimewidget.h"
#include "common/common.h"
#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/runtime/runtimeservice.h"
#include "projectparser.h"
#include "runtimemanager.h"
#include "configureprojpane.h"
#include "kitmanager.h"
#include "targetsmanager.h"

using namespace dpfservice;
void RuntimePlugin::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(RuntimeService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool RuntimePlugin::start()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService && windowService->addCentralNavigation) {
        windowService->addCentralNavigation(MWNA_RUNTIME, new AbstractCentral(RuntimeManager::instance()->getRuntimeWidget()));
    }

    // bind interface that get default output path.
    ProjectService *projService = ctx.service<ProjectService>(ProjectService::name());
    if (projService && !projService->getDefaultOutputPath) {
        projService->getDefaultOutputPath = std::bind(&KitManager::getDefaultOutputPath, KitManager::instance());
    }

    // bind interface that get default output path.
    RuntimeService *runTimeService = ctx.service<RuntimeService>(RuntimeService::name());
    if (runTimeService && !runTimeService->getActiveTarget) {
        runTimeService->getActiveTarget = std::bind(&TargetsManager::getActiveBuildTarget, TargetsManager::instance());
    }

    return true;
}

dpf::Plugin::ShutdownFlag RuntimePlugin::stop()
{
    return Sync;
}
