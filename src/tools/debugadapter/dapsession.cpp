/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "dapsession.h"

#include "backendglobal.h"
#include "dap/protocol.h"
#include "dap/session.h"
#include "dap/network.h"
#include "debugmanager.h"
#include "gdbproxy.h"

#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QtDBus/QDBusConnection>
#include <QList>

// keep the initialization in order.
static ConditionLock configured;

const int maxTryNum = 100;
using namespace dap;
DapSession::DapSession()
{
    qRegisterMetaType<std::shared_ptr<dap::ReaderWriter>>("std::shared_ptr<dap::ReaderWriter>");

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("com.deepin.unioncode.service")) {
        sessionBus.registerObject("/", &serverInfo, QDBusConnection::ExportAllContents);
    }
}

bool DapSession::start()
{
    auto onClientConnected = [this](const std::shared_ptr<ReaderWriter> &socket) {
        QMetaObject::invokeMethod(this, "initialize",  Qt::BlockingQueuedConnection, Q_ARG(std::shared_ptr<dap::ReaderWriter>, socket));
    };

    auto onError = [](const char *errMessage) {
        Log(errMessage);
    };

    // Create the network server
    server = dap::net::Server::create();

    auto checkPortFree = [](int port) {
        QProcess process;
        QString cmd = QString("fuser %1/tcp").arg(port);
        process.start(cmd);
        process.waitForFinished();
        QString ret = process.readAll();
        if (ret.isEmpty())
            return true;
        return false;
    };

    bool ret = false;
    int port = kPort;
    while (port - kPort < maxTryNum) {
        if (checkPortFree(port)) {
            // Start listening on kPort.
            // onClientConnected will be called when a client wants to connect.
            // onError will be called on any connection errors.
            ret = server->start(port, onClientConnected, onError);
            if (ret) {
                serverInfo.setPort(port);
                break;
            }
        }
        port++;
    }
    return ret;
}

void DapSession::stop()
{
    server->stop();
}

void DapSession::initialize(std::shared_ptr<dap::ReaderWriter> socket)
{
    session = dap::Session::create();
    session->bind(socket);

    initializeDebugMgr();

    registerHanlder();

    Log("DapSession initialized.\n");
}

void DapSession::initializeDebugMgr()
{
    // instance a debug manager object
    debugger = DebugManager::instance();

    Qt::ConnectionType SequentialExecution = Qt::BlockingQueuedConnection;

    // Output Event and Module Event
    connect(debugger, &DebugManager::streamDebugInternal, [&](const QString sOut) mutable {
        handleEvent(sOut);
    });

    connect(debugger, &DebugManager::asyncStopped, GDBProxy::instance(), [this](const gdb::AsyncContext &ctx) mutable {
        handleAsyncStopped(ctx);
        isInferiorStopped = true;
    });

    connect(debugger, &DebugManager::asyncRunning, GDBProxy::instance(), [this](const QString &) mutable {
        isInferiorStopped = false;
    });

    connect(debugger, &DebugManager::threadGroupAdded, GDBProxy::instance(), [this](const gdb::Thread &thid) mutable {
        handleThreadGroupAdded(thid);
    });

    connect(debugger, &DebugManager::threadGroupExited, GDBProxy::instance(), [this](const gdb::Thread &thid, const QString &exitCode) mutable {
        handleThreadGroupExited(thid, exitCode);
    });

    connect(debugger, &DebugManager::threadGroupRemoved, GDBProxy::instance(), [this](const gdb::Thread &thid) mutable {
        handleThreadGroupRemoved(thid);
    });

    connect(debugger, &DebugManager::threadGroupStarted, GDBProxy::instance(), [this](const gdb::Thread &thid, const gdb::Thread &pid) mutable {
        handleThreadGroupStarted(thid, pid);
    });

    connect(debugger, &DebugManager::libraryLoaded, GDBProxy::instance(), [this](const gdb::Library &library) mutable {
        handleLibraryLoaded(library);
    });

    connect(debugger, &DebugManager::libraryUnloaded, GDBProxy::instance(), [this](const gdb::Library &library) mutable {
        handleLibraryUnloaded(library);
    });

    QObject::connect(debugger, &DebugManager::streamConsole, [&](const QString &text) mutable {
        handleStreamConsole(text);
    });

    QObject::connect(debugger, &DebugManager::updateStackFrame, GDBProxy::instance(), [this](const QList<gdb::Frame> &) {
    });

    connect(GDBProxy::instance(), &GDBProxy::sigQuit, debugger, &DebugManager::quit, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigKill, debugger, &DebugManager::kill, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigStart, debugger, &DebugManager::execute, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigLaunchLocal, debugger, &DebugManager::launchLocal, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigContinue, debugger, &DebugManager::commandContinue, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigPause, debugger, &DebugManager::commandPause, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigNext, debugger, &DebugManager::commandNext, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigStepin, debugger, &DebugManager::commandStep, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigStepout, debugger, &DebugManager::commandFinish, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigThreads, debugger, &DebugManager::threadInfo, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigSelectThread, debugger, &DebugManager::threadSelect, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigStackTrace, debugger, &DebugManager::stackListFrames, SequentialExecution);
    //connect(GDBProxy::instance(), &GDBProxy::sigScopes, debugger, &DebugManager::stackListVariables);
    connect(GDBProxy::instance(), &GDBProxy::sigVariables, debugger, &DebugManager::stackListVariables, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigSource, debugger, &DebugManager::listSourceFiles, SequentialExecution);
    connect(GDBProxy::instance(), &GDBProxy::sigBreakRemoveAll, debugger, &DebugManager::breakRemoveAll, SequentialExecution);
}

