// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "console.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "common/util/eventdefinitions.h"
#include "consolewidget.h"

using namespace dpfservice;
void Console::initialize()
{
    qInfo() << __FUNCTION__;
    //发布Console到edit导航栏界面布局
    if (QString(getenv("TERM")).isEmpty()) {
        setenv("TERM", "xterm-256color", 1);
    }
}

bool Console::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        windowService->addContextWidget(QString(tr("&Console")), new AbstractWidget(ConsoleWidget::instance()), MWNA_EDIT, true);
    }
    return true;
}

dpf::Plugin::ShutdownFlag Console::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}
