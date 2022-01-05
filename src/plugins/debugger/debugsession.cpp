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
#include "debugsession.h"

#include "io.h"
#include "protocol.h"
#include "session.h"
#include "rawdebugsession.h"
#include "network.h"

#include <QDebug>

#include <chrono>
#include <thread>

using namespace dap;
DebugSession::DebugSession(QObject *parent) : QObject(parent)
{
}

bool DebugSession::initialize()
{
    if (raw) {
        // if there was already a connection make sure to remove old listeners
        shutdown();
    }

    constexpr int kMaxAttempts = 10;
    // The socket might take a while to open - retry connecting.
    for (int attempt = 0; attempt < kMaxAttempts; attempt++) {
          auto connection = net::connect("localhost", 4711);
          if (!connection) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
          }

          // Socket opened. Create the debugger session and bind.
          session = dap::Session::create();
          session->bind(connection);

          raw.reset(new RawDebugSession(session, this));
          break;
    }

    // Just use temporary parameters now, same for the back
    InitializeRequest request;
  #ifdef DBG_TEST
    request.clientID = "vscode";
    request.clientName = "Code - OSS";
    request.adapterID = "cppdbg";
    request.pathFormat = "path";
    request.linesStartAt1 = true;
    request.columnsStartAt1 = true;
    request.supportsVariableType = true;
    request.supportsVariablePaging = true;
    request.supportsRunInTerminalRequest = true;
    request.locale = "en-US";
    request.supportsProgressReporting = true;
    request.supportsInvalidatedEvent = true;
    request.supportsMemoryReferences = true;
  #endif
    raw->initialize(request);
    auto init_res = session->send(request).get();
    if (init_res.error) {
        qDebug() << init_res.error.message.c_str();
    }

    initialized = true;

    return initialized;
}

void DebugSession::launch(bool noDebug)
{
    LaunchRequest rqLaunch;
    rqLaunch.noDebug = noDebug;
//    rqLaunch.restart = ?
    Promise<LaunchRequest> ret = raw->launch(rqLaunch);
}

void DebugSession::attach()
{
    AttachRequest rqAttach;
    raw->attach(rqAttach);
}

void DebugSession::restart()
{

}

void DebugSession::terminate(bool restart)
{
    raw->terminate(restart);
}

void DebugSession::disconnect(bool terminateDebuggee, bool restart)
{
    DisconnectRequest request;
    request.terminateDebuggee = terminateDebuggee;
    request.restart = restart;
    raw->disconnect(request);
}

void DebugSession::continueDbg(int threadId)
{
    ContinueRequest request;
    request.threadId = threadId;
    raw->continueDbg(request);
}

void DebugSession::pause(int threadId)
{
    PauseRequest request;
    request.threadId = threadId;
    raw->pause(request);
}

void DebugSession::stepIn(int threadId, int targetId, SteppingGranularity granularity)
{
    StepInRequest request;
    request.threadId = threadId;
    request.targetId = targetId;
    request.granularity = granularity;
    //? setLastSteppingGranularity(threadId, granularity);
    raw->stepIn(request);
}

void DebugSession::stepOut(int threadId, SteppingGranularity granularity)
{
    StepOutRequest request;
    request.threadId = threadId;
    request.granularity = granularity;
    //? setLastSteppingGranularity(threadId, granularity);
    raw->stepOut(request);
}

void DebugSession::next(int threadId, SteppingGranularity granularity)
{
    NextRequest request;
    request.threadId = threadId;
    request.granularity = granularity;
    raw->next(request);
}

void DebugSession::shutdown()
{
    if (raw) {
        raw->disconnect({});
        raw.reset(nullptr);
    }
}
