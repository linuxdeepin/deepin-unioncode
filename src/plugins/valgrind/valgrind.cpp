// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
