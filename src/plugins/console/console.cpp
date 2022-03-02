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
#include "base/abstractconsole.h"
#include "services/window/windowservice.h"
#include "qtermwidget.h" // 3drparty
#include "ColorScheme.h"

using namespace dpfservice;

class ConsoleWidget : public QTermWidget
{
public:
    ConsoleWidget(QWidget *parent = nullptr)
        : QTermWidget(parent)
    {
        setMargin(0);
        setTerminalOpacity(0);
        setForegroundRole(QPalette::ColorRole::Background);
        setAutoFillBackground(true);
        if (availableColorSchemes().contains("Linux"))
            setColorScheme("Linux");
        if (availableKeyBindings().contains("linux"))
            setKeyBindings("linux");
        setScrollBarPosition(QTermWidget::ScrollBarRight);
        setTerminalSizeHint(false);
        setAutoClose(false);
        changeDir(QDir::homePath());
        sendText("clear\n");
    }

    virtual ~ConsoleWidget()
    {
        qInfo() << __FUNCTION__;
    }
};

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
        ConsoleWidget* console = new ConsoleWidget();
        emit windowService->setEditorConsole(new AbstractConsole(console));
    }
    return true;
}

dpf::Plugin::ShutdownFlag Console::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}
