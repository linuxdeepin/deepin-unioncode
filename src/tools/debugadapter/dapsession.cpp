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
#include "debugmanager.h"
#include "dapproxy.h"
#include "locker.h"

#include "dap/protocol.h"
#include "dap/session.h"
#include "dap/network.h"

#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QtDBus/QDBusConnection>
#include <QList>
#include <QMetaObject>
#include <QDBusMessage>

// keep the initialization in order.
static ConditionLock configured;
const int maxTryNum = 100;
using namespace dap;

class DapSessionPrivate
{
    friend class DapSession;

    std::unique_ptr<dap::Session> session;
    std::unique_ptr<dap::net::Server> server;

    DebugManager *debugger{nullptr};
    ServerInfo serverInfo;

    bool isDebuggeIsStartWithLaunchRequest = false;
    bool isDebuggeIsStartWithAttachRequest = false;
    bool isSupportsTerminateDebuggee = true;
    bool isGDbProcessTerminated = false;
    bool isThreadRequestReceived = false;
    bool isInferiorStopped = false;

    QString uuid;
};

DapSession::DapSession(QObject *parent)
    : QObject(parent)
    , d (new DapSessionPrivate())
{
    qRegisterMetaType<std::shared_ptr<dap::ReaderWriter>>("std::shared_ptr<dap::ReaderWriter>");
    registerDBusConnect();

    initializeDebugMgr();
}

DapSession::~DapSession()
{

}

void DapSession::registerDBusConnect()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.disconnect(QString(""),
                          "/path",
                          "com.deepin.unioncode.interface",
                          "get_cxx_dapport",
                          this, SLOT(slotReceiveClientInfo(QString)));
    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "get_cxx_dapport",
                       this, SLOT(slotReceiveClientInfo(QString)));


    connect(this, &DapSession::sigSendToClient, [](const QString &uuid, int port) {
        QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                      "com.deepin.unioncode.interface",
                                                      "dapport");
        msg << uuid
            << port
            << QString()
            << QString()
            << QStringList();
        QDBusConnection::sessionBus().send(msg);
    });
}

void DapSession::slotReceiveClientInfo(const QString &uuid)
{
    emit sigSendToClient(uuid, d->serverInfo.port());
}

bool DapSession::start()
{
    auto onClientConnected = [this](const std::shared_ptr<ReaderWriter> &socket) {
        QMetaObject::invokeMethod(this, "initialize",  Qt::BlockingQueuedConnection, Q_ARG(std::shared_ptr<dap::ReaderWriter>, socket));
    };

    auto onError = [](const char *errMessage) {
        Log(errMessage)
    };

    // Create the network server
    d->server = dap::net::Server::create();

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
            ret = d->server->start(port, onClientConnected, onError);
            if (ret) {
                d->serverInfo.setPort(port);
                break;
            }
        }
        port++;
    }
    return ret;
}

void DapSession::stop()
{
    d->server->stop();
}

void DapSession::initialize(std::shared_ptr<dap::ReaderWriter> socket)
{
    d->session.release();
    d->session = dap::Session::create();
    d->session->bind(socket);

    registerHanlder();

    Log("DapSession initialized.\n")
}

