/*
 * Copyright (C) 2020 ~ 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#include "valgrind.h"
#include "mainframe/valgrindrunner.h"
#include "mainframe/valgrindbar.h"

#include "common/common.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "services/window/windowelement.h"

#include <QAction>

void Valgrind::initialize()
{
    qInfo() << __FUNCTION__;
}

bool Valgrind::start()
{
    qInfo() << __FUNCTION__;
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        ValgrindBar *valgrindBar = new ValgrindBar();
        windowService->addContextWidget(tr("&Valgrind"), new AbstractWidget(valgrindBar), "Valgrind");
    }

    ValgrindRunner::instance()->initialize();

    return true;
}

dpf::Plugin::ShutdownFlag Valgrind::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}
