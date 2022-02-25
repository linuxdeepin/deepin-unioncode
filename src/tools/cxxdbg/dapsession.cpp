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

    connect(GDBProxy::instance(), &GDBProxy::sigQuit, debugger, &DebugManager::quit);
    connect(GDBProxy::instance(), &GDBProxy::sigStart, debugger, &DebugManager::execute);
    connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert);
    connect(GDBProxy::instance(), &GDBProxy::sigLaunchLocal, debugger, &DebugManager::launchLocal);

    connect(debugger, &DebugManager::gdbProcessStarted, GDBProxy::instance(), /*&GDBProxy::gdbProcessStarted*/[this](){isGdbProcessStarted = true;});
    connect(GDBProxy::instance(), &GDBProxy::sigContinue, debugger, &DebugManager::commandContinue);
    connect(GDBProxy::instance(), &GDBProxy::sigPause, debugger, &DebugManager::command);
    connect(GDBProxy::instance(), &GDBProxy::sigNext, debugger, &DebugManager::commandNext);
    connect(GDBProxy::instance(), &GDBProxy::sigStepin, debugger, &DebugManager::commandStep);
    connect(GDBProxy::instance(), &GDBProxy::sigStepout, debugger, &DebugManager::commandFinish);
    connect(GDBProxy::instance(), &GDBProxy::sigThreads, debugger, &DebugManager::command);
    connect(GDBProxy::instance(), &GDBProxy::sigSelectThread, debugger, &DebugManager::command);
    connect(GDBProxy::instance(), &GDBProxy::sigStackTrace, debugger, &DebugManager::command);
    connect(GDBProxy::instance(), &GDBProxy::sigScopes, debugger, &DebugManager::command);
    connect(GDBProxy::instance(), &GDBProxy::sigVariables, debugger, &DebugManager::command);
    connect(GDBProxy::instance(), &GDBProxy::sigSource, debugger, &DebugManager::command);
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
        //QObject::connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert);
        //emit GDBProxy::instance()->sigBreakInsert(QString("/home/zhxiao/workspaces/qtcreator/demo/main.cpp:main"));

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
        printf("<-- Server received SetDataBreakpoints request from client\n");
        dap::SetDataBreakpointsResponse response;

        printf("--> Server sent SetDataBreakpoints response to client\n");
        return response;
    });

    // Signal used to configurate the server session when ConfigurationDoneReqeust
