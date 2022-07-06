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
#include "buildergradle.h"
#include "mainframe/gradlegenerator.h"

#include "services/builder/builderservice.h"
#include "service/pluginservicecontext.h"
#include "services/project/projectservice.h"

#include <QMenu>

using namespace dpfservice;

void BuilderGradle::initialize()
{

}

bool BuilderGradle::start()
{
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        QString errorString;
        bool ret = builderService->regClass<GradleGenerator>(GradleGenerator::toolKitName(), &errorString);
        if (ret) {
            builderService->create<GradleGenerator>(GradleGenerator::toolKitName(), &errorString);
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag BuilderGradle::stop()
{
    return Sync;
}