void DapSession::registerHanlder()
{
    // The Initialize request is the first message sent from the client and
    // the response reports debugger capabilities.
    session->registerHandler([&](const dap::InitializeRequest &request) {
        return handleInitializeReq(request);
    });

    // The SetExceptionBreakpoints request instructs the debugger to set a exception breakpoints
    session->registerHandler([&](const dap::SetExceptionBreakpointsRequest &request) {
        Q_UNUSED(request);
        Log("<-- Server received setExceptionBreakpoints request from client\n");
        dap::SetExceptionBreakpointsResponse response;

        Log("--> Server sent setExceptionBreakpoints response to client\n");
        return response;
    });

    // The SetBreakpoints request instructs the debugger to clear and set a number
    // of line breakpoints for a specific source file.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_SetBreakpoints
    session->registerHandler([&](const dap::SetBreakpointsRequest &request) {
        return handleBreakpointReq(request);
    });

    // The SetFunctionBreakpointBreakpoints request instructs the debugger to set a function breakpoints
    session->registerHandler([&](const dap::SetFunctionBreakpointsRequest &request) {
        Q_UNUSED(request);
        dap::SetFunctionBreakpointsResponse response;
        Log("<-- Server received setFunctionBreakpoints request from client\n");
        auto breakpoints = request.breakpoints;
        for (auto &breakpoint : breakpoints) {
            auto functionName = breakpoint.name;
            qInfo() << functionName.c_str();
            if (!functionName.empty()) {
                debugger->breakInsert(functionName.c_str());
            }
        }
        // Generic setFunctionBreakpointResponse
        Log("--> Server sent setFunctionBreakpoints response to client\n");
        return response;
    });

    // The SetDataBreakpoints request instructs the debugger to set a data breakpoints
    session->registerHandler([&](const dap::SetDataBreakpointsRequest &request) {
        Q_UNUSED(request);
        dap::SetDataBreakpointsResponse response;
        Log("<-- Server received SetDataBreakpoints request from client\n");

        Log("--> Server sent SetDataBreakpoints response to client\n");
        return response;
    });

    // Signal used to configurate the server session when ConfigurationDoneReqeust
    // The ConfigurationDone request is made by the client once all configuration
    // requests have been made.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_ConfigurationDone
    session->registerHandler([&](const dap::ConfigurationDoneRequest &request) {
        Q_UNUSED(request);
        Log("<-- Server received configurationDone request from client\n");
        auto response = dap::ConfigurationDoneResponse();
        Log("--> Server sent configurationDone response to client\n");
        return response;
    });

    // execute debugger and debuggee after configurate done response
    session->registerSentHandler([&](const dap::ResponseOrError<dap::ConfigurationDoneResponse> &response) {
        Q_UNUSED(response);
        emit GDBProxy::instance()->sigLaunchLocal();

        connect(debugger, &DebugManager::asyncRunning, GDBProxy::instance(), [this](const QString &thid) mutable {
            // TODO(Any):multi-thread condition should be done.
            Q_UNUSED(thid)
            processId = debugger->getProcessId();
            threadId = thid.toInt();
            dap::integer pointerSize;
            dap::ProcessEvent processEvent;
            processEvent.name = processName.toStdString();
            processEvent.startMethod = "launch";
            processEvent.systemProcessId = processId;
            processEvent.pointerSize = pointerSize;
            session->send(processEvent);
            Log("--> Server sent process Event to client\n");

            configured.fire();
        }, Qt::UniqueConnection);
    });

    // The Launch request is made when the client instructs the debugger adapter
    // to start the debuggee. This request contains the launch arguments.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Launch
    session->registerHandler([&](const dap::LaunchRequest &request) {
        Q_UNUSED(request);
        dap::LaunchResponse response {};
        Log("<-- Server received launch request from client\n");
        isDebuggeIsStartWithLaunchRequest = true;
        if (request.type.has_value()) {
            debuggerName = "gdb";
            debugger->setGdbCommand(debuggerName);
        }
        if (request.program.has_value()) {
            processName = request.program.value().c_str();
            debugger->setGdbArgs(QStringList(processName));
        }
        emit GDBProxy::instance()->sigStart();
        auto message = QString{"gdb process started with pid: %1\n"}.arg(debugger->getProcessId());
        Log(message.toStdString().c_str());
        Log("--> Server sent launch response to client\n");
        return response;
    });

    session->registerSentHandler([&](const dap::ResponseOrError<dap::LaunchResponse> &response) {
        Q_UNUSED(response);
        Log("--> Server sent initialized event to client\n");
        session->send(dap::InitializedEvent());
    });

    // The Attach request
    session->registerHandler([=](const dap::AttachRequest &request) {
        Q_UNUSED(request);
        Log("<-- Server received attach reqeust from client\n");
        isDebuggeIsStartWithAttachRequest = true;
        dap::AttachResponse response;
        Log("--> Server sent attach response to client\n");
        return response;
    });

    // The Restart request
    // Restarts a debug session. Clients should only call this request if the capability
    // ‘supportsRestartRequest’ is true
    session->registerHandler([=](const dap::RestartRequest &request) {
        Q_UNUSED(request);
        dap::RestartResponse response;
        Log("<-- Server received restart request from client\n");

        isDebuggeIsStartWithLaunchRequest = true;
        // kill current process.
        emit GDBProxy::instance()->sigKill();
        // re-launch local.
        emit GDBProxy::instance()->sigLaunchLocal();

        Log("--> Server sent restart response to client\n");
        return response;
    });

    // The Terminate requestis sent from the client to the debug adapter
    //in order to give the debuggee a chance for terminating itself.
    // map to Debug Plugin Menu: Abort Debugging
    session->registerHandler([=](const dap::TerminateRequest &request) {
        Q_UNUSED(request);
        dap::TerminateResponse response;
        Log("<-- Server received terminate request from client\n");
        // send quit command to debugger
        emit GDBProxy::instance()->sigQuit();

        dap::TerminatedEvent terminatedEvent;
        session->send(terminatedEvent);
        Log("--> Server sent terminate response to client\n");
        return response;
    });

    // The Pause request instructs the debugger to pause execution of one or all
    // threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pause
    session->registerHandler([&](const dap::PauseRequest &request) {
        Q_UNUSED(request);
        Log("<-- Server received pause request from client\n");
        emit GDBProxy::instance()->sigPause();
        Log("--> Server sent pause response to client\n");
        return dap::PauseResponse();
    });

    // The Continue request instructs the debugger to resume execution of one or
    // all threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
    session->registerHandler([&](const dap::ContinueRequest &request) {
        Q_UNUSED(request);
        //        auto threadId = request.threadId;
        Log("<-- Server received continue request from client\n");
        emit GDBProxy::instance()->sigContinue();
        Log("--> Server received continue request from client\n");
        return dap::ContinueResponse();
    });

    // The Next request instructs the debugger to single line step for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Next
    session->registerHandler([&](const dap::NextRequest &request) {
        Q_UNUSED(request);
        Log("<-- Server received next request from client\n");
        emit GDBProxy::instance()->sigNext();
        Log("--> Server sent to  next response client\n");
        return dap::NextResponse();
    });

    // The StepIn request instructs the debugger to step-in for a specific thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
    session->registerHandler([&](const dap::StepInRequest &request) {
        Q_UNUSED(request);
        //        auto targetId = request.targetId;
        //        auto threadId = request.threadId;
        //        auto granularity = request.granularity;
        Log("<-- Server received stepin request from client\n");
        emit GDBProxy::instance()->sigStepin();
        Log("--> Server sent stepin response to client\n");
        return dap::StepInResponse();
    });

    // The StepOut request instructs the debugger to step-out for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepOut
    session->registerHandler([&](const dap::StepOutRequest &request) {
        Q_UNUSED(request);
        //        auto threadId = request.threadId;
        //        auto granularity = request.granularity;
        Log("<-- Server received stepout request from client\n");
        emit GDBProxy::instance()->sigStepout();
        Log("--> Server sent stepout response to client\n");
        return dap::StepOutResponse();
    });

    // The BreakpointLocations request returns all possible locations for source breakpoints in a range
    session->registerHandler([&](const dap::BreakpointLocationsRequest &request) {
        Q_UNUSED(request);
        Log("<-- Server received BreakpointLocations  request from client\n");
        dap::BreakpointLocationsResponse response;
        Log("--> Server sent BreakpointLocations response to client\n");
        return response;
    });

    // The Threads request queries the debugger's list of active threads.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Threads
    session->registerHandler([&](const dap::ThreadsRequest &request) {
        configured.wait();
        Q_UNUSED(request);
        isThreadRequestReceived = true;
        dap::ThreadsResponse response;
        Log("<-- Server recevied Thread request from client\n");
        debugger->threadInfo();

        Log("--> Server sent Thread response to client\n");
        QList<gdb::Thread> allThreads = debugger->allThreadList();
        dap::array<dap::Thread> retThreads;
        for (auto it : allThreads) {
            dap::Thread thread;
            thread.id = it.id;
            thread.name = it.name.toStdString();
            retThreads.push_back(thread);
        }

        response.threads = retThreads;
        return response;
    });

    // The request returns a stacktrace from the current execution state of a given thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StackTrace
    session->registerHandler([&](const dap::StackTraceRequest &request)
                                     -> dap::StackTraceResponse {

        auto threadid = request.threadId.operator long();

        // select a given thread id
        // -thread-select 3
        gdb::Thread threadSelected;
        threadSelected.id = static_cast<int>(threadid);
        emit GDBProxy::instance()->sigSelectThread(threadSelected);

        dap::StackTraceResponse response;
        dap::array<dap::StackFrame> stackFrames;
        Log("<-- Server received StackTrace request from the client\n");
        QList<gdb::Frame> frames;
        if (isInferiorStopped) {
            debugger->stackListFrames();
            frames = debugger->allStackframes();
        }

        for (const auto &frame : frames) {
            dap::Source source;
            dap::StackFrame stackframe;
            stackframe.id = frame.level;
            stackframe.line = frame.line;
            stackframe.column = 1;
            stackframe.name = frame.func.toStdString();
            auto address = "0x" + QString::number(frame.addr, 16);
            stackframe.instructionPointerReference = address.toStdString();
            source.name = frame.func.toStdString();
            source.path = frame.fullpath.toStdString();
            stackframe.source = source;
            stackFrames.push_back(stackframe);
        }

        stackframes = frames;
        response.stackFrames = stackFrames;
        Log("--> Server sent StackTrace response to the client\n");

        return response;
    });

    // The Scopes(all variables in selected thread and frame) request reports all the scopes of the given stack frame.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Scopes
    session->registerHandler([&](const dap::ScopesRequest &request)
                                     -> dap::ScopesResponse {
        Q_UNUSED(request);
        auto frameId = request.frameId;
        static dap::ScopesResponse response;
        dap::array<dap::Scope> scopes;
        Log("<-- Server received Scopes request from the client\n");
        //emit GDBProxy::instance()->sigScopes(frameId);

        // locals
        dap::Scope scopeLocals;
        scopeLocals.presentationHint = "locals";
        scopeLocals.name = "Locals";
        scopeLocals.expensive = false;
        scopeLocals.variablesReference = frameId;
        scopes.push_back(scopeLocals);

        // register
        dap::Scope scopeRegisters;
        scopeRegisters.presentationHint = "registers";
        scopeRegisters.name = "Registers";
        scopeRegisters.expensive = false;
        scopeRegisters.variablesReference = frameId + 1;
        scopes.push_back(scopeRegisters);
        response.scopes = scopes;
        Log("--> Server sent Scopes response to the client\n");
        return response;
    });

    // Retrieves all child variables for the given variable reference(the given scope).
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Variables
    session->registerHandler([&](const dap::VariablesRequest &request)
                                     -> dap::ResponseOrError<dap::VariablesResponse> {
        Q_UNUSED(request);
        Log("<-- Server received Variables request from the client\n");
        ResponseOrError<dap::VariablesResponse> response;
        debugger->stackListVariables();
        dap::array<dap::Variable> variables;
        auto variableList = debugger->allVariableList();
        for (const auto &var : variableList) {
            dap::Variable variable;
            variable.name = var.name.toStdString();
            variable.type = var.type.toStdString();
            variable.value = var.value.toStdString();
            variables.push_back(variable);
        }
        response.response.variables = variables;
        Log("--> Server sent Variables response to the client\n");
        return response;
    });

    // The Source request retrieves the source code for a given source file.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Source
    session->registerHandler([&](const dap::SourceRequest &request)
                                     -> dap::ResponseOrError<dap::SourceResponse> {
        Q_UNUSED(request);
        dap::ResponseOrError<dap::SourceResponse> response;
        Log("<-- Server received Source request from the client\n");
        emit GDBProxy::instance()->sigSource();
        Log("--> Server sent Source response to the client\n");
        return response;
    });

    // The Disconnect request is made by the client before it disconnects
    // from the server.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Disconnect
    // map to Debug Plugin Menu: Deattach
    session->registerHandler([&](const dap::DisconnectRequest &request) {
        Q_UNUSED(request);
        Log("<-- Server received disconnnect request from client\n");
        if (isSupportsTerminateDebuggee) {
            if (isDebuggeIsStartWithLaunchRequest) {
                emit GDBProxy::instance()->sigQuit();
            } else if (isDebuggeIsStartWithAttachRequest) {
                qInfo() << "no supports terminate with attach target\n";
            }
        }
        return dap::DisconnectResponse {};
    });
}