//    std::condition_variable configurated_cv;
//    std::mutex configurated_mutex;
    // The ConfigurationDone request is made by the client once all configuration
    // requests have been made.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_ConfigurationDone
    session->registerHandler([&](const dap::ConfigurationDoneRequest &request) {
        Q_UNUSED(request);
        printf("<-- Server received configurationDone request from client\n");

//        std::unique_lock<std::mutex> lock(configurated_mutex);
        isConfiguratedDone = true;
//        configurated_cv.notify_one();
        auto response = dap::ConfigurationDoneResponse();

        printf("--> Server sent configurationDone response to client\n");
        return response;
    });

    // The Launch request is made when the client instructs the debugger adapter
    // to start the debuggee. This request contains the launch arguments.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Launch
    //
    // must launch gdb process in the main thread.
    // default, lauch request will stop at entry(main)
    session->registerHandler([&](const dap::LaunchRequestExtend &request) {
        Q_UNUSED(request);
        dap::LaunchResponse response {};

        printf("<-- Server received launch request from client\n");


        isDebuggeIsStartWithLaunchRequest = true;
        if (request.type.has_value()) {
            debugger->setGdbCommand("gdb");
        }
        if (request.program.has_value()) {
            debugger->setGdbArgs(QStringList(request.program.value().c_str()));
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
        //            qint64 pid = 28343; // from attach request
        //            QObject::connect(GDBProxy::instance(), &GDBProxy::sigAttach, debugger, &DebugManager::command);
        //            QString cmd("-target-attach ");
        //            cmd.append(QString::number(pid));
        //            emit GDBProxy::instance()->sigAttach(cmd);

        printf("--> Server sent attach response to client\n");
        return response;
    });

    // The Restart request
    // Restarts a debug session. Clients should only call this request if the capability
    // ‘supportsRestartRequest’ is true
    session->registerHandler([=](const dap::RestartRequest &request) {
        Q_UNUSED(request);
        printf("<-- Server received restart request from client\n");

        isDebuggeIsStartWithLaunchRequest = true;
        dap::RestartResponse response;

        // send quit signal to debugger
        emit GDBProxy::instance()->sigQuit();

        debugger->setGdbCommand("gdb");
        debugger->setGdbArgs({ "input your program path." });
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

        emit GDBProxy::instance()->sigPause("-exec-pause");

        printf("--> Server sent pause response to client\n");
        return dap::PauseResponse();
    });

    // The Continue request instructs the debugger to resume execution of one or
    // all threads.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
    session->registerHandler([&](const dap::ContinueRequest &request) {
        Q_UNUSED(request);
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
        printf("<-- Server received next request from client\n");

        emit GDBProxy::instance()->sigNext();

        printf("--> Server sent to  next response client\n");
        return dap::NextResponse();
    });

    // The StepIn request instructs the debugger to step-in for a specific thread.
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
    session->registerHandler([&](const dap::StepInRequest &request) {
        Q_UNUSED(request);
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
        printf("<-- Server recevied Thread request from client\n");
        dap::ThreadsResponse response;
        dap::Thread thread;
        QString cmd("-thread-list-ids");
        emit GDBProxy::instance()->sigThreads(cmd);

        printf("--> Server sent Thread response to client\n");
        return response;
    });

    // The request returns a stacktrace from the current execution state of a given thread.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StackTrace
    //
    // the given thread id was get from StackTrace request object.
    //
    session->registerHandler(
            [&](const dap::StackTraceRequest &request)
                    -> dap::ResponseOrError<dap::StackTraceResponse> {
                Q_UNUSED(request);
                printf("<-- Server received StackTrace request from the client\n");
                auto threadid = request.threadId.operator long();
                dap::ResponseOrError<dap::StackTraceResponse> response;

                // select a given thread id
                // -thread-select 3
                QString selectThread("-thread-select ");
                selectThread.append(QString::number(threadid));
                emit GDBProxy::instance()->sigSelectThread(selectThread);

                // request all stack frames by omitting
                // the startFrame and levels arguments.
                QString cmd("-stack-list-frames");
                emit GDBProxy::instance()->sigStackTrace(cmd);

                printf("--> Server sent StackTrace response to the client\n");
                return response;
            });

    // The Scopes(all variables in selected thread and frame) request reports all the scopes of the given stack frame.
    //
    // frameId was given by Scope request
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Scopes
    session->registerHandler([&](const dap::ScopesRequest &request)
                                     -> dap::ResponseOrError<dap::ScopesResponse> {
        Q_UNUSED(request);
        printf("<-- Server received Scopes request from the client\n");
        dap::ResponseOrError<dap::ScopesResponse> response {};
        auto threadId = 1;
        auto frameId = request.frameId.operator long();

        // -stack-list-variables --thread threadId --frame frameId --all-values
        QString scopes { "-stack-list-variables --thread " };
        scopes.append(QString::number(threadId));
        scopes.append("--frame ");
        scopes.append(QString::number(frameId));
        scopes.append(" --all-values");
        emit GDBProxy::instance()->sigScopes(scopes);

        printf("--> Server sent Scopes response to the client\n");
        return response;
    });

    // Retrieves all child variables for the given variable reference(the given scope).
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Variables
    session->registerHandler([&](const dap::VariablesRequest &request)
                                     -> dap::ResponseOrError<dap::VariablesResponse> {
        Q_UNUSED(request);
        printf("<-- Server received Variables request from the client\n");
        dap::ResponseOrError<dap::VariablesResponse> response;
        QString variables { "-stack-list-locals" };
        emit GDBProxy::instance()->sigVariables(variables);

        printf("--> Server sent Variables response to the client\n");
        return response;
    });

    // The Source request retrieves the source code for a given source file.
    //
    // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Source
    // gdb list command???
    session->registerHandler([&](const dap::SourceRequest &request)
                                     -> dap::ResponseOrError<dap::SourceResponse> {
        Q_UNUSED(request);
        printf("<-- Server received Source request from the client\n");
        dap::ResponseOrError<dap::SourceResponse> response;
        //auto sourcePath = request.source->path.value().c_str();
        QString source { "-file-list-exec-source-files" };
        emit GDBProxy::instance()->sigSource(source);

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
                //                        QObject::connect(GDBProxy::instance(), &GDBProxy::sigDetach, debugger, &DebugManager::command);
                //                        QString cmd("-target-detach ");
                //                        cmd.append(QString::number(pid));
                //                        emit GDBProxy::instance()->sigDetach(cmd);
            }
        }

        return dap::DisconnectResponse {};
    });
}

