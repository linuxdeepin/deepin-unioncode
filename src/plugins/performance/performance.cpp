// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainframe/mainwindow.h"
#include "performance.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"

#include <QAction>

#include <unistd.h>

namespace {
static MainWindow *mainWindow{nullptr};
}

using namespace dpfservice;

void Performance::initialize()
{

}

bool Performance::start()
{
    qInfo() << __FUNCTION__;

    if (!::mainWindow) {
        ::mainWindow = new MainWindow();
    }

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (::mainWindow && windowService) {
        windowService->addContextWidget(QString(tr("&Performance")), new AbstractWidget(::mainWindow), false);
    }

    return true;
}

dpf::Plugin::ShutdownFlag Performance::stop()
{
    return Sync;
}
