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
#include "windowkeeper.h"
#include "windowtheme.h"
#include "services/window/windowservice.h"

#include <framework/framework.h>

#include <QStatusBar>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>

using namespace dpfservice;

static WindowKeeper *windowKeeper = nullptr;

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
    WindowTheme::setTheme(":/dark-one.css");

    //创建窗口管理器
    if (!windowKeeper) {
        windowKeeper = new WindowKeeper();
    }

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    delete windowKeeper;
    return Sync;
}
