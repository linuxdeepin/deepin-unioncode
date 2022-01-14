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

#include "rawdebugsession.h"
#include "runtimecfgprovider.h"
#include "debugservice.h"
#include "debuggerglobals.h"
#include "eventsender.h"

#include "dap/io.h"
#include "dap/protocol.h"
#include "dap/session.h"
#include "dap/network.h"

#include <QDebug>
#include <QUuid>

#include <chrono>
#include <thread>

using namespace dap;
DebugSession::DebugSession(QObject *parent)
    : QObject(parent),
      id(QUuid::createUuid().toString().toStdString())
{
}

Capabilities DebugSession::capabilities() const
{
    return raw->capabilities();
}

bool DebugSession::initialize(const char *ip, int port, dap::InitializeRequest &iniRequest)
{
    if (raw) {
        // if there was already a connection make sure to remove old listeners
        shutdown();
    }

    rtCfgProvider.reset(new RunTimeCfgProvider(this));

    constexpr int kMaxAttempts = 10;
    bool connected = false;
    // The socket might take a while to open - retry connecting.
    for (int attempt = 0; attempt < kMaxAttempts; attempt++) {
        auto connection = net::connect(ip, port);
        if (!connection) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        // Socket opened. Create the debugger session and bind.
        session = dap::Session::create();
        session->bind(connection);

        raw.reset(new RawDebugSession(session, this));

        connected = true;
        break;
    }

    if (!connected) {
        return false;
    }

    auto init_res = raw->initialize(iniRequest).get();
    if (init_res.error) {
        initialized = false;
        shutdown();
        qDebug() << init_res.error.message.c_str();
        return false;
    } else {
        initialized = true;
        raw->setExceptionBreakpoints({});
    }
    registerHandlers();

    return initialized;
}

bool DebugSession::launch(const char *config, bool noDebug)
{
    Q_UNUSED(noDebug)
    if (!raw)
        return false;

    bool bSuccess = session->send(config);
    if (!bSuccess) {
        qDebug() << "launch request failed.";
        return false;
    }
    return true;
}

bool DebugSession::attach(dap::AttachRequest &config)
{
    Q_UNUSED(config)
    return true;
}