void DapSession::initializeDebugMgr()
{
    // instance a debug manager object
    if (!d->debugger)
        d->debugger = DebugManager::instance();

    Qt::ConnectionType SequentialExecution = Qt::BlockingQueuedConnection;

    // Output Event and Module Event
    connect(d->debugger, &DebugManager::streamDebugInternal, [&](const QStringList &textList) mutable {
        handleOutputTextEvent(textList);
    });

    connect(d->debugger, &DebugManager::asyncStopped, DapProxy::instance(), [this](const dap::StoppedEvent &stoppedEvent) mutable {
        handleAsyncStopped(stoppedEvent);
        d->isInferiorStopped = true;
    });

    connect(d->debugger, &DebugManager::asyncExited, DapProxy::instance(), [this](const dap::ExitedEvent &exitedEvent) mutable {
        handleAsyncExited(exitedEvent);
        d->isInferiorStopped = true;
    });

    connect(d->debugger, &DebugManager::asyncRunning, DapProxy::instance(), [this](const QString &) mutable {
        d->isInferiorStopped = false;
    });

    connect(d->debugger, &DebugManager::libraryLoaded, DapProxy::instance(), [this](const dap::ModuleEvent &moduleEvent) mutable {
        handleLibraryLoaded(moduleEvent);
    });

    connect(d->debugger, &DebugManager::libraryUnloaded, DapProxy::instance(), [this](const dap::ModuleEvent &moduleEvent) mutable {
        handleLibraryUnloaded(moduleEvent);
    });

    QObject::connect(d->debugger, &DebugManager::streamConsole, [&](const QString &text) mutable {
        handleStreamConsole(text);
    });

    connect(DapProxy::instance(), &DapProxy::sigQuit, d->debugger, &DebugManager::quit, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigKill, d->debugger, &DebugManager::kill, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigStart, d->debugger, &DebugManager::execute, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigBreakInsert, d->debugger, &DebugManager::breakInsert, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigLaunchLocal, d->debugger, &DebugManager::launchLocal, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigContinue, d->debugger, &DebugManager::commandContinue, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigPause, d->debugger, &DebugManager::commandPause, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigNext, d->debugger, &DebugManager::commandNext, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigStepin, d->debugger, &DebugManager::commandStep, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigStepout, d->debugger, &DebugManager::commandFinish, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigThreads, d->debugger, &DebugManager::threadInfo, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigSelectThread, d->debugger, &DebugManager::threadSelect, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigStackTrace, d->debugger, &DebugManager::stackListFrames, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigVariables, d->debugger, &DebugManager::stackListVariables, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigSource, d->debugger, &DebugManager::listSourceFiles, SequentialExecution);
    connect(DapProxy::instance(), &DapProxy::sigBreakRemoveAll, d->debugger, &DebugManager::breakRemoveAll, SequentialExecution);
}