void DapSession::handleAsyncStopped(const gdb::AsyncContext &ctx)
{
    dap::StoppedEvent stoppedEvent;
    currentFile = ctx.frame.file;
    currentLine = ctx.frame.line;
    switch (ctx.reason) {
    case gdb::AsyncContext::Reason::breakpointHhit: {
        stoppedEvent.reason = "breakpoint-hit";
        break;
    }
    case gdb::AsyncContext::Reason::endSteppingRange: {
        stoppedEvent.reason = "step";
        break;
    }
    case gdb::AsyncContext::Reason::exitedNormally: {
        stoppedEvent.reason = "exited-normally";
        dap::ExitedEvent exitEvent;
        session->send(exitEvent);
        break;
    }
    case gdb::AsyncContext::Reason::exitedSignalled: {
        stoppedEvent.reason = "exited-signalled";
        dap::ExitedEvent exitEvent;
        session->send(exitEvent);
        break;
    }
    case gdb::AsyncContext::Reason::signalReceived: {
        stoppedEvent.reason = "signal-received";
        break;
    }
    case gdb::AsyncContext::Reason::Unknown: {
        stoppedEvent.reason = "unknown";
        dap::ExitedEvent exitEvent;
        session->send(exitEvent);
        break;
    }
    case gdb::AsyncContext::Reason::readWatchpointTrigger: {
        stoppedEvent.reason = "read-watchporint-trigger";
        break;
    }
    case gdb::AsyncContext::Reason::accessWatchpointTrigger: {
        stoppedEvent.reason = "access-wathcpoint-trigger";
        break;
    }
    case gdb::AsyncContext::Reason::functionFinished: {
        stoppedEvent.reason = "function-finished";
        break;
    }
    case gdb::AsyncContext::Reason::locationReached: {
        stoppedEvent.reason = "location-reached";
        break;
    }
    case gdb::AsyncContext::Reason::watchpointScope: {
        stoppedEvent.reason = "wathcpoint-scope";
        break;
    }
    case gdb::AsyncContext::Reason::exec: {
        stoppedEvent.reason = "exec";
        break;
    }
    case gdb::AsyncContext::Reason::exited: {
        stoppedEvent.reason = "exit";
        dap::ExitedEvent exitEvent;
        session->send(exitEvent);
        break;
    }
    case gdb::AsyncContext::Reason::solibEvent: {
        stoppedEvent.reason = "solib-event";
        break;
    }
    case gdb::AsyncContext::Reason::fork: {
        stoppedEvent.reason = "fork";
        break;
    }
    case gdb::AsyncContext::Reason::vfork: {
        stoppedEvent.reason = "vfork";
        break;
    }
    case gdb::AsyncContext::Reason::syscallEntry: {
        stoppedEvent.reason = "syscall-entry";
        break;
    }
    case gdb::AsyncContext::Reason::syscallReturn: {
        stoppedEvent.reason = "syscall-return";
        break;
    }
    case gdb::AsyncContext::Reason::watchpointTrigger: {
        stoppedEvent.reason = "watchpoint-trigger";
        break;
    }
    }
    dap::Source source;
    source.name = ctx.frame.file.toStdString();
    source.path = ctx.frame.fullpath.toStdString();
    stoppedEvent.threadId = ctx.threadId.toInt();
    stoppedEvent.allThreadsStopped = true;
    stoppedEvent.source = source;
    stoppedEvent.line = ctx.frame.line;
    session->send(stoppedEvent);
    Log("--> Server sent stopped Event to client\n");
}

