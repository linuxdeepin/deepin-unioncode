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
#include "interface/variable.h"

#include <QSharedPointer>
#include <QPointer>
#include <QWidget>

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
    bool launch(dap::LaunchRequest &config) override;
    bool attach(dap::AttachRequest &config) override;

    void restart() override;
    void terminate(bool restart = false) override;
    void disconnect(bool terminateDebuggee = true, bool restart = false) override;

    void sendBreakpoints(const QString &sourcePath, dap::array<IBreakpoint> &breakpointsToSend) override;
    void sendFunctionBreakpoints(dap::array<IFunctionBreakpoint> &fbpts) override;
    void sendExceptionBreakpoints(dap::array<IExceptionBreakpoint> &exbpts) override;
    dap::optional<dap::DataBreakpointInfoResponse> dataBreakpointInfo(
            dap::string &name, dap::optional<dapNumber> variablesReference) override;
    void sendDataBreakpoints(dap::array<IDataBreakpoint> dataBreakpoints) override;
    void sendInstructionBreakpoints(dap::array<IInstructionBreakpoint> instructionBreakpoints) override;
    //    dap::array<IPosition> breakpointsLocations(URI uri, number lineNumber);
    dap::optional<dap::Breakpoint> getDebugProtocolBreakpoint(dap::string &breakpointId) override;
    //    dap::optional<dap::Response> customRequest(dap::string &request, dap::any args);
    dap::optional<dap::StackTraceResponse> stackTrace(dapNumber threadId, dapNumber startFrame, dapNumber levels) override;
    dap::optional<IExceptionInfo> exceptionInfo(dapNumber threadId) override;
    dap::optional<dap::ScopesResponse> scopes(dapNumber frameId, dapNumber threadId) override;
    dap::optional<dap::VariablesResponse> variables(dapNumber variablesReference,
                                                    dap::optional<dapNumber> threadId,
                                                    dap::optional<dap::string> filter,
                                                    dap::optional<dapNumber> start,
                                                    dap::optional<dapNumber> count) override;
    dap::optional<dap::EvaluateResponse> evaluate(
            dap::string &expression, dapNumber frameId, dap::optional<dap::string> context) override;
    void restartFrame(dapNumber frameId, dapNumber threadId) override;
    void setLastSteppingGranularity(dapNumber threadId, dap::optional<dap::SteppingGranularity> granularity) override;

    void next(dap::integer threadId, dap::optional<dap::SteppingGranularity> granularity) override;
    void stepIn(dap::integer threadId, dap::optional<dap::integer> targetId, dap::optional<dap::SteppingGranularity> granularity) override;
    void stepOut(dap::integer threadId, dap::optional<dap::SteppingGranularity> granularity) override;
    void stepBack(dapNumber threadId, dap::optional<dap::SteppingGranularity> granularity) override;
    void continueDbg(dap::integer threadId) override;
    void reverseContinue(dapNumber threadId) override;
    void pause(dap::integer threadId) override;
    void terminateThreads(dap::array<dapNumber> &threadIds) override;
    dap::optional<dap::SetVariableResponse> setVariable(
            dapNumber variablesReference, dap::string &name, dap::string &value) override;
    dap::optional<dap::SetExpressionResponse> setExpression(
            dapNumber frameId, dap::string &expression, dap::string &value) override;
    dap::optional<dap::GotoTargetsResponse> gotoTargets(
            dap::Source &source, dapNumber line, dapNumber column) override;
    dap::optional<dap::GotoResponse> goto_(dapNumber threadId, dapNumber targetId) override;
    dap::optional<dap::StepInTargetsResponse> stepInTargets(dapNumber frameId) override;
    dap::optional<dap::CancelResponse> cancel(dap::string &progressId) override;
    // threads.
    dap::optional<Thread *> getThread(dapNumber threadId) override;
    dap::optional<dap::array<IThread *>> getAllThreads() const override;
    void clearThreads(bool removeThreads, dap::optional<dapNumber> reference) override;
    dap::array<IRawStoppedDetails *> &getStoppedDetails() override;
    void rawUpdate(IRawModelUpdate *data) override;
    void fetchThreads(dap::optional<IRawStoppedDetails> stoppedDetails) override;
    dap::optional<dap::Source> getSourceForUri(QUrl &uri) override;
    Source *getSource(dap::optional<dap::Source> raw) override;

    dap::string getId() override;
    dap::integer getThreadId() override;
    dap::string getLabel() const override;
    void setName(dap::string &name) override;

    bool getLocals(dap::integer frameId, IVariables *out) override;

    dap::Session *getDapSession() const;
    dap::RawDebugSession *getRawSession() const;

    dap::array<dap::Thread> fetchThreads(IRawStoppedDetails *stoppedDetails);

    bool launchJavaDap(const QString &workDir,
                       const QString &mainClass,
                       const QString &projectName,
                       const QStringList &classPaths,
                       const QString &javaExec);
    bool attachPythonDap(int port,
                         const QString &workspace);
    void closeSession();
signals:
    void sigRegisterHandlers();

public slots:

private:
    void shutdown();
    void onBreakpointHit(const dap::StoppedEvent &event);
    void onStep(const dap::StoppedEvent &event);

    dap::Source getRawSource(QUrl &uri);
    void cancelAllRequests();

    bool getVariables(dap::integer variablesRef, IVariables *out, dap::integer depth = 0);

    QSharedPointer<dap::RawDebugSession> raw;
    QSharedPointer<RunTimeCfgProvider> rtCfgProvider;

    bool initialized = false;

    std::shared_ptr<dap::Session> session;

    std::string id;

    dap::integer threadId = 0;
    std::map<dapNumber, Thread *> threads;
    dap::array<dapNumber> threadIds;

    DebugModel *model = nullptr;

    dap::optional<dap::string> name;

    dap::array<IRawStoppedDetails *> stoppedDetails;

    std::map<dap::string, Source *> sources;

    QPointer<QWidget> alertBox;
};
}

#endif   // DEBUGSESSION_H
