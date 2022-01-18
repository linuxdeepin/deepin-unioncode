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
#include "debugger.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "appoutputpane.h"

DebugManager::DebugManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<OutputFormat>("OutputFormat");
}

bool DebugManager::initialize()
{
    debugger.reset(new Debugger(this));
    outputPane.reset(new AppOutputPane());

    connect(debuggerSignals, &DebuggerSignals::breakpointAdded, this, &DebugManager::slotBreakpointAdded);
    connect(debuggerSignals, &DebuggerSignals::addOutput, this, &DebugManager::slotOutput);

    return true;
}

AppOutputPane *DebugManager::getOutputPane() const
{
    return outputPane.get();
}

void DebugManager::startDebug()
{
    debugger->startDebug();
}

void DebugManager::detachDebug()
{
    debugger->detachDebug();
}

void DebugManager::interruptDebug()
{
    debugger->interruptDebug();
}

void DebugManager::continueDebug()
{
    debugger->continueDebug();
}

void DebugManager::abortDebug()
{
    debugger->abortDebug();
}

void DebugManager::restartDebug()
{
    debugger->restartDebug();
}

void DebugManager::stepOver()
{
    debugger->stepOver();
}

void DebugManager::stepIn()
{
    debugger->stepIn();
}

void DebugManager::stepOut()
{
    debugger->stepOut();
}

void DebugManager::slotBreakpointAdded(const QString &filepath, int lineNumber)
{
    debugger->addBreakpoint(filepath, lineNumber);
}

void DebugManager::slotOutput(const QString &content, OutputFormat format)
{
     outputPane->appendText(content, format);
}