void DapSession::registerHanlder()
{
    // The Initialize request is the first message sent from the client and
    // the response reports debugger capabilities.
    d->session->registerHandler([&](const dap::InitializeRequest &request) {
        return handleInitializeReq(request);
    });

    // The SetExceptionBreakpoints request instructs the debugger to set a exception breakpoints
    d->session->registerHandler([&](const dap::SetExceptionBreakpointsRequest &request) {
        Q_UNUSED(request)
        Log("<-- Server received setExceptionBreakpoints request from client\n")
        dap::SetExceptionBreakpointsResponse response;

        Log("--> Server sent setExceptionBreakpoints response to client\n")
        return response;
    });

    // The SetBreakpoints request instructs the debugger to clear and set a number
    // of line breakpoints for a specific source file.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_SetBreakpoints
    d->session->registerHandler([&](const dap::SetBreakpointsRequest &request) {
        return handleBreakpointReq(request);
    });

    // The SetFunctionBreakpointBreakpoints request instructs the debugger to set a function breakpoints
    d->session->registerHandler([&](const dap::SetFunctionBreakpointsRequest &request) {
        Q_UNUSED(request)
        dap::SetFunctionBreakpointsResponse response;
        Log("<-- Server received setFunctionBreakpoints request from client\n")
        auto breakpoints = request.breakpoints;
        for (auto &breakpoint : breakpoints) {
            auto functionName = breakpoint.name;
            qInfo() << functionName.c_str();
            if (!functionName.empty()) {
                d->debugger->breakInsert(functionName.c_str());
            }
        }
        // Generic setFunctionBreakpointResponse
        Log("--> Server sent setFunctionBreakpoints response to client\n")
        return response;
    });

    // The SetDataBreakpoints request instructs the debugger to set a data breakpoints
    d->session->registerHandler([&](const dap::SetDataBreakpointsRequest &request) {
        Q_UNUSED(request)
        dap::SetDataBreakpointsResponse response;
        Log("<-- Server received SetDataBreakpoints request from client\n")

        Log("--> Server sent SetDataBreakpoints response to client\n")
        return response;
    });

    // Signal used to configurate the server session when ConfigurationDoneReqeust
    // The ConfigurationDone request is made by the client once all configuration
    // requests have been made.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_ConfigurationDone
    d->session->registerHandler([&](const dap::ConfigurationDoneRequest &request) {
        Q_UNUSED(request)
        Log("<-- Server received configurationDone request from client\n")
        auto response = dap::ConfigurationDoneResponse();
        Log("--> Server sent configurationDone response to client\n")
        return response;
    });

    // execute debugger and debuggee after configurate done response
    d->session->registerSentHandler([&](const dap::ResponseOrError<dap::ConfigurationDoneResponse> &response) {
        Q_UNUSED(response)
        emit DapProxy::instance()->sigLaunchLocal();

        connect(d->debugger, &DebugManager::asyncRunning, DapProxy::instance(),
                [this](const QString& processName, const QString& theadId) mutable {
            // TODO(Any):multi-thread condition should be done.
            Q_UNUSED(theadId)
            dap::integer pointerSize;
            dap::ProcessEvent processEvent;
            processEvent.name = processName.toStdString();
            processEvent.startMethod = "launch";
            processEvent.systemProcessId = d->debugger->getProcessId();
            processEvent.pointerSize = pointerSize;
            d->session->send(processEvent);
            Log("--> Server sent process Event to client\n")

            configured.fire();
        }, Qt::UniqueConnection);
    });

    // The Launch request is made when the client instructs the debugger adapter
    // to start the debuggee. This request contains the launch arguments.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Launch
    d->session->registerHandler([&](const dap::LaunchRequest &request) {
        Q_UNUSED(request)
        dap::LaunchResponse response {};
        Log("<-- Server received launch request from client\n")
        d->isDebuggeIsStartWithLaunchRequest = true;

        if (request.name.has_value() && request.program.has_value() ) {
            d->debugger->initDebugger(request.name.value().c_str(), QStringList{request.program.value().c_str()});
        }
        emit DapProxy::instance()->sigStart();
        auto message = QString{"gdb process started with pid: %1\n"}.arg(d->debugger->getProcessId());
        Log(message.toStdString().c_str())
        Log("--> Server sent launch response to client\n")
        return response;
    });

    d->session->registerSentHandler([&](const dap::ResponseOrError<dap::LaunchResponse> &response) {
        Q_UNUSED(response)
        Log("--> Server sent initialized event to client\n")
        d->session->send(dap::InitializedEvent());
    });

    // The Attach request
    d->session->registerHandler([=](const dap::AttachRequest &request) {
        Q_UNUSED(request)
        Log("<-- Server received attach reqeust from client\n")
        d->isDebuggeIsStartWithAttachRequest = true;
        dap::AttachResponse response;
        Log("--> Server sent attach response to client\n")
        return response;
    });

    // The Restart request
    // Restarts a debug session. Clients should only call this request if the capability
    // ‘supportsRestartRequest’ is true
    d->session->registerHandler([=](const dap::RestartRequest &request) {
        Q_UNUSED(request)
        dap::RestartResponse response;
        Log("<-- Server received restart request from client\n")

        d->isDebuggeIsStartWithLaunchRequest = true;
        // kill current process.
        emit DapProxy::instance()->sigKill();
        // re-launch local.
        emit DapProxy::instance()->sigLaunchLocal();

        Log("--> Server sent restart response to client\n")
        return response;
    });

    // The Terminate requestis sent from the client to the debug adapter
    //in order to give the debuggee a chance for terminating itself.
    // map to Debug Plugin Menu: Abort Debugging
    d->session->registerHandler([=](const dap::TerminateRequest &request) {
        Q_UNUSED(request)
        dap::TerminateResponse response;
        Log("<-- Server received terminate request from client\n")
        // send quit command to debugger
        emit DapProxy::instance()->sigQuit();

        dap::TerminatedEvent terminatedEvent;
        d->session->send(terminatedEvent);
        Log("--> Server sent terminate response to client\n")
        return response;
    });

    // The Pause request instructs the debugger to pause execution of one or all
    // threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pause
    d->session->registerHandler([&](const dap::PauseRequest &request) {
        Q_UNUSED(request)
        Log("<-- Server received pause request from client\n")
        emit DapProxy::instance()->sigPause();
        Log("--> Server sent pause response to client\n")
        return dap::PauseResponse();
    });

    // The Continue request instructs the debugger to resume execution of one or
    // all threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
    d->session->registerHandler([&](const dap::ContinueRequest &request) {
        Q_UNUSED(request)
        //        auto threadId = request.threadId;
        Log("<-- Server received continue request from client\n")
        emit DapProxy::instance()->sigContinue();
        Log("--> Server received continue request from client\n")
        return dap::ContinueResponse();
    });

    // The Next request instructs the debugger to single line step for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Next
    d->session->registerHandler([&](const dap::NextRequest &request) {
        Q_UNUSED(request)
        Log("<-- Server received next request from client\n")
        emit DapProxy::instance()->sigNext();
        Log("--> Server sent to  next response client\n")
        return dap::NextResponse();
    });

    // The StepIn request instructs the debugger to step-in for a specific thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
    d->session->registerHandler([&](const dap::StepInRequest &request) {
        Q_UNUSED(request)
        //        auto targetId = request.targetId;
        //        auto threadId = request.threadId;
        //        auto granularity = request.granularity;
        Log("<-- Server received stepin request from client\n")
        emit DapProxy::instance()->sigStepin();
        Log("--> Server sent stepin response to client\n")
        return dap::StepInResponse();
    });

    // The StepOut request instructs the debugger to step-out for a specific
    // thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepOut
    d->session->registerHandler([&](const dap::StepOutRequest &request) {
        Q_UNUSED(request)
        //        auto threadId = request.threadId;
        //        auto granularity = request.granularity;
        Log("<-- Server received stepout request from client\n")
        emit DapProxy::instance()->sigStepout();
        Log("--> Server sent stepout response to client\n")
        return dap::StepOutResponse();
    });

    // The BreakpointLocations request returns all possible locations for source breakpoints in a range
    d->session->registerHandler([&](const dap::BreakpointLocationsRequest &request) {
        Q_UNUSED(request)
        Log("<-- Server received BreakpointLocations  request from client\n")
        dap::BreakpointLocationsResponse response;
        Log("--> Server sent BreakpointLocations response to client\n")
        return response;
    });

    // The Threads request queries the debugger's list of active threads.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Threads
    d->session->registerHandler([&](const dap::ThreadsRequest &request) {
        configured.wait();
        Q_UNUSED(request)
        d->isThreadRequestReceived = true;
        dap::ThreadsResponse response;
        Log("<-- Server recevied Thread request from client\n")
        d->debugger->threadInfo();

        Log("--> Server sent Thread response to client\n")
        response.threads = d->debugger->allThreadList();
        return response;
    });

    // The request returns a stacktrace from the current execution state of a given thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StackTrace
    d->session->registerHandler([&](const dap::StackTraceRequest &request)
                                     -> dap::StackTraceResponse {

        auto threadid = request.threadId.operator long();

        // select a given thread id
        // -thread-select 3
        emit DapProxy::instance()->sigSelectThread(static_cast<int>(threadid));

        dap::StackTraceResponse response;
        Log("<-- Server received StackTrace request from the client\n")
        if (d->isInferiorStopped) {
            d->debugger->stackListFrames();
            response.stackFrames = d->debugger->allStackframes();
            Log("--> Server sent StackTrace response to the client\n")
        }

        return response;
    });

    // The Scopes(all variables in selected thread and frame) request reports all the scopes of the given stack frame.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Scopes
    d->session->registerHandler([&](const dap::ScopesRequest &request)
                                     -> dap::ScopesResponse {
        Q_UNUSED(request)
        auto frameId = request.frameId;
        static dap::ScopesResponse response;
        dap::array<dap::Scope> scopes;
        Log("<-- Server received Scopes request from the client\n")
        //emit DapProxy::instance()->sigScopes(frameId);

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
        Log("--> Server sent Scopes response to the client\n")
        return response;
    });

    // Retrieves all child variables for the given variable reference(the given scope).
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Variables
    d->session->registerHandler([&](const dap::VariablesRequest &request)
                                     -> dap::ResponseOrError<dap::VariablesResponse> {
        Q_UNUSED(request)
        Log("<-- Server received Variables request from the client\n")
        ResponseOrError<dap::VariablesResponse> response;
        d->debugger->stackListVariables();
        response.response.variables = d->debugger->allVariableList();
        Log("--> Server sent Variables response to the client\n")
        return response;
    });

    // The Source request retrieves the source code for a given source file.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Source
    d->session->registerHandler([&](const dap::SourceRequest &request)
                                     -> dap::ResponseOrError<dap::SourceResponse> {
        Q_UNUSED(request)
        dap::ResponseOrError<dap::SourceResponse> response;
        Log("<-- Server received Source request from the client\n")
        emit DapProxy::instance()->sigSource();
        Log("--> Server sent Source response to the client\n")
        return response;
    });

    // The Disconnect request is made by the client before it disconnects
    // from the server.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Disconnect
    // map to Debug Plugin Menu: Deattach
    d->session->registerHandler([&](const dap::DisconnectRequest &request) {
        Q_UNUSED(request)
        Log("<-- Server received disconnnect request from client\n")
        if (d->isSupportsTerminateDebuggee) {
            if (d->isDebuggeIsStartWithLaunchRequest) {
                emit DapProxy::instance()->sigQuit();
            } else if (d->isDebuggeIsStartWithAttachRequest) {
                qInfo() << "no supports terminate with attach target\n";
            }
        }
        return dap::DisconnectResponse {};
    });
}