void DebugSession::restart()
{
    raw->restart({});
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

void DebugSession::continueDbg(integer threadId)
{
    ContinueRequest request;
    request.threadId = threadId;
    raw->continueDbg(request);
}

void DebugSession::pause(integer threadId)
{
    PauseRequest request;
    request.threadId = threadId;
    raw->pause(request);
}

void DebugSession::stepIn(dap::integer threadId, dap::optional<integer> targetId, dap::optional<dap::SteppingGranularity> granularity)
{
    StepInRequest request;
    request.threadId = threadId;
    request.targetId = targetId;
    request.granularity = granularity;
    //? setLastSteppingGranularity(threadId, granularity);
    raw->stepIn(request);
}

void DebugSession::stepOut(integer threadId, dap::optional<dap::SteppingGranularity> granularity)
{
    StepOutRequest request;
    request.threadId = threadId;
    request.granularity = granularity;
    //? setLastSteppingGranularity(threadId, granularity);
    raw->stepOut(request);
}

void DebugSession::next(integer threadId, dap::optional<dap::SteppingGranularity> granularity)
{
    NextRequest request;
    request.threadId = threadId;
    request.granularity = granularity;
    raw->next(request);
}

void DebugSession::sendBreakpoints(dap::array<IBreakpoint> &breakpointsToSend)
{
    if (!raw)
        return;

    if (!raw->readyForBreakpoints()) {
        qInfo() << "break point not ready!";
        return;
    }
    SetBreakpointsRequest request;
    dap::array<SourceBreakpoint> breakpoints;
    for (auto it : breakpointsToSend) {
        Source source;
        source.path = it.uri.toString().toStdString();
        source.name = undefined;
        request.source = source;
        SourceBreakpoint bt;
        bt.line = it.lineNumber; // + 1
        breakpoints.push_back(bt);
    }

    request.breakpoints = breakpoints;
    auto response = raw->setBreakpoints(request);
    if (response.valid()) {
        response.wait();
    }
}

string DebugSession::getId()
{
    return id;
}

integer DebugSession::getThreadId()
{
    return threadId;
}

void DebugSession::shutdown()
{
    if (raw) {
        raw->disconnect({});
        raw.reset(nullptr);
    }
}

void DebugSession::registerHandlers()
{
    /*
     *  Process the only one reverse request.
     */
    session->registerHandler([&](const RunInTerminalRequest &request) {
        Q_UNUSED(request)
        qInfo() << "\n--> recv : "
                << "RunInTerminalRequest";
        return RunInTerminalResponse();
    });

    /*
     *  Register events.
     */
    // This event indicates that the debug adapter is ready to accept configuration requests.
    session->registerHandler([&](const InitializedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "InitializedEvent";
        raw->setReadyForBreakpoints(true);
        debugService->sendAllBreakpoints(this);
        raw->configurationDone().wait();
    });

    // The event indicates that the execution of the debuggee has stopped due to some condition.
    session->registerHandler([&](const StoppedEvent &event) {
        qInfo() << "\n--> recv : "
                << "StoppedEvent";
        qInfo() << "\n THREAD STOPPED. Reason : " << event.reason.c_str();

        threadId = event.threadId.value(0);
        if (event.reason == "function breakpoint"
            || event.reason == "breakpoint") {
            onBreakpointHit(event);
        } else if (event.reason == "step") {
            onStep(event);
        } else if (event.reason == "exception") {
            qInfo() << "\n description : " << event.description->c_str();
            qInfo() << "\n text :" << event.text->c_str();
        }
    });

    // The event indicates that the execution of the debuggee has continued.
    //    session->registerHandler([&](const ContinuedEvent &event){
    //        allThreadsContinued = event.allThreadsContinued;
    //        Q_UNUSED(event)
    //        qInfo() << "\n--> recv : " << "ContinuedEvent";
    //    });

    // The event indicates that the debuggee has exited and returns its exit code.
    session->registerHandler([&](const ExitedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ExitedEvent";
    });

    // The event indicates that debugging of the debuggee has terminated.
    // This does not mean that the debuggee itself has exited.
    session->registerHandler([&](const TerminatedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "TerminatedEvent";
    });

    // The event indicates that a thread has started or exited.
    session->registerHandler([&](const ThreadEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ThreadEvent";
    });

    // The event indicates that the target has produced some output.
    session->registerHandler([&](const OutputEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "OutputEvent\n"
                << "content : " << event.output.c_str();
    });

    // The event indicates that some information about a breakpoint has changed.
    session->registerHandler([&](const BreakpointEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "BreakpointEvent";
    });

    // The event indicates that some information about a module has changed.
    session->registerHandler([&](const ModuleEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ModuleEvent";
    });

    // The event indicates that some source has been added, changed,
    // or removed from the set of all loaded sources.
    session->registerHandler([&](const LoadedSourceEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "LoadedSourceEvent";
    });

    // The event indicates that the debugger has begun debugging a new process.
    session->registerHandler([&](const ProcessEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ProcessEvent";
    });

    //    // The event indicates that one or more capabilities have changed.
    //    session->registerHandler([&](const CapabilitiesEvent &event){
    //        Q_UNUSED(event)
    //        qInfo() << "\n--> recv : " << "CapabilitiesEvent";
    //    });

    session->registerHandler([&](const ProgressStartEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ProgressStartEvent";
    });

    session->registerHandler([&](const ProgressUpdateEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ProgressUpdateEvent";
    });

    session->registerHandler([&](const ProgressEndEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ProgressEndEvent";
    });

    // This event signals that some state in the debug adapter has changed
    // and requires that the client needs to re-render the data snapshot previously requested.
    session->registerHandler([&](const InvalidatedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "InvalidatedEvent";
    });
}

void DebugSession::fetchThreads(IRawStoppedDetails stoppedDetails)
{
    Q_UNUSED(stoppedDetails)
    // TODO(mozart)
}

void DebugSession::onBreakpointHit(const StoppedEvent &event)
{
    auto source = event.source;
    if (source) {
        auto path = source.value().path.value();
        int line = static_cast<int>(event.line.value());
        EventSender::jumpTo(path, line);
    }
}

void DebugSession::onStep(const StoppedEvent &event)
{
    auto source = event.source;
    if (source) {
        auto path = source.value().path.value();
        int line = static_cast<int>(event.line.value());
        EventSender::jumpTo(path, line);
    }
}