void DapSession::handleEvent(const QString &sOut)
{
    QString eventOutput;
    qInfo() << sOut;
    // for =thread-group-added output
    do {
        if (!sOut.contains("gdbCommand")) {
            eventOutput += sOut.split(":").last();
        }

        if (eventOutput.contains("=thread-group-added") && eventOutput.contains("\(gdb\)")) {
            dap::OutputEvent outputEvent;
            outputEvent.output = eventOutput.toStdString();
            session->send(outputEvent);
            eventOutput.clear();
        }
    } while (sOut.contains("~\"done.\n"));

    if (sOut.contains("=thread-group-started")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();

        // sent process event
        dap::ProcessEvent processEvent;
        QString threadGroupId;
        QString processId;
        auto outList = QList(eventOutput.split(","));
        for (auto &out : outList) {
            auto kv = out.remove("\"").remove("\n");
            if (kv.contains("id")) {
                threadGroupId = kv.split("=").last();
                continue;
            }
            if (kv.contains("pid")) {
                processId = kv.split("=").last();
                continue;
            }
        }
        processEvent.startMethod = "launch";
        processEvent.systemProcessId = processId.toInt();
        session->send(processEvent);
        eventOutput.clear();
    }

    if (sOut.contains("-thread-created")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();
        qInfo() << eventOutput;

        dap::ThreadEvent threadEvent;
        threadEvent.threadId = eventOutput.contains("id=");
        session->send(threadEvent);
        eventOutput.clear();
    }

    if (sOut.contains("=library-loaded")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();

        QString libraryId;
        QString targetName;
        QString hostName;
        QString symbolsLoaded;
        QString threadGroupId;
        QString ranges;
        auto outList = QList(eventOutput.split(","));
        for (auto &out : outList) {
            auto kv = out.remove("\n").remove("\"");
            if (kv.contains("id")) {
                libraryId = kv.split("=").last();
                continue;
            }
            if (kv.contains("target-name")) {
                targetName = kv.split("=").last();
                continue;
            }
            if (kv.contains("host-name")) {
                hostName = kv.split("=").last();
                continue;
            }
            if (kv.contains("sysbols-loaded")) {
                symbolsLoaded = kv.split("=").last();
                continue;
            }
            if (kv.contains("thread-group")) {
                threadGroupId = kv.split("=").last();
                continue;
            }
            if (kv.contains("ranges")) {
                ranges = kv.split("=").last();
                continue;
            }
        }

        dap::OutputEvent outputEvent;
        outputEvent.category = "console";
        outputEvent.output = targetName.toStdString();
        session->send(outputEvent);

        dap::ModuleEvent moduleEvent;
        moduleEvent.reason = "new";
        //moduleEvent.module.id = 1;
        moduleEvent.module.name = targetName.toStdString();
        moduleEvent.module.path = targetName.toStdString();
        moduleEvent.module.symbolFilePath = targetName.toStdString();
        session->send(moduleEvent);
        eventOutput.clear();
    }

    // *running,thread-id=all
    if (sOut.contains("*running")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();
        QString threadId;
        auto outList = QList(eventOutput.split(","));
        for (auto &out : outList) {
            if (out.contains("thread-id")) {
                threadId = out.split("=").last().remove("\"").remove("\n");
                continue;
            }
        }

        dap::OutputEvent outputEvent;
        outputEvent.output = threadId.toStdString();
        session->send(outputEvent);
        eventOutput.clear();
    }

    if (sOut.contains("=breakpoint-modified")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();
        QString bkpt;
        auto outList = QList(eventOutput.split(","));
        for (auto &out : outList) {
            if (out.contains("bkpt")) {
                bkpt = out.split("=").last().remove("{").remove("}");
                qInfo() << bkpt;
                continue;
            }
        }
        QString bkptNumber;
        QString bkptFunc;
        QString bkptFile;
        QString bkptLine;
        auto bkptList = QList(bkpt.split(","));
        for (auto &out : bkptList) {
            if (out.contains("number")) {
                bkptNumber = out.split("=").last().remove("\"").remove("\n");
                continue;
            }
            if (out.contains("func")) {
                bkptFunc = out.split("=").last().remove("\"").remove("\n");
                continue;
            }
            if (out.contains("file")) {
                bkptFile = out.split("=").last().remove("\"").remove("\n");
                continue;
            }
            if (out.contains("line")) {
                bkptLine = out.split("=").last().remove("\"").remove("\n");
                continue;
            }
        }
        dap::OutputEvent outputEvent;
        outputEvent.category = "stdout";
        //outputEvent.output = {"Breakpoint %1, %2 at %3:%4\n"}.arg(bkptNumber, bkptFunc, bkptFile, bkptLine);
        session->send(outputEvent);
        eventOutput.clear();
    }

    if (sOut.contains("*stopped")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();
        QString reason;
        QString disp;
        QString bkptno;
        QString frame;
        QString threadId;
        QString stoppedThreads;
        auto outList = QList(eventOutput.split(","));
        for (auto &out : outList) {
            if (out.contains("reason")) {
                reason = out.split("=").last().remove("\"");
                continue;
            }
            if (reason == "breakpoint hit") {
                if (out.contains("disp")) {
                    disp = out.split("=").last().remove("\"");
                }

                if (out.contains("bkptno")) {
                    bkptno = out.split("=").last().remove("\"");
                }

                if (out.contains("frame")) {
                    frame = out.split("=").last().remove("{").remove("}");
                }

                if (out.contains("thread-id")) {
                    threadId = out.split("=").last().remove("\"");
                }

                if (out.contains("stopped-threads")) {
                    stoppedThreads = out.split("=").last().remove("\"");
                }

                if (out.contains("core")) {
                    auto cpuCore = out.split("=").last().remove("\"").remove("\n");
                }
            }
        }

        if (reason == "breakpoint hit") {
            dap::StoppedEvent stoppedEvent;
            stoppedEvent.reason = reason.toStdString();
            stoppedEvent.description = "breakpoint";
            stoppedEvent.threadId = threadId.toInt();
            stoppedEvent.text = "breakpoint";
            if (stoppedThreads == "all") {
                stoppedEvent.allThreadsStopped = true;
            }
            dap::integer hitBreakpointId = bkptno.toInt();
            //stoppedEvent.hitBreakpointIds.value();
            session->send(stoppedEvent);
            eventOutput.clear();
        }
    }

    if (sOut.contains("~\"[Inferior ")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();

        dap::OutputEvent outputEvent;
        outputEvent.output = eventOutput.remove("~\"").remove("\"").remove("\n").toStdString();
        session->send(outputEvent);
        eventOutput.clear();
    }

    if (sOut.contains("=thread-exited")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();
        QString threadId;
        QString threadGroupId;
        auto outList = QList(eventOutput.split(","));
        for (auto &out : outList) {
            auto kv = out.remove("\"").remove("\n");
            if (kv.contains("id")) {
                threadId = kv.split("=").last();
            }
            if (kv.contains("group-id")) {
                threadGroupId = kv.split("=").last();
            }
        }

        dap::OutputEvent outputEvent;
        outputEvent.category = "console";
        outputEvent.output = threadId.toStdString();
        session->send(outputEvent);
        eventOutput.clear();
    }

    if (sOut.contains("=thread-group-exited")) {
        eventOutput.clear();
        eventOutput = sOut.split(":").last();
        QString exitCode;
        QString threadGroupId;
        auto outList = QList(eventOutput.split(","));
        for (auto &out : outList) {
            auto kv = out.remove("\"").remove("\n");
            if (kv.contains("exit-code")) {
                exitCode = kv.split("=").last();
            }
        }

        dap::OutputEvent outputEvent;
        outputEvent.output = exitCode.toStdString();
        session->send(outputEvent);
        eventOutput.clear();
    }
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
