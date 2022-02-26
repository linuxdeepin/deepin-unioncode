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
#include "protocol_extend.h"
#include "debugmanager.h"
#include "gdbproxy.h"

#include <QDebug>

constexpr int kPort = 4711;
using namespace dap;
DapSession::DapSession()
{
    qRegisterMetaType<std::shared_ptr<dap::ReaderWriter>>("std::shared_ptr<dap::ReaderWriter>");
}

bool DapSession::start()
{
    auto onClientConnected = [this](const std::shared_ptr<ReaderWriter> &socket) {
        QMetaObject::invokeMethod(this, "initialize", Q_ARG(std::shared_ptr<dap::ReaderWriter>, socket));
    };

    auto onError = [](const char *errMessage) {
        Log(errMessage);
    };

    // Create the network server
    server = dap::net::Server::create();
    // Start listening on kPort.
    // onClientConnected will be called when a client wants to connect.
    // onError will be called on any connection errors.
    return server->start(kPort, onClientConnected, onError);
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

    // Output Event and Module Event
    connect(debugger, &DebugManager::streamDebugInternal, [&](const QString sOut) mutable {
        handleEvent(sOut);
    });

    connect(debugger, &DebugManager::asyncStopped, GDBProxy::instance(), [this](const gdb::AsyncContext& ctx) mutable {
        handleAsyncStopped(ctx);
    });

    connect(debugger, &DebugManager::threadGroupAdded, GDBProxy::instance(), [this](const gdb::Thread& thid) mutable {
        handleThreadGroupAdded(thid);
    });

    connect(debugger, &DebugManager::threadGroupExited, GDBProxy::instance(), [this](const gdb::Thread& thid, const QString& exitCode) mutable {
        handleThreadGroupExited(thid, exitCode);
    });

    connect(debugger, &DebugManager::threadGroupRemoved, GDBProxy::instance(), [this](const gdb::Thread& thid) mutable {
        handleThreadGroupRemoved(thid);
    });

    connect(debugger, &DebugManager::threadGroupStarted, GDBProxy::instance(), [this](const gdb::Thread& thid, const gdb::Thread& pid) mutable {
        handleThreadGroupStarted(thid, pid);
    });

    connect(debugger, &DebugManager::libraryLoaded, GDBProxy::instance(), [this](const gdb::Library& library) mutable {
        handleLibraryLoaded(library);
    });

    connect(debugger, &DebugManager::libraryUnloaded, GDBProxy::instance(), [this](const gdb::Library& library) mutable {
        handleLibraryUnloaded(library);
    });

    QObject::connect(debugger, &DebugManager::streamConsole, [&](const QString& text) mutable {
        handleStreamConsole(text);
    });

    connect(debugger, &DebugManager::gdbProcessStarted, GDBProxy::instance(), [this](){isGdbProcessStarted = true;});
    connect(GDBProxy::instance(), &GDBProxy::sigQuit, debugger, &DebugManager::quit);
    connect(GDBProxy::instance(), &GDBProxy::sigStart, debugger, &DebugManager::execute);
    connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert);
    connect(GDBProxy::instance(), &GDBProxy::sigLaunchLocal, debugger, &DebugManager::launchLocal);
    connect(GDBProxy::instance(), &GDBProxy::sigContinue, debugger, &DebugManager::commandContinue);
    connect(GDBProxy::instance(), &GDBProxy::sigPause, debugger, &DebugManager::commandPause);
    connect(GDBProxy::instance(), &GDBProxy::sigNext, debugger, &DebugManager::commandNext);
    connect(GDBProxy::instance(), &GDBProxy::sigStepin, debugger, &DebugManager::commandStep);
    connect(GDBProxy::instance(), &GDBProxy::sigStepout, debugger, &DebugManager::commandFinish);
    connect(GDBProxy::instance(), &GDBProxy::sigThreads, debugger, &DebugManager::threadInfo);
    connect(GDBProxy::instance(), &GDBProxy::sigSelectThread, debugger, &DebugManager::threadSelect);
    connect(GDBProxy::instance(), &GDBProxy::sigStackTrace, debugger, &DebugManager::stackListFrames);
    connect(GDBProxy::instance(), &GDBProxy::sigScopes, debugger, &DebugManager::stackListVariables);
    connect(GDBProxy::instance(), &GDBProxy::sigVariables, debugger, &DebugManager::stackListLocals);
    connect(GDBProxy::instance(), &GDBProxy::sigSource, debugger, &DebugManager::listSourceFiles);
}

