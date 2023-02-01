/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#include "debugmanager.h"
#include "dap/debugger.h"
#include "debuggerglobals.h"
#include "interface/menumanager.h"

#include "services/debugger/debuggerservice.h"
#include "common/util/custompaths.h"

using namespace DEBUG_NAMESPACE;
DebugManager::DebugManager(QObject *parent)
    : QObject(parent)
{
    // TODO(mozart):backend not support re-connect yet,
    // so kill it when client launched.
    // those code will be removed when backend got modified.
    QProcess::execute("killall -9 debugadapter");
}

bool DebugManager::initialize(dpfservice::WindowService *windowService,
                              dpfservice::DebuggerService *debuggerService)
{
    debugger = new Debugger(this);
    runner = new Runner(this);

    connect(runner, &Runner::sigOutputMsg, debugger, &Debugger::printOutput);

    menuManager.reset(new MenuManager());
    menuManager->initialize(windowService);

    connect(debugger, &Debugger::runStateChanged, this, &DebugManager::handleRunStateChanged);
    using namespace std::placeholders;
    if (!debuggerService->runCoredump) {
        debuggerService->runCoredump = std::bind(&DebugManager::runCoredump, this, _1, _2, _3);
    }

    launchBackend();

    return true;
}

AppOutputPane *DebugManager::getOutputPane() const
{
    return debugger->getOutputPane();
}

QWidget *DebugManager::getStackPane() const
{
    return debugger->getStackPane();
}

QTreeView *DebugManager::getLocalsPane() const
{
    return debugger->getLocalsPane();
}

QTreeView *DebugManager::getBreakpointPane() const
{
    return debugger->getBreakpointPane();
}

void DebugManager::run()
{
    Debugger::RunState state = debugger->getRunState();
    switch (state) {
    case Debugger::RunState::kNoRun:
    case Debugger::RunState::kPreparing:
        launchBackend();
        AsynInvoke(debugger->startDebug())
        break;
    case Debugger::RunState::kRunning:
        // TODO(mozart):stop debug
        break;
    case Debugger::RunState::kStopped:
        continueDebug();
        break;
    }
}

void DebugManager::detachDebug()
{
    AsynInvoke(debugger->detachDebug());
}

void DebugManager::interruptDebug()
{
    AsynInvoke(debugger->interruptDebug());
}

void DebugManager::continueDebug()
{
    AsynInvoke(debugger->continueDebug());
}

void DebugManager::abortDebug()
{
    AsynInvoke(debugger->abortDebug());
}

void DebugManager::restartDebug()
{
    AsynInvoke(debugger->restartDebug());
}

void DebugManager::stepOver()
{
    AsynInvoke(debugger->stepOver());
}

void DebugManager::stepIn()
{
    AsynInvoke(debugger->stepIn());
}

void DebugManager::stepOut()
{
    AsynInvoke(debugger->stepOut());
}

void DebugManager::handleRunStateChanged(Debugger::RunState state)
{
    menuManager->handleRunStateChanged(state);

    if(state == Debugger::kStart || state == Debugger::kRunning) {
        emit debugStarted();
    }
}

void DebugManager::launchBackend()
{
    // launch backend by client.
    if (backend.isOpen())
        return;

    QString toolPath = CustomPaths::global(CustomPaths::Tools);
    QString backendPath = toolPath + QDir::separator() + "debugadapter";

    backend.setProgram(backendPath);
    backend.start();
    backend.waitForStarted();
}

bool DebugManager::runCoredump(const QString &target, const QString &core, const QString &kit)
{
    launchBackend();
    return QtConcurrent::run(debugger, &Debugger::runCoredump, target, core, kit);
}
