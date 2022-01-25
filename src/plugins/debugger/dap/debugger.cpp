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
#include "debugger.h"
#include "runtimecfgprovider.h"
#include "debugsession.h"
#include "debugservice.h"
#include "debuggerglobals.h"
#include "debugmodel.h"

#include "dap/io.h"
#include "dap/protocol.h"

#include <QDebug>
#include <QUuid>

#include "stdlib.h"

/**
 * @brief Debugger::Debugger
 * For serial debugging service
 */
using namespace dap;
using namespace DEBUG_NAMESPACE;
Debugger::Debugger(QObject *parent)
    : QObject(parent)
{
    session.reset(new DebugSession(debugService->getModel(), this));
    rtCfgProvider.reset(new RunTimeCfgProvider(this));
}

void Debugger::startDebug()
{
    // Setup debug environment.
    auto iniRequet = rtCfgProvider->initalizeRequest();
    bool bSuccess = session->initialize(rtCfgProvider->ip(),
                                        rtCfgProvider->port(),
                                        iniRequet);

    // Launch debuggee.
    if (bSuccess) {
        bSuccess &= session->launch(rtCfgProvider->launchRequest().c_str());
    }
    if (!bSuccess) {
        qCritical() << "startDebug failed!";
    } else {
        started = true;
    }
    debugService->getModel()->addSession(session.get());
}

void Debugger::detachDebug()
{
}

void Debugger::interruptDebug()
{
    // Just use temporary parameters now, same for the back
    integer threadId = session->getThreadId();
    session->pause(threadId);
}

void Debugger::continueDebug()
{
    integer threadId = session->getThreadId();
    session->continueDbg(threadId);
}

void Debugger::abortDebug()
{
    session->terminate();
    started = false;
}

void Debugger::restartDebug()
{
    session->restart();
}

void Debugger::stepOver()
{
    integer threadId = session->getThreadId();
    session->next(threadId, undefined);
}

void Debugger::stepIn()
{
    integer threadId = session->getThreadId();
    session->stepIn(threadId, undefined, undefined);
}

void Debugger::stepOut()
{
    integer threadId = session->getThreadId();
    session->stepOut(threadId, undefined);
}

void Debugger::addBreakpoint(const QString &filepath, int lineNumber)
{
    dap::array<IBreakpointData> rawBreakpoints;
    IBreakpointData bpData;
    bpData.id = QUuid::createUuid().toString().toStdString();
    bpData.lineNumber = lineNumber;
    bpData.enabled = true;   // TODO(mozart):get from editor.
    rawBreakpoints.push_back(bpData);

    if (started) {
        debugService->addBreakpoints(filepath, rawBreakpoints, session.get());
    } else {
        debugService->addBreakpoints(filepath, rawBreakpoints, undefined);
    }
}

bool Debugger::getLocals(dap::integer frameId, IVariables *out)
{
    return session->getLocals(frameId, out);
}