void DapSession::registerHanlder()
{
    //qint64 pid; // gdb process pid
    dap::array<dap::Thread> threads;
    dap::Thread currentThread;

    // The Initialize request is the first message sent from the client and
    // the response reports debugger capabilities.
    session->registerHandler([&](const dap::InitializeRequest &request) {
        return handleInitializeReq(request);
    });

    // The SetExceptionBreakpoints request instructs the debugger to set a exception breakpoints
    session->registerHandler([&](const dap::SetExceptionBreakpointsRequest &request) {
        Q_UNUSED(request);
        printf("<-- Server received setExceptionBreakpoints request from client\n");
        dap::SetExceptionBreakpointsResponse response;

        printf("--> Server sent setExceptionBreakpoints response to client\n");
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
        printf("<-- Server received setFunctionBreakpoints request from client\n");
        auto breakpoints = request.breakpoints;
        for (auto &breakpoint : breakpoints) {
            auto functionName = breakpoint.name;
            qInfo() << functionName.c_str();
            emit GDBProxy::instance()->sigBreakInsert(QString("/home/zhxiao/workspaces/qtcreator/demo/main.cpp:swap"));
        }
        // Generic setFunctionBreakpointResponse
        printf("--> Server sent setFunctionBreakpoints response to client\n");
        return response;
    });

    // The SetDataBreakpoints request instructs the debugger to set a data breakpoints
    session->registerHandler([&](const dap::SetDataBreakpointsRequest &request) {
        Q_UNUSED(request);
        dap::SetDataBreakpointsResponse response;
        printf("<-- Server received SetDataBreakpoints request from client\n");

        printf("--> Server sent SetDataBreakpoints response to client\n");
        return response;
    });

    // Signal used to configurate the server session when ConfigurationDoneReqeust
    // The ConfigurationDone request is made by the client once all configuration
    // requests have been made.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_ConfigurationDone
    session->registerHandler([&](const dap::ConfigurationDoneRequest &request) {
        Q_UNUSED(request);
        printf("<-- Server received configurationDone request from client\n");
        isConfiguratedDone = true;
        auto response = dap::ConfigurationDoneResponse();
        printf("--> Server sent configurationDone response to client\n");
        return response;
    });

    // execute debugger and debuggee after configurate done response
    session->registerSentHandler([&](const dap::ResponseOrError<dap::ConfigurationDoneResponse>& response){
        Q_UNUSED(response);
        if (isConfiguratedDone && isDebuggeIsStartWithLaunchRequest && !isLaunchLocalTarget) {
            emit GDBProxy::instance()->sigLaunchLocal();
        }
        isLaunchLocalTarget = true;
        connect(debugger, &DebugManager::asyncRunning, GDBProxy::instance(), [this](const QString& thid) mutable {
            processId = debugger->getProcessId();
            threadId = processId;
            dap::integer pointerSize;
            dap::ProcessEvent processEvent;
            processEvent.name = processName.toStdString();
            processEvent.startMethod = "launch";
            processEvent.systemProcessId = processId;
            processEvent.pointerSize = pointerSize;
            if (isLaunchLocalTarget) {
                session->send(processEvent);
                printf("--> Server sent process Event to client\n");
            }

            dap::ThreadEvent threadEvent;
            threadEvent.reason = "started";
            threadEvent.threadId = processId;
            if (isLaunchLocalTarget) {
                session->send(threadEvent);
                printf("--> Server sent thread Event to client\n");
            }
        });
    });

    // The Launch request is made when the client instructs the debugger adapter
    // to start the debuggee. This request contains the launch arguments.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Launch
    session->registerHandler([&](const dap::LaunchRequestExtend &request) {
        Q_UNUSED(request);
        dap::LaunchResponse response {};
        printf("<-- Server received launch request from client\n");
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
        printf("--> Server sent launch response to client\n");
        return response;
    });

    session->registerSentHandler([&](const dap::ResponseOrError<dap::LaunchResponse> &response) {
        Q_UNUSED(response);
        printf("--> Server sent initialized event to client\n");
        session->send(dap::InitializedEvent());
    });

    // The Attach request
    session->registerHandler([=](const dap::AttachRequest &request) {
        Q_UNUSED(request);
        printf("<-- Server received attach reqeust from client\n");
        isDebuggeIsStartWithAttachRequest = true;
        dap::AttachResponse response;
        printf("--> Server sent attach response to client\n");
        return response;
    });

    // The Restart request
    // Restarts a debug session. Clients should only call this request if the capability
    // ‘supportsRestartRequest’ is true
    session->registerHandler([=](const dap::RestartRequest &request) {
        Q_UNUSED(request);
        dap::RestartResponse response;
        printf("<-- Server received restart request from client\n");

        isDebuggeIsStartWithLaunchRequest = true;
        // send quit signal to debugger
        emit GDBProxy::instance()->sigQuit();

        debugger->setGdbCommand(debuggerName);
        debugger->setGdbArgs(QStringList(processName));
        emit GDBProxy::instance()->sigStart();
        emit GDBProxy::instance()->sigBreakInsert("main");
        emit GDBProxy::instance()->sigLaunchLocal();

        printf("--> Server sent restart response to client\n");
        return response;
    });

    // The Terminate requestis sent from the client to the debug adapter
    //in order to give the debuggee a chance for terminating itself.
    // map to Debug Plugin Menu: Abort Debugging
    session->registerHandler([=](const dap::TerminateRequest &request) {
        Q_UNUSED(request);
        dap::TerminateResponse response;
        printf("<-- Server received terminate request from client\n");
        // send quit command to debugger
        emit GDBProxy::instance()->sigQuit();
        printf("--> Server sent terminate response to client\n");
        return response;
    });

    // The Pause request instructs the debugger to pause execution of one or all
    // threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pause
    session->registerHandler([&](const dap::PauseRequest &request) {
        Q_UNUSED(request);
        printf("<-- Server received pause request from client\n");
        emit GDBProxy::instance()->sigPause();
        printf("--> Server sent pause response to client\n");
        return dap::PauseResponse();
    });

    // The Continue request instructs the debugger to resume execution of one or
    // all threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
    session->registerHandler([&](const dap::ContinueRequest &request) {
        Q_UNUSED(request);
//        auto threadId = request.threadId;
        printf("<-- Server received continue request from client\n");
        emit GDBProxy::instance()->sigContinue();
        printf("--> Server received continue request from client\n");
        return dap::ContinueResponse();
    });

    // The Next request instructs the debugger to single line step for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Next
    session->registerHandler([&](const dap::NextRequest &request) {
        Q_UNUSED(request);
//        auto threadId = request.threadId;
//        auto granularity = request.granularity;
        printf("<-- Server received next request from client\n");
        emit GDBProxy::instance()->sigNext();
        printf("--> Server sent to  next response client\n");
        return dap::NextResponse();
    });

    // The StepIn request instructs the debugger to step-in for a specific thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
    session->registerHandler([&](const dap::StepInRequest &request) {
        Q_UNUSED(request);
//        auto targetId = request.targetId;
//        auto threadId = request.threadId;
//        auto granularity = request.granularity;
        printf("<-- Server received stepin request from client\n");
        emit GDBProxy::instance()->sigStepin();
        printf("--> Server sent stepin response to client\n");
        return dap::StepInResponse();
    });

    // The StepOut request instructs the debugger to step-out for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepOut
    session->registerHandler([&](const dap::StepOutRequest &request) {
        Q_UNUSED(request);
//        auto threadId = request.threadId;
//        auto granularity = request.granularity;
        printf("<-- Server received stepout request from client\n");
        emit GDBProxy::instance()->sigStepout();
        printf("--> Server sent stepout response to client\n");
        return dap::StepOutResponse();
    });

    // The BreakpointLocations request returns all possible locations for source breakpoints in a range
    session->registerHandler([&](const dap::BreakpointLocationsRequest &request) {
        Q_UNUSED(request);
        printf("<-- Server received BreakpointLocations  request from client\n");
        dap::BreakpointLocationsResponse response;
        printf("--> Server sent BreakpointLocations response to client\n");
        return response;
    });

    // The Threads request queries the debugger's list of active threads.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Threads
    session->registerHandler([&](const dap::ThreadsRequest &request) {
        Q_UNUSED(request);
        isThreadRequestReceived = true;
        dap::ThreadsResponse response;
        printf("<-- Server recevied Thread request from client\n");
        emit GDBProxy::instance()->sigThreads();
//        connect(debugger, &DebugManager::updateThreads, this,  [&](int currentId, const QList<gdb::Thread>& threads) mutable {
//            Q_UNUSED(currentId);
//            dap::ThreadsResponse response;
//            for (auto& thread : threads) {
//                dap::Thread thid;
//                thid.id = threadId;
//                thid.name = processName.toStdString();
//                response.threads.push_back(thid);
//            }
//        });

        printf("--> Server sent Thread response to client\n");
        dap::Thread thread;
        thread.id = processId;
        thread.name = processName.toStdString();
        dap::array<dap::Thread> threads;
        threads.push_back(thread);
        response.threads = threads;
        return response;
    });

    // The request returns a stacktrace from the current execution state of a given thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StackTrace
    session->registerHandler([&](const dap::StackTraceRequest &request)
                    -> dap::ResponseOrError<dap::StackTraceResponse> {
        Q_UNUSED(request);
//        auto threadId = request.threadId;
//        auto startFrame = request.startFrame;
//        auto levels = request.levels;
        printf("<-- Server received StackTrace request from the client\n");
        dap::ResponseOrError<dap::StackTraceResponse> response;
        emit GDBProxy::instance()->sigStackTrace();
        connect(debugger, &DebugManager::updateStackFrame, this, [&](const QList<gdb::Frame>& stackFrames) mutable {
            dap::StackFrame sf;
            for (auto& stackFrame : stackFrames) {
                sf.id = 1000;
                sf.name = stackFrame.func.toStdString();
                sf.source->name = stackFrame.file.toStdString();
                sf.source->path = stackFrame.fullpath.toStdString();
                sf.line = stackFrame.line;
                sf.column = 1;
                // sf.instructionPointerReference = stackFrame.addr;
                response.response.stackFrames.push_back(sf);
            }
            return response;
        });
        printf("--> Server sent StackTrace response to the client\n");
        return StackTraceResponse();
    });

    // The Scopes(all variables in selected thread and frame) request reports all the scopes of the given stack frame.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Scopes
    session->registerHandler([&](const dap::ScopesRequest &request)
                                     -> dap::ResponseOrError<dap::ScopesResponse> {
        Q_UNUSED(request);
//        auto frameId = request.frameId;
        printf("<-- Server received Scopes request from the client\n");
        dap::ResponseOrError<dap::ScopesResponse> response;
        emit GDBProxy::instance()->sigScopes();
        connect(debugger, &DebugManager::updateLocalVariables, this, [&](const QList<gdb::Variable>& variableList) mutable {
            dap::array<dap::Scope> scopes;
            for (auto& variable : variableList) {
                dap::Scope scope;
                scope.name = variable.name.toStdString();
                scopes.push_back(scope);
            }
            response.response.scopes = scopes;
        });
        printf("--> Server sent Scopes response to the client\n");
        return response;
    });

    // Retrieves all child variables for the given variable reference(the given scope).
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Variables
    session->registerHandler([&](const dap::VariablesRequest &request)
                                     -> dap::ResponseOrError<dap::VariablesResponse> {
        Q_UNUSED(request);
//        auto count = request.count;
//        auto start = request.start;
//        auto filter = request.filter;
//        auto variableReference = request.variablesReference;
        printf("<-- Server received Variables request from the client\n");
        ResponseOrError<dap::VariablesResponse> response;
        emit GDBProxy::instance()->sigVariables();
        connect(debugger, &DebugManager::variablesChanged, this, [&](const QStringList& changedNames) mutable {
            dap::array<dap::Variable> variables;
            for (auto& name : changedNames) {
                dap::Variable variable;
                variable.name = name.toStdString();
                variables.push_back(variable);
            }
            response.response.variables = variables;
        });
        printf("--> Server sent Variables response to the client\n");
        return response;
    });

    // The Source request retrieves the source code for a given source file.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Source
    session->registerHandler([&](const dap::SourceRequest &request)
                                     -> dap::ResponseOrError<dap::SourceResponse> {
        Q_UNUSED(request);
        dap::ResponseOrError<dap::SourceResponse> response;
        printf("<-- Server received Source request from the client\n");
        emit GDBProxy::instance()->sigSource();
        printf("--> Server sent Source response to the client\n");
        return response;
    });

    // The Disconnect request is made by the client before it disconnects
    // from the server.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Disconnect
    // map to Debug Plugin Menu: Deattach
    session->registerHandler([&](const dap::DisconnectRequest &request) {
        Q_UNUSED(request);
        printf("<-- Server received disconnnect request from client\n");
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

void DapSession::handleAsyncStopped(const gdb::AsyncContext& ctx)
{
    dap::StoppedEvent stoppedEvent;
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
                break;
            }
            case gdb::AsyncContext::Reason::exitedSignalled: {
                stoppedEvent.reason = "exited-signalled";
                break;
            }
            case gdb::AsyncContext::Reason::signalReceived: {
                stoppedEvent.reason = "signal-received";
                break;
            }
            case gdb::AsyncContext::Reason::Unknown: {
                stoppedEvent.reason = "unknown";
                break;
            }
            case gdb::AsyncContext::Reason::readWatchpointTrigger: {
                stoppedEvent.reason = "read-watchporint-trigger";
                break;
            }
                case gdb::AsyncContext::Reason::accessWatchpointTrigger:{
                stoppedEvent.reason = "access-wathcpoint-trigger";
                break;
            }
                case gdb::AsyncContext::Reason::functionFinished:{
                stoppedEvent.reason = "function-finished";
                break;
            }
                case gdb::AsyncContext::Reason::locationReached:{
                stoppedEvent.reason = "location-reached";
                break;
            }
                case gdb::AsyncContext::Reason::watchpointScope:{
                stoppedEvent.reason = "wathcpoint-scope";
                break;
            }
                case gdb::AsyncContext::Reason::exec:{
                stoppedEvent.reason = "exec";
                break;
            }
                case gdb::AsyncContext::Reason::exited:{
                stoppedEvent.reason = "exit";
                break;
            }
                case gdb::AsyncContext::Reason::solibEvent:{
                stoppedEvent.reason = "solib-event";
                break;
            }
                case gdb::AsyncContext::Reason::fork:{
                stoppedEvent.reason = "fork";
                break;
            }
                case gdb::AsyncContext::Reason::vfork:{
                stoppedEvent.reason = "vfork";
                break;
            }
                case gdb::AsyncContext::Reason::syscallEntry:{
                stoppedEvent.reason = "syscall-entry";
                break;
            }
                case gdb::AsyncContext::Reason::syscallReturn:{
                stoppedEvent.reason = "syscall-return";
                break;
            }
                case gdb::AsyncContext::Reason::watchpointTrigger:{
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
     session->send(stoppedEvent);
     printf("--> Server sent stopped Event to client\n");
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
//    processId = pid.id;
//    threadId = pid.id + thid.id;
}

void DapSession::handleThreadGroupExited(const gdb::Thread &thid, const QString &exitCode)
{
    qInfo() << "thread group exited\n";
}

void DapSession::hanleThreadCreated(const gdb::Thread &thid, const QString &groupId)
{
//    threadId = processId + thid.id;
//    threadGroupId = groupId;
}

void DapSession::handleThreadExited(const gdb::Thread &thid, const QString &groupId)
{
//    threadId = processId + thid.id;
//    threadGroupId = groupId;
}

void DapSession::handleThreadSelected(const gdb::Thread &thid, const gdb::Frame &frame)
{

}

void DapSession::hanldeUpdateThreads(int currentId, const QList<gdb::Thread> &threads)
{

}

void DapSession::handleLibraryLoaded(const gdb::Library& library)
{
    dap::OutputEvent outputEvent;
    dap::ModuleEvent moduleEvent;
    outputEvent.category = "console";
    auto targetName = library.targetName;
    outputEvent.output = QString{"Loaded %1. Symbols loaded.\n"}.arg(targetName).toStdString();
    moduleEvent.reason = "new";
    dap::Module module;
    module.id = library.id.toStdString();
    module.name = library.targetName.toStdString();
    module.path = library.hostName.toStdString();
    module.symbolFilePath = library.hostName.toStdString();
    moduleEvent.module = module;
    session->send(outputEvent);
    printf("--> Server sent output event to client\n");
    session->send(moduleEvent);
    printf("--> Server sent module event to client\n");
}

void DapSession::handleLibraryUnloaded(const gdb::Library &library)
{

}

void DapSession::handleStreamConsole(const QString &text)
{
    if (isLaunchLocalTarget) {
        dap::OutputEvent outputEvent;
        QString output = text;
        outputEvent.category = "stdout";
        outputEvent.output = output.toStdString();
        if (isConfiguratedDone) {
            session->send(outputEvent);
            printf("--> Server sent output event to client\n");
        }
    }
}

void DapSession::handleEvent(const QString &sOut)
{
    QString eventOutput;
    do {
        if (!sOut.contains("gdbCommand")) {
            eventOutput += sOut.split(":").last();
        }

        if (eventOutput.contains("=thread-group-added") && eventOutput.contains("\(gdb\)")) {
            dap::OutputEvent outputEvent;
            outputEvent.output = eventOutput.toStdString();
            if (!isLaunchLocalTarget && isConfiguratedDone) {
                session->send(outputEvent);
                printf("--> Server sent output event to client.\n");
            }
            eventOutput.clear();
        }
    } while (sOut.contains("~\"done.\n"));
}

dap::SetBreakpointsResponse DapSession::handleBreakpointReq(const SetBreakpointsRequest &request)
{
    Q_UNUSED(request);
    printf("<-- Server received setBreakpoints request from client\n");
    // breakpoints : [{"line": 26}, {"line": 30}]
    // source : {"path": "/home/zhxiao/workspaces/qtcreator/demo/main.cpp"}

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
        dap::SetBreakpointsResponse response;

        // Generic response
        printf("--> Server sent  setBreakpoints response to client\n");
        response.breakpoints = breakpoints;
        return response;
    }

    // return empty response to client, when lines and breakpoints all empty.
    // Generic response
    printf("--> Server sent  setBreakpoints response to client\n");
    dap::SetBreakpointsResponse response;
    return response;
}

InitializeResponse DapSession::handleInitializeReq(const InitializeRequest &request)
{
    Q_UNUSED(request) //! TODO(Mozart):get more info from here.

    printf("<-- Server received initialize request from client\n");
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

    printf("--> Server sent initialize response to client\n");
    return  response;
}