void DapSession::handleThreadGroupAdded(const gdb::Thread &thid)
{
    threadGroupId = QString::number(thid.id);
}

void DapSession::handleThreadGroupRemoved(const gdb::Thread &thid)
{
    threadGroupId = QString::number(thid.id);
}

void DapSession::handleThreadGroupStarted(const gdb::Thread &thid, const gdb::Thread &pid)
{
    Q_UNUSED(thid)
    Q_UNUSED(pid)
}

void DapSession::handleThreadGroupExited(const gdb::Thread &thid, const QString &exitCode)
{
    Q_UNUSED(thid)
    Q_UNUSED(exitCode)
}

void DapSession::hanleThreadCreated(const gdb::Thread &thid, const QString &groupId)
{
    Q_UNUSED(thid)
    Q_UNUSED(groupId)
}

void DapSession::handleThreadExited(const gdb::Thread &thid, const QString &groupId)
{
    Q_UNUSED(groupId)
    dap::ThreadEvent threadEvent;
    threadEvent.reason = "exited";
    threadEvent.threadId = thid.id;
    session->send(threadEvent);
}

void DapSession::handleThreadSelected(const gdb::Thread &thid, const gdb::Frame &frame)
{
    Q_UNUSED(thid)
    Q_UNUSED(frame)
}

