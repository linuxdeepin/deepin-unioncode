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
#ifndef DEBUGSESSION_H
#define DEBUGSESSION_H

#include "rawdebugsession.h"
#include "dap/session.h"
#include "dap/protocol.h"
#include "debug.h"

#include <QObject>
#include <QSharedPointer>

#include <memory>

namespace dap {
class RawDebugSession;
}

class RunTimeCfgProvider;

namespace DEBUG_NAMESPACE {
class DebugModel;
class DebugSession : public QObject, public IDebugSession
{
    Q_OBJECT
public:
    DebugSession(DebugModel *, QObject *parent = nullptr);
    ~DebugSession() override;

    const dap::Capabilities &capabilities() const override;

    bool initialize(const char *ip, int port, dap::InitializeRequest &iniRequest) override;

    bool launch(const char *config, bool noDebug = false) override;
    bool attach(dap::AttachRequest &config) override;

    void restart() override;
    void terminate(bool restart = false) override;
    void disconnect(bool terminateDebuggee = true, bool restart = false) override;

    void sendBreakpoints(dap::array<IBreakpoint> &breakpointsToSend) override;
    void sendFunctionBreakpoints(dap::array<IFunctionBreakpoint> &fbpts) override;
    void sendExceptionBreakpoints(dap::array<IExceptionBreakpoint> &exbpts) override;
    dap::optional<dap::DataBreakpointInfoResponse> dataBreakpointInfo(
            dap::string &name, dap::optional<number> variablesReference) override;
    void sendDataBreakpoints(dap::array<IDataBreakpoint> dataBreakpoints) override;
    void sendInstructionBreakpoints(dap::array<IInstructionBreakpoint> instructionBreakpoints) override;
//    dap::array<IPosition> breakpointsLocations(URI uri, number lineNumber);
    dap::optional<dap::Breakpoint> getDebugProtocolBreakpoint(dap::string &breakpointId) override;
//    dap::optional<dap::Response> customRequest(dap::string &request, dap::any args);
    dap::optional<dap::StackTraceResponse> stackTrace(number threadId, number startFrame, number levels) override;
    dap::optional<IExceptionInfo> exceptionInfo(number threadId) override;
    dap::optional<dap::ScopesResponse> scopes(number frameId, number threadId) override;
    dap::optional<dap::VariablesResponse> variables(number variablesReference,
                                                    dap::optional<number> threadId,
                                                    dap::optional<dap::string> filter,
                                                    dap::optional<number> start,
                                                    dap::optional<number> count) override;
    dap::optional<dap::EvaluateResponse> evaluate(
            dap::string &expression, number frameId, dap::optional<dap::string> context) override;
    void restartFrame(number frameId, number threadId) override;
    void setLastSteppingGranularity(number threadId, dap::optional<dap::SteppingGranularity> granularity) override;

    void next(dap::integer threadId, dap::optional<dap::SteppingGranularity> granularity) override;
    void stepIn(dap::integer threadId, dap::optional<dap::integer> targetId, dap::optional<dap::SteppingGranularity> granularity) override;
    void stepOut(dap::integer threadId, dap::optional<dap::SteppingGranularity> granularity) override;
    void stepBack(number threadId, dap::optional<dap::SteppingGranularity> granularity) override;
    void continueDbg(dap::integer threadId) override;
    void reverseContinue(number threadId) override;
    void pause(dap::integer threadId) override;
    void terminateThreads(dap::array<number> &threadIds) override;
    dap::optional<dap::SetVariableResponse> setVariable(
            number variablesReference, dap::string &name, dap::string &value) override;
    dap::optional<dap::SetExpressionResponse> setExpression(
            number frameId, dap::string &expression, dap::string &value) override;
    dap::optional<dap::GotoTargetsResponse> gotoTargets(
            dap::Source &source, number line, number column) override;
    dap::optional<dap::GotoResponse> goto_(number threadId, number targetId) override;
//    dap::optional<dap::SourceResponse> loadSource(QUrl &resource);
//    dap::array<dap::Source> getLoadedSources();
//    dap::optional<dap::CompletionsResponse> completions(
//            dap::optional<number> frameId,
//            dap::optional<number> threadId,
//            dap::string &text,
//            dap::Position &position,
//            number overwriteBefore);
    dap::optional<dap::StepInTargetsResponse> stepInTargets(number frameId) override;
    dap::optional<dap::CancelResponse> cancel(dap::string &progressId) override;
//    dap::optional<dap::array<dap::DisassembledInstruction>> disassemble(dap::string &memoryReference, number offset, number instructionOffset, number instructionCount);
//    dap::optional<dap::ReadMemoryResponse> readMemory(dap::string &memoryReference, number offset, number count);
//    dap::optional<dap::WriteMemoryResponse> writeMemory(dap::string &memoryReference, number offset, dap::string &data, dap::optional<bool> allowPartial);
    // threads.
    dap::optional<Thread *> getThread(number threadId) override;
    dap::optional<dap::array<IThread *> > getAllThreads() const override;
    void clearThreads(bool removeThreads, dap::optional<number> reference) override;
    dap::optional<IRawStoppedDetails> getStoppedDetails() const override;
    void rawUpdate(IRawModelUpdate *data) override;
    void fetchThreads(dap::optional<IRawStoppedDetails> stoppedDetails) override;
    dap::optional<dap::Source> getSourceForUri(QUrl &uri) override;
//    dap::Source getSource(dap::optional<dap::Source> raw) override;
    Source *getSource(dap::optional<dap::Source> raw) override;

    dap::string getId() override;
    dap::integer getThreadId() override;
    dap::string getLabel() const override;
    void setName(dap::string &name) override;
signals:

public slots:

private:
    void shutdown();
    void registerHandlers();
    void fetchThreads(IRawStoppedDetails *stoppedDetails);
    void onBreakpointHit(const dap::StoppedEvent &event);
    void onStep(const dap::StoppedEvent &event);

    dap::Source getRawSource(QUrl &uri);
    void cancelAllRequests();


    QSharedPointer<dap::RawDebugSession> raw;
    QSharedPointer<RunTimeCfgProvider> rtCfgProvider;

    bool initialized = false;

    std::shared_ptr<dap::Session> session;

    std::string id;

    dap::integer threadId = 0;
    std::map<number, Thread *> threads;
    dap::array<number> threadIds;

    DebugModel *model = nullptr;

    dap::optional<dap::string> name;

    dap::array<IRawStoppedDetails *> stoppedDetails;

    std::map<dap::string, Source *> sources;
};
}

#endif   // DEBUGSESSION_H
