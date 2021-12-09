/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "runtime.h"
#include "outputwidget.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"

using namespace dpfservice;

void Runtime::initialize()
{

}

bool Runtime::start()
{
#if 0
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        //发布Console到edit导航栏界面布局
        emit windowService->addContextWidget("Output", new AbstractWidget(new OutputWidget));
    }
#endif
    return true;
}

dpf::Plugin::ShutdownFlag Runtime::stop()
{
    return Sync;
}