void DapSession::hanldeUpdateThreads(int currentId, const QList<gdb::Thread> &threads)
{
    Q_UNUSED(currentId)
    Q_UNUSED(threads)
}

void DapSession::handleLibraryLoaded(const gdb::Library &library)
{
    dap::OutputEvent outputEvent;
    dap::ModuleEvent moduleEvent;
    outputEvent.category = "console";
    auto targetName = library.targetName;
    outputEvent.output = QString { "Loaded %1. Symbols loaded.\n" }.arg(targetName).toStdString();
    moduleEvent.reason = "new";
    dap::Module module;
    module.id = library.id.toStdString();
    module.name = library.targetName.toStdString();
    module.path = library.hostName.toStdString();
    module.symbolFilePath = library.hostName.toStdString();
    moduleEvent.module = module;
    session->send(outputEvent);
    Log("--> Server sent output event to client\n");
    session->send(moduleEvent);
    Log("--> Server sent module event to client\n");
}

void DapSession::handleLibraryUnloaded(const gdb::Library &library)
{
    Q_UNUSED(library)
}

void DapSession::handleStreamConsole(const QString &text)
{
    dap::OutputEvent outputEvent;
    QString output = text;
    outputEvent.category = "stdout";
    outputEvent.output = output.toStdString();
    session->send(outputEvent);
    Log("--> Server sent output event to client\n");
}

