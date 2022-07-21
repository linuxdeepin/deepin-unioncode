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
#include "optionjava.h"
#include "mainframe/optionjavagenerator.h"

#include "base/abstractmenu.h"
#include "base/abstractmainwindow.h"
#include "base/abstractwidget.h"

#include "services/option/optionservice.h"

#include "framework/listener/listener.h"

using namespace dpfservice;

void OptionJava::initialize()
{
    DPF_USE_NAMESPACE
}

bool OptionJava::start()
{
    auto &ctx = dpfInstance.serviceContext();
    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    if (!optionService) {
        qCritical() << "Failed, not found OptionJava service!";
        abort();
    }
    optionService->implGenerator<OptionJavaGenerator>(OptionJavaGenerator::kitName());
    return true;
}

dpf::Plugin::ShutdownFlag OptionJava::stop()
{
    return Sync;
}
