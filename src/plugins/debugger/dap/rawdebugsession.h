// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAWDEBUGSESSION_H
#define RAWDEBUGSESSION_H

#include "dap/session.h"
#include "dap/protocol.h"
#include "dap/future.h"

#include <QObject>

#include <memory>
#include <any>

namespace dap {

/**
 * @brief RawDebugSession is implemented by orignal
 * DAP protocol,used by top level class DapDebugger.
 */
class RawDebugSession : public QObject
{
    Q_OBJECT
public:
    template<typename T>
    using promiseEx = future<ResponseOrError<typename T::Response>>;

    using ErrorHandler = std::function<void(const std::string&)>;

    // value type returned.
    #define Promise RawDebugSession::promiseEx

    explicit RawDebugSession(std::shared_ptr<Session>&, QObject *parent = nullptr);

    bool initialize();

    void start();
    Promise<InitializeRequest> initialize(const InitializeRequest &request);
    bool disconnect(const DisconnectRequest &request);

    Promise<LaunchRequest> launch(const LaunchRequest &request);
    Promise<AttachRequest> attach(const AttachRequest &request);

    bool terminate(bool restart);
    bool restart(const RestartRequest &request);
    Promise<NextRequest> next(const NextRequest &request);
    Promise<StepInRequest> stepIn(const StepInRequest &request);
    Promise<StepOutRequest> stepOut(const StepOutRequest &request);
    Promise<ContinueRequest> continueDbg(const ContinueRequest &request);
    Promise<PauseRequest> pause(const PauseRequest &args);
    Promise<TerminateThreadsRequest> terminateThreads(const TerminateThreadsRequest &request);
    Promise<SetVariableRequest> setVariable(const SetVariableRequest &request);
    Promise<SetExpressionRequest> setExpression(const SetExpressionRequest &request);
    Promise<RestartFrameRequest> restartFrame(const RestartFrameRequest &request);
    Promise<StepInTargetsRequest> stepInTargets(const StepInTargetsRequest &request);
    Promise<CompletionsRequest> completions(const CompletionsRequest &request);
    Promise<SetBreakpointsRequest> setBreakpoints(const SetBreakpointsRequest &request);
    Promise<SetFunctionBreakpointsRequest> setFunctionBreakpoints(const SetFunctionBreakpointsRequest &request);
    Promise<DataBreakpointInfoRequest> dataBreakpointInfo(const DataBreakpointInfoRequest &request);
    Promise<SetDataBreakpointsRequest> setDataBreakpoints(const SetDataBreakpointsRequest &request);
    Promise<SetExceptionBreakpointsRequest> setExceptionBreakpoints(const SetExceptionBreakpointsRequest &request);
    Promise<BreakpointLocationsRequest> breakpointLocations(const BreakpointLocationsRequest &request);
    Promise<ConfigurationDoneRequest> configurationDone();
    Promise<StackTraceRequest> stackTrace(const StackTraceRequest &request);
    Promise<ExceptionInfoRequest> exceptionInfo(const ExceptionInfoRequest &request);
    Promise<ScopesRequest> scopes(const ScopesRequest &request);
    Promise<VariablesRequest> variables(const VariablesRequest &request);
    Promise<SourceRequest> source(const SourceRequest &request);
    Promise<LoadedSourcesRequest> loadedSources(const LoadedSourcesRequest &request);
    Promise<ThreadsRequest> threads();
    Promise<EvaluateRequest> evaluate(const EvaluateRequest &request);
    Promise<StepBackRequest> stepBack(const StepBackRequest &request);
    Promise<ReverseContinueRequest> reverseContinue(const ReverseContinueRequest &request);
    Promise<GotoTargetsRequest> gotoTargets(const GotoTargetsRequest &request);
    Promise<GotoRequest> goto_(const GotoRequest &request);
    Promise<SetInstructionBreakpointsRequest> setInstructionBreakpoints(const SetInstructionBreakpointsRequest &request);
    Promise<DisassembleRequest> disassemble(const DisassembleRequest &request);
    Promise<CancelRequest> cancel(const CancelRequest &request);

    const dap::Capabilities &capabilities() const;
    bool shutdown(optional<boolean> terminateDebuggee, optional<boolean> restart = false);

    bool readyForBreakpoints() const;
    void setReadyForBreakpoints(bool bReady);
signals:

public slots:

private:    
    void registerHandlers();
    void mergeCapabilities(const InitializeResponse &capabilities);

    // Send sends the request to the debugger, waits for the request to complete,
    // and then assigns the response to |res|.
    // Returns true on success, false on error.
    template <typename REQUEST, typename RESPONSE>
    bool send(const REQUEST& request, RESPONSE* res);

    // Send sends the request to the debugger, and waits for the request to
    // complete.
    // Returns true on success, false on error.
    template <typename REQUEST>
    bool syncSend(const REQUEST& request);

    // Send sends the request to the debugger and return future result.
    template <typename REQUEST>
    Promise<REQUEST> send(const REQUEST &request);

    void onError(const std::string& error);

    /**
     * private parameters.
     */
    ErrorHandler errHandler;

    std::shared_ptr<Session> session;

    bool allThreadsContinued = true;
    bool _readyForBreakpoints = false;
    Capabilities _capabilities;

    // shutdown
    bool inShutdown = false;
    bool terminated = false;
};

} // end dap namespace.

#endif // RAWDEBUGSESSION_H
