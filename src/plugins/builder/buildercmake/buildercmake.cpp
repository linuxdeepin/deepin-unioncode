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
#include "buildercmake.h"
#include "mainframe/cmakegenerator.h"

#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "service/pluginservicecontext.h"

using namespace dpfservice;

void BuilderCMake::initialize()
{

}

bool BuilderCMake::start()
{
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        QString errorString;
        bool ret = builderService->regClass<CMakeGenerator>(CMakeGenerator::toolKitName(), &errorString);
        if (ret) {
            builderService->create<CMakeGenerator>(CMakeGenerator::toolKitName(), &errorString);
        }
    }

    return true;
}

dpf::Plugin::ShutdownFlag BuilderCMake::stop()
{
    return Sync;
}
