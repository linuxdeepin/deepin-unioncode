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
#include "toolchecker.h"
#include "mainframe/backendchecker.h"

#include "services/toolchecker/toolcheckerservice.h"

#include <QAction>
#include <QLabel>
#include <QDialog>
#include <QProgressBar>

void ToolChecker::initialize()
{
    qInfo() << Q_FUNC_INFO;
    QString errStr;
    // 发布窗口服务
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(dpfservice::ToolCheckerSevice::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool ToolChecker::start()
{
    qInfo() << __FUNCTION__;
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    auto toolCheckerService = ctx.service<ToolCheckerSevice>(ToolCheckerSevice::name());
    if (!toolCheckerService->checkLanguageBackend) {
        using namespace std::placeholders;
        toolCheckerService->checkLanguageBackend
                = std::bind(&BackendChecker::checkLanguageBackend,
                            &BackendChecker::instance(), _1);
    }

    return true;
}

dpf::Plugin::ShutdownFlag ToolChecker::stop()
{
    return Sync;
}
