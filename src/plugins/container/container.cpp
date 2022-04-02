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
#include "container.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "services/container/containerservice.h"
#include "containerwidget.h"

using namespace dpfservice;
void Container::initialize()
{
    qInfo() << __FUNCTION__;
    //发布Container到edit导航栏界面布局
    if (QString(getenv("TERM")).isEmpty()) {
        setenv("TERM", "xterm-256color", 1);
    }
    setenv("SHELL","/bin/bash",1);

    // 发布容器服务
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(ContainerService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool Container::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        windowService->addContextWidget("virtenv", new AbstractWidget(ContainerWidget::instance()));
    }

    ContainerService *containerService = ctx.service<ContainerService>(ContainerService::name());
    if (containerService) {
        if (!containerService->addContainerMount) { // 发布容器挂载点服务接口
            //containerService->addContainerMount = std::bind();
        }
    }

    return true;
}

dpf::Plugin::ShutdownFlag Container::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}
