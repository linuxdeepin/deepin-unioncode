// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "console.h"
#include "consolemanager.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "services/terminal/terminalservice.h"
#include "common/util/eventdefinitions.h"

using namespace dpfservice;
void Console::initialize()
{
    qInfo() << __FUNCTION__;
    //发布Console到edit导航栏界面布局
    if (QString(getenv("TERM")).isEmpty()) {
        setenv("TERM", "xterm-256color", 1);
    }

    // load terminal service.
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(TerminalService::name(), &errStr)) {
        qCritical() << errStr;
    }
}

bool Console::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    auto consoleManager = new ConsoleManager;
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        windowService->addContextWidget(TERMINAL_TAB_TEXT, new AbstractWidget(consoleManager), true);
    }

    // bind service.
    auto terminalService = ctx.service<TerminalService>(TerminalService::name());
    if (terminalService) {
        using namespace std::placeholders;
        terminalService->sendCommand = std::bind(&ConsoleManager::sendCommand, consoleManager, _1);
        terminalService->executeCommand = std::bind(&ConsoleManager::executeCommand, consoleManager, _1, _2, _3, _4, _5);
        terminalService->createConsole = std::bind(&ConsoleManager::newConsole, consoleManager, _1, _2);
        terminalService->selectConsole = std::bind(&ConsoleManager::selectConsole, consoleManager, _1);
        terminalService->run2Console = std::bind(&ConsoleManager::run2Console, consoleManager, _1, _2);
    }
    return true;
}

dpf::Plugin::ShutdownFlag Console::stop()
{
    return Sync;
}