void DapSession::handleAsyncStopped(const dap::StoppedEvent &stoppedEvent)
{
    d->session->send(stoppedEvent);
    Log("--> Server sent stopped event to client\n")
}

void DapSession::handleAsyncExited(const dap::ExitedEvent &exitedEvent)
{
    d->session->send(exitedEvent);
    Log("--> Server sent exited event to client\n")
}

void DapSession::handleThreadExited(const int threadId, const QString &groupId)
{
    Q_UNUSED(groupId)
    dap::ThreadEvent threadEvent;
    threadEvent.reason = "exited";
    threadEvent.threadId = threadId;
    d->session->send(threadEvent);
    Log("--> Server sent thread event to client\n")
}

void DapSession::handleLibraryLoaded(const dap::ModuleEvent &moduleEvent)
{
    dap::OutputEvent outputEvent;
    outputEvent.category = "console";
    QString targetName = QString::fromStdString(moduleEvent.module.name);
    outputEvent.output = QString("Loaded %1. Symbols loaded.\n")
            .arg(targetName).toStdString();
    d->session->send(outputEvent);
    Log("--> Server sent output event to client\n")
    d->session->send(moduleEvent);
    Log("--> Server sent module event to client\n")
}

void DapSession::handleLibraryUnloaded(const dap::ModuleEvent &moduleEvent)
{
    dap::OutputEvent outputEvent;
    outputEvent.category = "console";
    QString targetName = QString::fromStdString(moduleEvent.module.name);
    outputEvent.output = QString("Unloaded %1. Symbols loaded.\n")
            .arg(targetName).toStdString();
    d->session->send(outputEvent);
    Log("--> Server sent output event to client\n")
    d->session->send(moduleEvent);
    Log("--> Server sent module event to client\n")
}

