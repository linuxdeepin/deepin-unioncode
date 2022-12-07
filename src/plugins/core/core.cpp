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
#include "core.h"
#include "mainframe/windowkeeper.h"
#include "mainframe/windowtheme.h"
#include "services/window/windowservice.h"

#include <framework/framework.h>
#include "common/common.h"

#include <QStatusBar>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>

using namespace dpfservice;

void Core::initialize()
{
    qInfo() << Q_FUNC_INFO;
    QString errStr;

    // 发布窗口服务
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(WindowService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool Core::start()
{
    qInfo() << "set Application Theme";
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        using namespace std::placeholders;

        if (!windowService->insertAction) {
            windowService->insertAction = std::bind(&WindowKeeper::insertAction,
                                                    WindowKeeper::instace(), _1, _2, _3);
        }

        if (!windowService->addOpenProjectAction) {
            windowService->addOpenProjectAction = std::bind(&WindowKeeper::addOpenProjectAction,
                                                            WindowKeeper::instace(), _1, _2);
        }
    }

    WindowKeeper::instace();
    WindowTheme::setTheme(":/dark-one.css");

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        ActionManager::getInstance()->readUserSetting();
    });

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    delete WindowKeeper::instace();
    return Sync;
}
