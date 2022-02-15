/**
 * C/C++ Debug Adaptor Protocol Server drived by google cppdap library
 *
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xiaozaihu<xiaozaihu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             xiaozaihu<xiaozaihu@uniontech.com>
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

#include "dap/protocol.h"
#include "dap/session.h"
#include "dap/network.h"
#include "gdbproxy.h"
#include "debugmanager.h"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTimer>
#include <QThread>
#include <QDebug>

#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <unordered_set>

constexpr int kPort = 4711;
static bool isDebuggeIsStartWithLaunchRequest = false;
static bool isDebuggeIsStartWithAttachRequest = false;
static bool isSupportsTerminateDebuggee = true;
bool isGdbProcessStarted;
bool isGDbProcessTerminated;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("CxxDbg");
    QCoreApplication::setApplicationVersion("1.0");

    // instance a debug manager object
    auto debugger = DebugManager::instance();

    // signals send from debugger manager(gdb) to our debug adapter server.
    //
    // try to receving gdbProcessStarted signals from debugger.
//    QObject::connect(debugger, &DebugManager::gdbProcessStarted, [=](){
//        qInfo() << "gdb process started";
//    });

//    // try to receving streamDebugInternal signals from debugger
    QObject::connect(debugger, &DebugManager::streamDebugInternal, [=](const QString sOut){
        qInfo() << sOut;
    });

//    // try to receving asyncRunning signal from debugger.
//    QObject::connect(debugger, &DebugManager::asyncRunning, [=](const QString threadid) {
//        qInfo() << threadid;
//    });

//    // try to receving asyncStopped signal from debugger.
//    QObject::connect(debugger, &DebugManager::asyncStopped, [=](const gdb::AsyncContext ctx){
//        qInfo() << "Stopped reason";
//    });

//    // try to receving breakpointInserted signal from debugger.
//    QObject::connect(debugger, &DebugManager::breakpointInserted, [](){
//        qInfo() << "breakpoint inserted";
//    });

//    // try to receving breakpoint modified signal from debugger.
//    QObject::connect(debugger, &DebugManager::breakpointModified, [](){
//        qInfo() << "breakpoint modified";
//    });

//    // try to receving breakpoint removed signal from debugger.
//    QObject::connect(debugger, &DebugManager::breakpointRemoved, [](){
//        qInfo() << "breakpoint removed";
//    });

    // Those Callback handlers for
    //  a socket connection to the server
    auto onClientConnected =
            [&](const std::shared_ptr<dap::ReaderWriter>& socket) {
        auto session = dap::Session::create();
        session->bind(socket);
        qint64 pid; // gdb process pid
        // The Initialize request is the first message sent from the client and
        // the response reports debugger capabilities.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Initialize
        session->registerHandler([&](const dap::InitializeRequest& request) {
            dap::InitializeResponse response{};
            response.supportsConfigurationDoneRequest = true;
            response.supportsFunctionBreakpoints = true;
            //response.supportsInstructionBreakpoints = true;
            response.supportsRestartRequest = true;
            response.supportsDataBreakpoints = true;
            response.supportsTerminateRequest = true;
            response.supportTerminateDebuggee = true;

            // client is UnionCode
            //clientIsUnionCode = (request.clientID.value("") == "UnionCode");
            printf("Server received initialize request from client\n");
            return response;
        });

        // When the Initialize response has been sent,
        //   we need to send the initialized event.
        //
        // We use the registerSentHandler() to ensure the event is sent *after* the
        // initialize response.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Events_Initialized
        session->registerSentHandler(
                    [&](const dap::ResponseOrError<dap::InitializeResponse>&) {
            printf("Server sent initialized event to client\n");
            session->send(dap::InitializedEvent());
        });

        // Signal used to configurate the server session when ConfigurationDoneReqeust
        bool configurated = false;
        std::condition_variable configurated_cv;
        std::mutex configurated_mutex;
        // The ConfigurationDone request is made by the client once all configuration
        // requests have been made.
        // This example debugger uses this request to 'start' the debugger.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_ConfigurationDone
        session->registerHandler([&](const dap::ConfigurationDoneRequest&) {
            std::unique_lock<std::mutex> lock(configurated_mutex);
            configurated = true;
            configurated_cv.notify_one();
            auto response = dap::ConfigurationDoneResponse();
            printf("Server received configurationDone request from client\n");
            return response;
        });


        // The Launch request is made when the client instructs the debugger adapter
        // to start the debuggee. This request contains the launch arguments.
        //
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Launch
        //
        // must launch gdb process in the main thread.
        // default, lauch request will stop at entry(main)
        session->registerHandler([=](const dap::LaunchRequest&) {
            isDebuggeIsStartWithLaunchRequest = true;
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigStart, debugger, &DebugManager::execute);
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert);
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigLaunchLocal, debugger, &DebugManager::launchLocal);
            QObject::connect(debugger, &DebugManager::gdbProcessStarted, GDBProxy::instance(), &GDBProxy::gdbProcessStarted);
            QObject::connect(debugger, &DebugManager::breakpointModified, GDBProxy::instance(), &GDBProxy::breakpointModified);
            QObject::connect(debugger, &DebugManager::libraryLoaded, GDBProxy::instance(), &GDBProxy::librayLoaded);
            QObject::connect(debugger, &DebugManager::asyncRunning, GDBProxy::instance(), &GDBProxy::asyncRunning);
            debugger->setGdbCommand("/usr/bin/gdb");
            debugger->setGdbArgs({"/home/zhxiao/workspaces/qtcreator/demo/main"});
            emit GDBProxy::instance()->sigStart();
            GDBProxy::instance()->gdbProcessStarted();
            if (!isGdbProcessStarted) {
                qInfo() << "gdb process start failed!\n";
                qApp->exit(1);
            }
            emit GDBProxy::instance()->sigBreakInsert("main");
            GDBProxy::instance()->breakpointModified();
            emit GDBProxy::instance()->sigLaunchLocal();
            GDBProxy::instance()->librayLoaded();
            GDBProxy::instance()->asyncRunning();
            dap::LaunchResponse response{};
            printf("Server received launch request from client\n");
            return response;
        });

        // The Attach request
        session->registerHandler([=](const dap::AttachRequest&) {
            isDebuggeIsStartWithAttachRequest = true;
            dap::AttachResponse response;
//            qint64 pid = 28343; // from attach request
//            QObject::connect(GDBProxy::instance(), &GDBProxy::sigAttach, debugger, &DebugManager::command);
//            QString cmd("-target-attach ");
//            cmd.append(QString::number(pid));
//            emit GDBProxy::instance()->sigAttach(cmd);

            printf("Server received attach reqeust from client\n");
            return response;
        });

        // The Restart request
        // Restarts a debug session. Clients should only call this request if the capability
        // ‘supportsRestartRequest’ is true
        session->registerHandler([=](const dap::RestartRequest&) {
            isDebuggeIsStartWithLaunchRequest = true;
            dap::RestartResponse response;
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigQuit, debugger, &DebugManager::quit);
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigStart, debugger, &DebugManager::execute);
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert);
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigLaunchLocal, debugger, &DebugManager::launchLocal);
            // send quit signal to debugger
            emit GDBProxy::instance()->sigQuit();

            // send start signal to debugger
            debugger->setGdbCommand("/usr/bin/gdb");
            debugger->setGdbArgs({"/home/zhxiao/workspaces/qtcreator/demo/main"});
            emit GDBProxy::instance()->sigStart();
            emit GDBProxy::instance()->sigBreakInsert("main");
            emit GDBProxy::instance()->sigLaunchLocal();

            printf("Server received restart request from client\n");
            return response;
        });

        // The Terminate requestis sent from the client to the debug adapter
        //in order to give the debuggee a chance for terminating itself.
        // map to Debug Plugin Menu: Abort Debugging
        session->registerHandler([=](const dap::TerminateRequest&) {
            dap::TerminateResponse response;
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigQuit, debugger, &DebugManager::quit);
            emit GDBProxy::instance()->sigQuit();

            printf("Server received terminate request from client\n");
            return response;
        });

        // The Pause request instructs the debugger to pause execution of one or all
        // threads.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Pause
        session->registerHandler([&](const dap::PauseRequest&) {
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigPause, debugger, &DebugManager::command);
            emit GDBProxy::instance()->sigPause("-continue-pause");

            QObject::connect(debugger, &DebugManager::asyncStopped, GDBProxy::instance(), &GDBProxy::asyncStopped);
            GDBProxy::instance()->asyncStopped();
            printf("Server received pause request from client\n");
            return dap::PauseResponse();
        });

        // The Continue request instructs the debugger to resume execution of one or
        // all threads.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Continue
        session->registerHandler([&](const dap::ContinueRequest&) {
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigContinue, debugger, &DebugManager::commandContinue);
            emit GDBProxy::instance()->sigContinue();

            printf("Server received continue request from client\n");
            return dap::ContinueResponse();
        });

        // The Next request instructs the debugger to single line step for a specific
        // thread.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Next
        session->registerHandler([&](const dap::NextRequest&) {
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigNext, debugger, &DebugManager::commandNext);
            emit GDBProxy::instance()->sigNext();

            dap::StoppedEvent event;
            event.reason = "stepover";
            session->send(event);
            printf("Server received next request from client\n");
            return dap::NextResponse();
        });

        // The StepIn request instructs the debugger to step-in for a specific thread.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepIn
        session->registerHandler([&](const dap::StepInRequest&) {
            // Step-in treated as step-over as there's only one stack frame.
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigStepin, debugger, &DebugManager::commandStep);
            emit GDBProxy::instance()->sigStepin();

            QObject::connect(debugger, &DebugManager::asyncStopped, GDBProxy::instance(), &GDBProxy::asyncStopped);
            GDBProxy::instance()->asyncStopped();

            dap::StoppedEvent event;
            event.reason = "stepin";
            session->send(event);
            printf("Server received stepin request from client\n");
            return dap::StepInResponse();
        });

        // The StepOut request instructs the debugger to step-out for a specific
        // thread.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StepOut
        session->registerHandler([&](const dap::StepOutRequest&) {
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigStepout, debugger, &DebugManager::commandFinish);
            emit GDBProxy::instance()->sigStepout();

            QObject::connect(debugger, &DebugManager::asyncStopped, GDBProxy::instance(), &GDBProxy::asyncStopped);
            GDBProxy::instance()->asyncStopped();

            dap::StoppedEvent event;
            event.reason = "stepout";
            session->send(event);
            printf("Server received stepout request from client\n");
            return dap::StepOutResponse();
        });

        // The BreakpointLocations request returns all possible locations for source breakpoints in a range
        session->registerHandler([&](const dap::BreakpointLocationsRequest& ) {
            dap::BreakpointLocationsResponse response;
            return response;
        });

        // The SetBreakpoints request instructs the debugger to clear and set a number
        // of line breakpoints for a specific source file.
        //
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_SetBreakpoints
        session->registerHandler([&](const dap::SetBreakpointsRequest& request) {
            dap::SetBreakpointsResponse response;
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert);
            emit GDBProxy::instance()->sigBreakInsert(QString("/home/zhxiao/workspaces/qtcreator/demo/main.cpp:main"));
            QObject::connect(debugger, &DebugManager::breakpointModified, GDBProxy::instance(), &GDBProxy::breakpointModified);
            GDBProxy::instance()->breakpointModified();
            printf("Server received setBreakpoints request from client\n");
            return response;
        });

        // The SetExceptionBreakpoints request instructs the debugger to set a exception breakpoints
        session->registerHandler([&](const dap::SetExceptionBreakpointsRequest& request) {
            dap::SetExceptionBreakpointsResponse response;
            //QObject::connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert);
            //emit GDBProxy::instance()->sigBreakInsert(QString("/home/zhxiao/workspaces/qtcreator/demo/main.cpp:main"));

            printf("Server received setExceptionBreakpoints request from client\n");
            return response;
        });

        // The SetFunctionBreakpointBreakpoints request instructs the debugger to set a function breakpoints
        session->registerHandler([&](const dap::SetFunctionBreakpointsRequest& request) {
            dap::SetFunctionBreakpointsResponse response;
            QObject::connect(GDBProxy::instance(), &GDBProxy::sigBreakInsert, debugger, &DebugManager::breakInsert);
            emit GDBProxy::instance()->sigBreakInsert(QString("/home/zhxiao/workspaces/qtcreator/demo/main.cpp:swap"));

            printf("Server received setFunctionBreakpoints request from client\n");
            return response;
        });

        // The SetDataBreakpoints request instructs the debugger to set a data breakpoints
        session->registerHandler([&](const dap::SetDataBreakpointsRequest& request) {
            dap::SetDataBreakpointsResponse response;

            printf("Server received SetDataBreakpoints request from client\n");
            return response;
        });

        // The Threads request queries the debugger's list of active threads.
        //
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Threads
        session->registerHandler([&](const dap::ThreadsRequest&) {
              dap::ThreadsResponse response;
              dap::Thread thread;
              QObject::connect(GDBProxy::instance(), &GDBProxy::sigThreads, debugger, &DebugManager::command);
              QString cmd("-thread-list-ids");
              emit GDBProxy::instance()->sigThreads(cmd);

              printf("Server recevied Thread request from client\n");
              return response;
        });

        // The request returns a stacktrace from the current execution state of a given thread.
        //
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_StackTrace
        //
        // the given thread id was get from StackTrace request object.
        //
        session->registerHandler(
            [&](const dap::StackTraceRequest& request)
                -> dap::ResponseOrError<dap::StackTraceResponse> {
               auto threadid = request.threadId.operator long();
               dap::StackTraceResponse response;
               QObject::connect(GDBProxy::instance(), &GDBProxy::sigSelectThread, debugger, &DebugManager::command);
               QObject::connect(GDBProxy::instance(), &GDBProxy::sigStackTrace, debugger, &DebugManager::command);

               // select a given thread id
               // -thread-select 3
               QString selectThread("-thread-select ");
               selectThread.append(QString::number(threadid));
               emit GDBProxy::instance()->sigSelectThread(selectThread);

               // request all stack frames by omitting
               // the startFrame and levels arguments.
               QString cmd("-stack-list-frames");
               emit GDBProxy::instance()->sigStackTrace(cmd);

               printf("Server received StackTrace request from the client\n");
               return response;
        });

        // The Scopes(all variables in selected thread and frame) request reports all the scopes of the given stack frame.
        //
        // frameId was given by Scope request
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Scopes
        session->registerHandler([&](const dap::ScopesRequest& request)
                                     -> dap::ResponseOrError<dap::ScopesResponse> {
              dap::ScopesResponse response{};
              auto frameId = request.frameId.operator long();
              QObject::connect(GDBProxy::instance(), &GDBProxy::sigScopes, debugger, &DebugManager::command);

              // -stack-list-variables --thread threadId --frame frameId --all-values
              QString scopes(QString("-stack-list-variables --thread %1 --frame %2 --all-values"));
              emit GDBProxy::instance()->sigScopes(scopes);

              printf("Server received Scopes request from the client\n");
              return response;
        });

        // Retrieves all child variables for the given variable reference(the given scope).
        //
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Variables
        session->registerHandler([&](const dap::VariablesRequest& request)
                                     -> dap::ResponseOrError<dap::VariablesResponse> {
              dap::VariablesResponse response;
              QObject::connect(GDBProxy::instance(), &GDBProxy::sigVariables, debugger, &DebugManager::command);
              QString variables{"-stack-list-locals"};
              emit GDBProxy::instance()->sigVariables(variables);

              printf("Server received Variables request from the client\n");
              return response;
        });

        // The Source request retrieves the source code for a given source file.
        //
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Source
        // gdb list command???
        session->registerHandler([&](const dap::SourceRequest& request)
                                     -> dap::ResponseOrError<dap::SourceResponse> {
              dap::SourceResponse response;
              //auto sourcePath = request.source->path.value().c_str();
              QObject::connect(GDBProxy::instance(), &GDBProxy::sigSource, debugger, &DebugManager::command);
              QString source{"-file-list-exec-source-files"};
              emit GDBProxy::instance()->sigSource(source);

              printf("Server received Source request from the client\n");
              return response;
        });

        // Signal used to terminate the server session when a DisconnectRequest
        // is made by the client.
        bool terminate = false;
        std::condition_variable terminate_cv;
        // guards 'terminate'
        std::mutex terminate_mutex;
        // The Disconnect request is made by the client before it disconnects
        // from the server.
        // https://microsoft.github.io/debug-adapter-protocol/specification#Requests_Disconnect
        // map to Debug Plugin Menu: Deattach
        session->registerHandler([&](const dap::DisconnectRequest&) {
                // Client wants to disconnect. Set terminate to true, and signal the
                // condition variable to unblock the server thread.
                std::unique_lock<std::mutex> lock(terminate_mutex);
                terminate = true;
                terminate_cv.notify_one();

                if (isSupportsTerminateDebuggee) {
                    if (isDebuggeIsStartWithLaunchRequest) {
                        QObject::connect(GDBProxy::instance(), &GDBProxy::sigQuit, debugger, &DebugManager::quit);
                        emit GDBProxy::instance()->sigQuit();
                    } else if (isDebuggeIsStartWithAttachRequest) {
//                        QObject::connect(GDBProxy::instance(), &GDBProxy::sigDetach, debugger, &DebugManager::command);
//                        QString cmd("-target-detach ");
//                        cmd.append(QString::number(pid));
//                        emit GDBProxy::instance()->sigDetach(cmd);
                    }
                }
                printf("Server received disconnnect request from client\n");
                return dap::DisconnectResponse{};
        });

        // Wait for the client to disconnect (or reach a 5 second timeout)
        // before releasing the session and disconnecting the socket to the
        // client.
        std::unique_lock<std::mutex> lock(terminate_mutex);
        //terminate_cv.wait_for(lock, std::chrono::seconds(5), [&] { return terminate; });
        terminate_cv.wait(lock, [&] {return terminate; });
        printf("Server closing connection\n");
    };

    // Error handler
    auto onError = [&](const char* msg) {
        printf("Server error: %s\n", msg);
    };

    // Create the network server
    auto server = dap::net::Server::create();
    // Start listening on kPort.
    // onClientConnected will be called when a client wants to connect.
    // onError will be called on any connection errors.
    server->start(kPort, onClientConnected, onError);

    return a.exec();
}

