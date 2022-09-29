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
#include "rawdebugsession.h"
#include "dap/session.h"
#include "dap/protocol.h"
#include "objects.hpp"

#include <QDebug>

namespace dap {

/**
 * @brief rawDebugSession implimented by original dap
 * protocol.TODO(mozart):Decide whether to use synchronous
 * or asynchronous.
 * @param parent
 */
RawDebugSession::RawDebugSession(std::shared_ptr<Session> &_session, QObject *parent)
    : QObject(parent)
    , session(_session)
{
    initialize();
}

bool RawDebugSession::initialize()
{
    errHandler = [&](const std::string& err){
        onError(err);
    };
    return true;
}

Promise<InitializeRequest> RawDebugSession::initialize(const InitializeRequest &args)
{
    auto response = send(args);
    auto capabilities = response.get().response;
    mergeCapabilities(capabilities);
    return response;
}

bool RawDebugSession::disconnect(const DisconnectRequest &args)
{
    optional<boolean> terminateDebuggee;
    if (capabilities().supportTerminateDebuggee)
        terminateDebuggee = args.terminateDebuggee;
    return shutdown(terminateDebuggee, args.restart);
}

void RawDebugSession::mergeCapabilities(const InitializeResponse &capabilities)
{
    _capabilities = objects::mixin(_capabilities, capabilities);
}

void RawDebugSession::onError(const std::string& error)
{
    qInfo() << "Error :" << error.data();
}

void RawDebugSession::start()
{
    // All the handlers we care about have now been registered.
    // We now bind the session to stdin and stdout to connect to the client.
    // After the call to bind() we should start receiving requests, starting with
    // the Initialize request.
//    std::shared_ptr<dap::Reader> in = dap::file(stdin, false);
//    std::shared_ptr<dap::Writer> out = dap::file(stdout, false);

//    // connect to server.
//    session->connect(in, out);
}

Promise<LaunchRequest> RawDebugSession::launch(const LaunchRequest &request)
{
    auto response = send(request);
    return response;
}

Promise<LaunchJavaRequest> RawDebugSession::launch(const LaunchJavaRequest &request)
{
    auto response = send(request);
    return response;
}

Promise<AttachRequest> RawDebugSession::attach(const AttachRequest &request)
{
    auto response = send(request);
    return response;
}

bool RawDebugSession::terminate(bool restart)
{
    Q_UNUSED(restart)
    if (capabilities().supportsTerminateRequest) {
        TerminateRequest request;
        auto response = send(request);
        response.wait();
        return true;
    }
    qInfo() << "terminated not supported";
    return false;
}

bool RawDebugSession::restart(const RestartRequest &request)
{
    if (_capabilities.supportsRestartRequest) {
        send(request);
        return true;
    }
    qInfo() << "restart not supported";
    return false;
}

Promise<NextRequest> RawDebugSession::next(const NextRequest &request)
{
    auto response = send(request);
    response.wait();
    // fire notify event.
    return response;
}

Promise<StepInRequest> RawDebugSession::stepIn(const StepInRequest &request)
{
    auto response = send(request);
    response.wait();
    // fire notify event.
    return response;
}

Promise<StepOutRequest> RawDebugSession::stepOut(const StepOutRequest &request)
{
    auto response = send(request);
    response.wait();
    // fire notify event.
    return response;
}

Promise<ContinueRequest> RawDebugSession::continueDbg(const ContinueRequest &request)
{
    auto response = send(request);
    response.wait();
    auto responseBody = response.get().response;
    if (responseBody.allThreadsContinued.has_value()) {
        allThreadsContinued = responseBody.allThreadsContinued.value();
    }
    // fire event.

    return response;
}

Promise<PauseRequest> RawDebugSession::pause(const PauseRequest &request)
{
    auto response = send(request);
    return response;
}

Promise<TerminateThreadsRequest> RawDebugSession::terminateThreads(const TerminateThreadsRequest &request)
{
    if (_capabilities.supportsTerminateThreadsRequest) {
        return send(request);
    }
    qInfo() << "terminateThreads not supported";
    return {};
}

Promise<SetVariableRequest> RawDebugSession::setVariable(const SetVariableRequest &request)
{
    if (_capabilities.supportsSetVariable) {
        return send(request);
    }
    qInfo() << "supportsSetVariable not supported";
    return {};
}

Promise<SetExpressionRequest> RawDebugSession::setExpression(const SetExpressionRequest &request)
{
    if (_capabilities.supportsSetExpression) {
        return send(request);
    }
    qInfo() << "supportsSetExpression not supported";
    return {};
}

Promise<RestartFrameRequest> RawDebugSession::restartFrame(const RestartFrameRequest &request)
{
    if (_capabilities.supportsRestartFrame) {
        auto response = send(request);
        response.wait();
        return response;
    }
    qInfo() << "supportsRestartFrame not supported";
    return {};
}

Promise<StepInTargetsRequest> RawDebugSession::stepInTargets(const StepInTargetsRequest &request)
{
    if (_capabilities.supportsStepInTargetsRequest) {
        return send(request);
    }
    qInfo() << "supportsStepInTargetsRequest not supported";
    return {};
}

Promise<CompletionsRequest> RawDebugSession::completions(const CompletionsRequest &request)
{
    if (_capabilities.supportsCompletionsRequest) {
        return send(request);
    }
    qInfo() << "supportsCompletionsRequest not supported";
    return {};
}

Promise<SetBreakpointsRequest> RawDebugSession::setBreakpoints(const SetBreakpointsRequest &request)
{
    return send(request);
}

Promise<SetFunctionBreakpointsRequest> RawDebugSession::setFunctionBreakpoints(
        const SetFunctionBreakpointsRequest &request)
{
    if (_capabilities.supportsFunctionBreakpoints) {
        return send(request);
    }
    qInfo() << "supportsFunctionBreakpoints not supported";
    return {};
}

Promise<DataBreakpointInfoRequest> RawDebugSession::dataBreakpointInfo(const DataBreakpointInfoRequest &request)
{
    if (_capabilities.supportsDataBreakpoints) {
        return send(request);
    }
    qInfo() << "supportsDataBreakpoints not supported";
    return {};
}

Promise<SetDataBreakpointsRequest> RawDebugSession::setDataBreakpoints(const SetDataBreakpointsRequest &request)
{
    if (_capabilities.supportsDataBreakpoints) {
        return send(request);
    }
    qInfo() << "supportsDataBreakpoints not supported";
    return {};
}

Promise<SetExceptionBreakpointsRequest> RawDebugSession::setExceptionBreakpoints(
        const SetExceptionBreakpointsRequest &request)
{
    return send(request);
}

Promise<BreakpointLocationsRequest> RawDebugSession::breakpointLocations(const BreakpointLocationsRequest &request)
{
    if (_capabilities.supportsBreakpointLocationsRequest) {
        return send(request);
    }
    qInfo() << "supportsBreakpointLocationsRequest not supported";
    return {};
}

Promise<ConfigurationDoneRequest> RawDebugSession::configurationDone()
{
    if (_capabilities.supportsConfigurationDoneRequest)
        return send(ConfigurationDoneRequest());
    qInfo() << "supportsConfigurationDoneRequest not supported";
    return {};
}

Promise<StackTraceRequest> RawDebugSession::stackTrace(const StackTraceRequest &request)
{
    return send(request);
}

Promise<ExceptionInfoRequest> RawDebugSession::exceptionInfo(const ExceptionInfoRequest &request)
{
    if (_capabilities.supportsExceptionInfoRequest) {
        return send(request);
    }
    qInfo() << "supportsExceptionInfoRequest not supported";
    return {};
}

Promise<ScopesRequest> RawDebugSession::scopes(const ScopesRequest &request)
{
    return send(request);
}

Promise<VariablesRequest> RawDebugSession::variables(const VariablesRequest &request)
{
    return send(request);
}

Promise<SourceRequest> RawDebugSession::source(const SourceRequest &request)
{
    return send(request);
}

Promise<LoadedSourcesRequest> RawDebugSession::loadedSources(const LoadedSourcesRequest &request)
{
    if (_capabilities.supportsLoadedSourcesRequest) {
        return send(request);
    }
    qInfo() << "supportsLoadedSourcesRequest not supported";
    return {};
}

Promise<ThreadsRequest> RawDebugSession::threads()
{
    ThreadsRequest request;
    return send(request);
}

Promise<EvaluateRequest> RawDebugSession::evaluate(const EvaluateRequest &request)
{
    return send(request);
}

Promise<StepBackRequest> RawDebugSession::stepBack(const StepBackRequest &request)
{
    if (_capabilities.supportsStepBack) {
        auto response = send(request);
        response.wait();
        // fire event.
        return response;
    }
    qInfo() << "supportsStepBack not supported";
    return {};
}

Promise<ReverseContinueRequest> RawDebugSession::reverseContinue(const ReverseContinueRequest &request)
{
    if (_capabilities.supportsStepBack) {
        auto response = send(request);
        response.wait();
        // fire event.
        return response;
    }
    qInfo() << "supportsStepBack not supported";
    return {};
}

Promise<GotoTargetsRequest> RawDebugSession::gotoTargets(const GotoTargetsRequest &request)
{
    if (_capabilities.supportsGotoTargetsRequest) {
        return send(request);
    }
    qInfo() << "supportsGotoTargetsRequest not supported";
    return {};
}

Promise<GotoRequest> RawDebugSession::goto_(const GotoRequest &request)
{
    if (_capabilities.supportsGotoTargetsRequest) {
        auto response = send(request);
        response.wait();
        // fire event.
        return response;
    }
    qInfo() << "supportsGotoTargetsRequest not supported";
    return {};
}

Promise<SetInstructionBreakpointsRequest> RawDebugSession::setInstructionBreakpoints(const SetInstructionBreakpointsRequest &request)
{
    if (_capabilities.supportsInstructionBreakpoints) {
        auto response = send(request);
        response.wait();
        // fire event.
        return response;
    }
    qInfo() << "supportsInstructionBreakpoints not supported";
    return {};
}

Promise<DisassembleRequest> RawDebugSession::disassemble(const DisassembleRequest &request)
{
    if (_capabilities.supportsDisassembleRequest) {
        auto response = send(request);
        response.wait();
        // fire event.
        return response;
    }
    qInfo() << "supportsDisassembleRequest not supported";
    return {};
}

Promise<CancelRequest> RawDebugSession::cancel(const CancelRequest &request)
{
    return send(request);
}

const Capabilities &RawDebugSession::capabilities() const
{
    return _capabilities;
}

bool RawDebugSession::shutdown(optional<boolean> terminateDebuggee, optional<boolean> restart)
{
    if (!inShutdown) {
        inShutdown = true;
        DisconnectRequest request;
        request.restart = restart;
        request.terminateDebuggee = terminateDebuggee;
        syncSend(request);
    }
    return true;
}

bool RawDebugSession::readyForBreakpoints() const
{
    return _readyForBreakpoints;
}

void RawDebugSession::setReadyForBreakpoints(bool bReady)
{
    _readyForBreakpoints = bReady;
}

void RawDebugSession::registerHandlers()
{
    /*
     *  Register events.
     */

    // The event indicates that one or more capabilities have changed.
    session->registerHandler([&](const CapabilitiesEvent &event){
        _capabilities = objects::mixin(_capabilities, event.capabilities);
        qInfo() << "\n--> recv : " << "CapabilitiesEvent";
    });

    // The event indicates that the execution of the debuggee has continued.
    session->registerHandler([&](const ContinuedEvent &event){
        allThreadsContinued = event.allThreadsContinued.value();
        Q_UNUSED(event)
        qInfo() << "\n--> recv : " << "ContinuedEvent";
    });
}

template<typename REQUEST, typename RESPONSE>
bool RawDebugSession::send(const REQUEST &request, RESPONSE *res)
{
    auto r = session->send(request).get();
    if (r.error) {
        errHandler(r.error.message);
        return false;
    }
    *res = r.response;
    return true;
}

template<typename REQUEST>
bool RawDebugSession::syncSend(const REQUEST &request)
{
    using RESPONSE = typename REQUEST::Response;
    RESPONSE response;
    return send(request, &response);
}

template<typename REQUEST>
Promise<REQUEST> RawDebugSession::send(const REQUEST &request)
{
    return session->send(request);
}
} // end namespace.