void DapSession::handleEvent(const QString &sOut)
{
    //qInfo() << sOut;
    QString eventOutput;
    do {
        if (!sOut.contains("gdbCommand")) {
            eventOutput += sOut.split(":").last();
        }

        if (eventOutput.contains("=thread-group-added") && eventOutput.contains("\(gdb\)")) {
            dap::OutputEvent outputEvent;
            outputEvent.output = eventOutput.toStdString();
            session->send(outputEvent);
            Log("--> Server sent output event to client.\n");
            eventOutput.clear();
        }
    } while (sOut.contains("~\"done.\n"));
}

dap::SetBreakpointsResponse DapSession::handleBreakpointReq(const SetBreakpointsRequest &request)
{
    dap::SetBreakpointsResponse response;

    Log("<-- Server received setBreakpoints request from client\n");
    if (request.source.path->empty()) {
        emit GDBProxy::instance()->sigBreakRemoveAll();
        return response;
    }

    const char *sourcePath = request.source.path->c_str();
    DebugManager::instance()->removeBreakpointInFile(sourcePath);
    if (request.breakpoints.has_value()) {
        dap::array<dap::Breakpoint> breakpoints;
        for (auto &breakpoint : request.breakpoints.value()) {
            dap::Breakpoint bp;
            dap::Source source;
            QString location;
            if (request.source.path.has_value()) {
                source = request.source;
            }
            location = source.path.value().c_str();
            location.append(":");
            location.append(QString::number(breakpoint.line));
            emit GDBProxy::instance()->sigBreakInsert(location);

            bp.line = breakpoint.line;
            bp.source = request.source;
            breakpoints.push_back(bp);
        }

        // Generic response
        Log("--> Server sent  setBreakpoints response to client\n");
        response.breakpoints = breakpoints;
    }

    // return empty response to client, when lines and breakpoints all empty.
    // Generic response
    Log("--> Server sent  setBreakpoints response to client\n");
    return response;
}

InitializeResponse DapSession::handleInitializeReq(const InitializeRequest &request)
{
    Q_UNUSED(request)   //! TODO(Mozart):get more info from here.

    Log("<-- Server received initialize request from client\n");
    dap::InitializeResponse response {};
    response.supportsConfigurationDoneRequest = true;
    response.supportsFunctionBreakpoints = true;
    response.supportsInstructionBreakpoints = true;
    response.supportsDataBreakpoints = true;
    response.supportsConditionalBreakpoints = true;
    response.supportsRestartRequest = true;
    response.supportsTerminateRequest = true;
    response.supportTerminateDebuggee = true;
    response.supportsCompletionsRequest = true;

    Log("--> Server sent initialize response to client\n");
    return response;
}
