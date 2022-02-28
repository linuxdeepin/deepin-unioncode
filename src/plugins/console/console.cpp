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
#include "console.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "qtermwidget.h" // 3drparty

using namespace dpfservice;

static QTermWidget *console = nullptr;

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
        console = new QTermWidget();
        qInfo() << "console" << console;
        console->setMargin(0);
        console->setTerminalOpacity(0);
        if (console->availableColorSchemes().contains("Linux"))
            console->setColorScheme("Linux");
        if (console->availableKeyBindings().contains("linux"))
            console->setKeyBindings("linux");
        console->setScrollBarPosition(QTermWidget::ScrollBarRight);
        console->setTerminalSizeHint(false);
        console->setAutoClose(true);
        console->changeDir(QDir::homePath());
        console->sendText("clear\n");
        emit windowService->setEditorConsole(new AbstractWidget(console));
    }
    return true;
}

dpf::Plugin::ShutdownFlag Console::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}