void DapSession::handleStreamConsole(const QString &text)
{
    dap::OutputEvent outputEvent;
    QString output = text;
    outputEvent.category = "stdout";
    outputEvent.output = output.toStdString();
    d->session->send(outputEvent);
    qInfo() << "Console" << output;
    Log("--> Server sent output event to client\n")
}

void DapSession::handleOutputTextEvent(const QStringList &textList)
{
    foreach (QString text, textList) {
        dap::OutputEvent outputEvent;
        outputEvent.output = text.toStdString();
        d->session->send(outputEvent);
        qInfo() << "OutputText" << text;
        Log("--> Server sent output event to client.\n")
    }
}

dap::SetBreakpointsResponse DapSession::handleBreakpointReq(const SetBreakpointsRequest &request)
{
    dap::SetBreakpointsResponse response;

    Log("<-- Server received setBreakpoints request from client\n")
    if (request.source.path->empty()) {
        emit DapProxy::instance()->sigBreakRemoveAll();
        return response;
    }

    const char *sourcePath = request.source.path->c_str();
    d->debugger->removeBreakpointInFile(sourcePath);
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
            emit DapProxy::instance()->sigBreakInsert(location);

            bp.line = breakpoint.line;
            bp.source = request.source;
            breakpoints.push_back(bp);
        }

        // Generic response
        Log("--> Server sent  setBreakpoints response to client\n")
        response.breakpoints = breakpoints;
    }

    // return empty response to client, when lines and breakpoints all empty.
    // Generic response
    Log("--> Server sent  setBreakpoints response to client\n")
    return response;
}

InitializeResponse DapSession::handleInitializeReq(const InitializeRequest &request)
{
    Q_UNUSED(request)   //! TODO(Mozart):get more info from here.

    Log("<-- Server received initialize request from client\n")
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

    Log("--> Server sent initialize response to client\n")
    return response;
}
