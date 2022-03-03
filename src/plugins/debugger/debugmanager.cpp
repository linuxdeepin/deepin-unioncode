/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#include "debugmanager.h"
#include "dap/debugger.h"
#include "debuggerglobals.h"
#include "common/util/custompaths.h"

using namespace DEBUG_NAMESPACE;
DebugManager::DebugManager(QObject *parent)
    : QObject(parent)
{
}

bool DebugManager::initialize()
{
    debugger.reset(new Debugger(this));

    debugger->initializeView();

    return true;
}

AppOutputPane *DebugManager::getOutputPane() const
{
    return debugger->getOutputPane();
}

QTreeView *DebugManager::getStackPane() const
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

void DebugManager::startDebug()
{
    launchBackend();
    AsynInvoke(debugger->startDebug());
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

void DebugManager::launchBackend()
{
    // TODO(mozart):backend not support multi-start,so re-launch
    // it when debugger start debug.
    // may not use those code when backend got modified.
    QProcess::execute("killall -9 cxxdbg");

    QString toolPath = CustomPaths::global(CustomPaths::Tools);
    QString backendPath = toolPath + "cxxdbg";

    backend.close();
    backend.startDetached(backendPath);
    backend.waitForFinished();
}
