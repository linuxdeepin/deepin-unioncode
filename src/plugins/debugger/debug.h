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
#ifndef DEBUG_H
#define DEBUG_H

#include "dap/protocol.h"
#include "debuggerglobals.h"
#include "interface/variable.h"

#include <QString>
#include <QUrl>
#include <QUuid>

#include <map>

namespace DEBUG_NAMESPACE {

#define undefined \
    {             \
    }
#define dapNumber dap::integer
#define ReadonlyArray dap::array

struct IDebugSession;
struct IStackFrame;
struct IExpression;
struct Thread;
struct Source;

struct IRawStoppedDetails
{
    dap::optional<dap::string> reason;
    dap::optional<dap::string> description;
    dap::optional<dapNumber> threadId;
    dap::optional<dap::string> text;
    dap::optional<dapNumber> totalFrames;
    dap::optional<bool> allThreadsStopped;
    dap::optional<dap::string> framesErrorMessage;
    dap::optional<dap::array<dapNumber>> hitBreakpointIds;
};

struct IRawModelUpdate
{
    dap::string sessionId;
    dap::array<dap::Thread> threads;
    dap::optional<IRawStoppedDetails> stoppedDetails;
};

struct IBreakpointData
{
    dap::optional<dap::string> id;
    dap::optional<dapNumber> lineNumber;
    dap::optional<dapNumber> column;
    bool enabled = true;
    dap::optional<dap::string> condition;
    dap::optional<dap::string> logMessage;
    dap::optional<dap::string> hitCondition;
};

struct IBreakpointUpdateData
{
    dap::optional<dap::string> condition;
    dap::optional<dap::string> hitCondition;
    dap::optional<dap::string> logMessage;
    dap::optional<dapNumber> lineNumber;
    dap::optional<dapNumber> column;
};

struct ITreeElement
{
    virtual dap::string getId() { return ""; }
};

struct IEnablement : public ITreeElement
{
    bool enabled = true;
};

struct IBaseBreakpoint : public IEnablement
{
    dap::optional<dap::string> condition;
    dap::optional<dap::string> hitCondition;
    dap::optional<dap::string> logMessage;
    bool verified = false;
    bool support = false;   // suported.
    dap::optional<dap::string> message;
    dap::array<dap::string> sessionsThatVerified;
    dap::optional<dapNumber> getIdFromAdapter(dap::string sessionId);
};

struct IInnerBreakpoint
{
    QUrl uri;
    dapNumber lineNumber = 0;
    dap::optional<dapNumber> endLineNumber;
    dap::optional<dapNumber> column;
    dap::optional<dapNumber> endColumn;
    dap::any adapterData;
};

struct IBreakpoint : public IBaseBreakpoint, public IInnerBreakpoint
{
};

struct IFunctionBreakpoint : public IBaseBreakpoint
{
    dap::string name;
};

struct IExceptionBreakpoint : public IBaseBreakpoint
{
    dap::string filter;
    dap::string label;
    dap::string description;
};

struct IDataBreakpoint : IBaseBreakpoint
{
    dap::string description;
    dap::string dataId;
    bool canPersist = false;
    dap::DataBreakpointAccessType accessType;
};

struct IInstructionBreakpoint : public IBaseBreakpoint
{
    // instructionReference is the instruction 'address' from the debugger.
    dap::string instructionReference;
    dapNumber offset = 0;
};

struct IExceptionInfo
{
    dap::optional<dap::string> id;
    dap::optional<dap::string> description;
    dap::string breakMode;
    dap::optional<dap::ExceptionDetails> details;
};

/**
  * Base structs.
  */

struct Enablement : public IEnablement
{
    Enablement(bool _enabled, dap::string &_id)
        : id(_id)
    {
        enabled = _enabled;
    }

    dap::string getId() override
    {
        return id;
    }

private:
    dap::string id;
};

struct IBreakpointSessionData : public dap::Breakpoint
{
    bool supportsConditionalBreakpoints;
    bool supportsHitConditionalBreakpoints;
    bool supportsLogPoints;
    bool supportsFunctionBreakpoints;
    bool supportsDataBreakpoints;
    bool supportsInstructionBreakpoints;
    dap::string sessionId;
};

struct BaseBreakpoint : public IBaseBreakpoint
{
    BaseBreakpoint(
            bool _enabled,
            dap::optional<dap::string> _hitCondition,
            dap::optional<dap::string> _condition,
            dap::optional<dap::string> _logMessage,
            const dap::string &_id)
        : id(_id)
    {
        enabled = _enabled;
        hitCondition = _hitCondition;
        condition = _condition;
        logMessage = _logMessage;
    }

    virtual ~BaseBreakpoint() {}

    void setSessionData(dap::string &sessionId, dap::optional<IBreakpointSessionData> data)
    {
        if (!data) {
            auto it = sessionData.begin();
            for (; it != sessionData.end(); ++it) {
                if (sessionId == it->first) {
                    it = sessionData.erase(it);
                }
            }
        } else {
            data.value().sessionId = sessionId;
            sessionData.insert(std::pair<dap::string, IBreakpointSessionData>(sessionId, data.value()));
        }

        // TODO(mozart)
    }

    dap::optional<dap::string> message()
    {
        if (!data) {
            return undefined;
        }
        return data.value().message;
    }

    bool verified()
    {
        if (data) {
            return data.value().verified;
        }
        return true;
    }

    dap::array<dap::string> sessionsThatVerified()
    {
        dap::array<dap::string> sessionIds;

        auto it = sessionData.begin();
        for (; it != sessionData.end(); ++it) {
            if (it->second.verified) {
                sessionIds.push_back(it->first);
            }
        }
        return sessionIds;
    }

    dap::optional<dapNumber> getIdFromAdapter(dap::string &sessionId)
    {
        dap::optional<IBreakpointSessionData> data = getData(sessionId);
        if (data) {
            return data.value().id;
        }
        return undefined;
    }

    dap::optional<IBreakpointSessionData> getData(dap::string &sessionId)
    {
        auto it = sessionData.begin();
        bool bFound = false;
        for (; it != sessionData.end(); ++it) {
            if (it->first == sessionId) {
                data = it->second;
                bFound = true;
                break;
            }
        }
        if (bFound) {
            return data;
        }
        return undefined;
    }

    dap::optional<dap::Breakpoint> getDebugProtocolBreakpoint(dap::string &sessionId)
    {
        dap::optional<dap::Breakpoint> bp;
        auto data = getData(sessionId);
        if (data) {
            bp.value().id = data->id;
            bp.value().verified = data->verified;
            bp.value().message = data->message;
            bp.value().source = data->source;
            bp.value().line = data->line;
            bp.value().column = data->column;
            bp.value().endLine = data->endLine;
            bp.value().endColumn = data->endColumn;
            bp.value().instructionReference = data->instructionReference;
            bp.value().offset = data->offset;
        }
        return bp;
    }

protected:
    dap::optional<IBreakpointSessionData> data;

    std::map<dap::string, IBreakpointSessionData> sessionData;
    dap::string id;
};

struct Breakpoint : public BaseBreakpoint, public IInnerBreakpoint
{
    Breakpoint(
            QUrl &uri,
            dapNumber lineNumber,
            dap::optional<dapNumber> column,
            bool enabled,
            dap::optional<dap::string> condition,
            dap::optional<dap::string> hitCondition,
            dap::optional<dap::string> logMessage,
            dap::any adapterData,
            const std::string &id = QUuid::createUuid().toString().toStdString())
        : BaseBreakpoint(enabled, hitCondition, condition, logMessage, id), _uri(uri), _lineNumber(lineNumber), _column(column), _adapterData(adapterData)
    {
    }

    bool isDirty(QUrl uri)
    {
        Q_UNUSED(uri)
        // Not support dirty check now.
        return false;
    }

    dapNumber lineNumber()
    {
        if (verified() && data && data.value().line) {
            return data.value().line.value();
        }
        return _lineNumber;
    }

    bool verified()
    {
        if (data) {
            return data.value().verified && !isDirty(_uri);
        }
        return true;
    }

    QUrl getUriFromSource(dap::Source source, dap::optional<dap::string> path, dap::string sessionId)
    {
        Q_UNUSED(path)
        Q_UNUSED(sessionId)
        return QUrl(source.path->c_str());
    }

    QUrl uri()
    {
        if (verified() && data && data.value().source) {
            return getUriFromSource(data.value().source.value(), data.value().source.value().path, data->sessionId);
        }
        return _uri;
    }

    dap::optional<dap::integer> column()
    {
        if (verified() && data && data.value().column) {
            return data.value().column;
        }
        return _column;
    }

    dap::optional<dap::string> message()
    {
        if (isDirty(uri())) {
            return "Unverified breakpoint. File is modified, please restart debug session.";
        }
        return BaseBreakpoint::message();
    }

    dap::any adapterData()
    {
        if (data && data.value().source && data.value().source.value().adapterData) {
            return data.value().source.value().adapterData.value();
        }
        return _adapterData;
    }

    dap::optional<dapNumber> endLineNumber()
    {
        if (verified() && data) {
            return data.value().endLine;
        }
        return undefined;
    }

    dap::optional<dapNumber> endColumn()
    {
        if (verified() && data) {
            return data.value().endColumn;
        }
        return undefined;
    }

    bool supported()
    {
        if (!data) {
            return true;
        }
        if (logMessage && !data.value().supportsLogPoints) {
            return false;
        }
        if (condition && !data.value().supportsConditionalBreakpoints) {
            return false;
        }
        if (hitCondition && !data.value().supportsHitConditionalBreakpoints) {
            return false;
        }

        return true;
    }

    void setSessionData(dap::string &sessionId, dap::optional<IBreakpointSessionData> data)
    {
        BaseBreakpoint::setSessionData(sessionId, data);
        if (_adapterData.is<std::nullptr_t>()) {
            _adapterData = adapterData();
        }
    }

    dap::string toString()
    {
        return undefined;
    }

    void update(IBreakpointUpdateData &data)
    {
        if (!data.lineNumber) {
            _lineNumber = data.lineNumber.value();
        }
        if (!data.column) {
            _column = data.column;
        }
        if (!data.condition) {
            condition = data.condition;
        }
        if (!data.hitCondition) {
            hitCondition = data.hitCondition;
        }
        if (!data.logMessage) {
            logMessage = data.logMessage;
        }
    }

private:
    QUrl _uri;
    dapNumber _lineNumber = 0;
    dap::optional<dapNumber> _column;
    dap::any _adapterData;
};

struct IThread : public ITreeElement
{
public:
    /**
     * Process the thread belongs to
     */
    IDebugSession *session = nullptr;

    /**
     * Id of the thread generated by the debug adapter backend.
     */
    dapNumber threadId = 0;

    /**
     * Name of the thread.
     */
    dap::string name;

    /**
     * Information about the current thread stop event. Undefined if thread is not stopped.
     */
    dap::optional<IRawStoppedDetails> stoppedDetails;

    /**
     * Information about the exception if an 'exception' stopped event raised and DA supports the 'exceptionInfo' request, otherwise undefined.
     */
    dap::optional<IExceptionInfo> exceptionInfo;

    dap::string stateLabel;

    /**
     * Gets the callstack if it has already been received from the debug
     * adapter.
     */
    ReadonlyArray<IStackFrame *> getCallStack();

    /**
     * Gets the top stack frame that is not hidden if the callstack has already been received from the debug adapter
     */
    dap::optional<IStackFrame *> getTopStackFrame();

    /**
     * Invalidates the callstack cache
     */
    void clearCallStack();

    /**
     * Indicates whether this thread is stopped. The callstack for stopped
     * threads can be retrieved from the debug adapter.
     */
    bool stopped = false;

    dap::any next(dap::SteppingGranularity granularity);
    dap::any stepIn(dap::optional<dap::SteppingGranularity> granularity);
    dap::any stepOut(dap::SteppingGranularity granularity);
    dap::any stepBack(dap::SteppingGranularity granularity);
    dap::any continue_();
    dap::any pause();
    dap::any terminate();
    dap::any reverseContinue();
};

struct IExpressionContainer : public ITreeElement
{
    bool hasChildren = false;
    dap::array<IExpression *> getChildren();
    dap::optional<dapNumber> reference;
    dap::string value;
    dap::string type;
    dap::optional<bool> valueChanged;
};

struct IExpression : public IExpressionContainer
{
    dap::string name;
};

struct IRange
{
    /**
     * Line number on which the range starts (starts at 1).
     */
    dapNumber startLineNumber = 0;
    /**
     * Column on which the range starts in line `startLineNumber` (starts at 1).
     */
    dapNumber startColumn = 0;
    /**
     * Line number on which the range ends.
     */
    dapNumber endLineNumber = 0;
    /**
     * Column on which the range ends in line `endLineNumber`.
     */
    dapNumber endColumn = 0;
};

struct IScope : public IExpressionContainer
{
    dap::string name;
    bool expensive = false;
    dap::optional<IRange> range;
};

struct IStackFrame : public ITreeElement
{
    virtual ~IStackFrame() {}
    IThread *thread = nullptr;
    dap::string name;
    dap::optional<dap::string> presentationHint;
    dapNumber frameId = 0;
    IRange range;
    Source *source = nullptr;
    bool canRestart;
    dap::optional<dap::string> instructionPointerReference;
    dap::array<IScope> getScopes();
    ReadonlyArray<IScope> getMostSpecificScopes(IRange range);
    void forgetScopes();
    dap::any restart();
    dap::string toString();
    //    openInEditor(editorService: IEditorService, preserveFocus?: boolean, sideBySide?: boolean, pinned?: boolean): Promise<IEditorPane | undefined>;
    bool equals(IStackFrame other);
};

enum State {
    kInactive,
    kInitializing,
    kStopped,
    kRunning
};

struct IEnvConfig
{
    // 'neverOpen' | 'openOnSessionStart' | 'openOnFirstSessionStart'
    dap::optional<dap::string> internalConsoleOptions;
    dap::optional<dap::string> preRestartTask;
    dap::optional<dap::string> postRestartTask;
    dap::optional<dap::string> preLaunchTaskr;
    dap::optional<dap::string> postDebugTask;
    dap::optional<dapNumber> debugServer;
    dap::optional<bool> noDebug;
};

struct IConfigPresentation
{
    dap::optional<bool> hidden;
    dap::optional<dap::string> group;
    dap::optional<dapNumber> order;
};

struct IConfig : public IEnvConfig
{
    // fundamental attributes
    dap::string type;
    dap::string request;
    dap::string name;
    dap::optional<IConfigPresentation *> presentation;

    // internals
    dap::optional<dap::string> __sessionId;
    dap::optional<dap::any> __restart;
    dap::optional<bool> __autoAttach;
    dap::optional<dapNumber> port;
};

struct IDebugSession : public ITreeElement
{
    virtual ~IDebugSession() {}
    virtual const dap::Capabilities &capabilities() const = 0;

    virtual bool initialize(const char *ip, int port, dap::InitializeRequest &iniRequest) = 0;

    virtual bool launch(const QString &targetPath, bool noDebug = false) = 0;
    virtual bool attach(dap::AttachRequest &config) = 0;

    virtual void restart() = 0;
    virtual void terminate(bool restart = false) = 0;
    virtual void disconnect(bool terminateDebuggee = true, bool restart = false) = 0;

    virtual void sendBreakpoints(const QString &sourcePath, dap::array<IBreakpoint> &breakpointsToSend) = 0;
    virtual void sendFunctionBreakpoints(dap::array<IFunctionBreakpoint> &fbpts) = 0;
    virtual void sendExceptionBreakpoints(dap::array<IExceptionBreakpoint> &exbpts) = 0;
    virtual dap::optional<dap::DataBreakpointInfoResponse> dataBreakpointInfo(
            dap::string &name, dap::optional<dapNumber> variablesReference) = 0;
    virtual void sendDataBreakpoints(dap::array<IDataBreakpoint> dataBreakpoints) = 0;
    virtual void sendInstructionBreakpoints(dap::array<IInstructionBreakpoint> instructionBreakpoints) = 0;
    //    dap::array<IPosition> breakpointsLocations(URI uri, number lineNumber);
    virtual dap::optional<dap::Breakpoint> getDebugProtocolBreakpoint(dap::string &breakpointId) = 0;
    //    dap::optional<dap::Response> customRequest(dap::string &request, dap::any args);
    virtual dap::optional<dap::StackTraceResponse> stackTrace(dapNumber threadId, dapNumber startFrame, dapNumber levels) = 0;
    virtual dap::optional<IExceptionInfo> exceptionInfo(dapNumber threadId) = 0;
    virtual dap::optional<dap::ScopesResponse> scopes(dapNumber frameId, dapNumber threadId) = 0;
    virtual dap::optional<dap::VariablesResponse> variables(dapNumber variablesReference,
                                                            dap::optional<dapNumber> threadId,
                                                            dap::optional<dap::string> filter,
                                                            dap::optional<dapNumber> start,
                                                            dap::optional<dapNumber> count) = 0;
    virtual dap::optional<dap::EvaluateResponse> evaluate(
            dap::string &expression, dapNumber frameId, dap::optional<dap::string> context) = 0;
    virtual void restartFrame(dapNumber frameId, dapNumber threadId) = 0;
    virtual void setLastSteppingGranularity(dapNumber threadId, dap::optional<dap::SteppingGranularity> granularity) = 0;

    virtual void next(dap::integer threadId, dap::optional<dap::SteppingGranularity> granularity) = 0;
    virtual void stepIn(dap::integer threadId, dap::optional<dap::integer> targetId,
                        dap::optional<dap::SteppingGranularity> granularity) = 0;
    virtual void stepOut(dap::integer threadId, dap::optional<dap::SteppingGranularity> granularity) = 0;
    virtual void stepBack(dapNumber threadId, dap::optional<dap::SteppingGranularity> granularity) = 0;
    virtual void continueDbg(dap::integer threadId) = 0;
    virtual void reverseContinue(dapNumber threadId) = 0;
    virtual void pause(dap::integer threadId) = 0;
    virtual void terminateThreads(dap::array<dapNumber> &threadIds) = 0;
    virtual dap::optional<dap::SetVariableResponse> setVariable(
            dapNumber variablesReference, dap::string &name, dap::string &value) = 0;
    virtual dap::optional<dap::SetExpressionResponse> setExpression(
            dapNumber frameId, dap::string &expression, dap::string &value) = 0;
    virtual dap::optional<dap::GotoTargetsResponse> gotoTargets(dap::Source &source, dapNumber line, dapNumber column) = 0;
    virtual dap::optional<dap::GotoResponse> goto_(dapNumber threadId, dapNumber targetId) = 0;
    //    dap::optional<dap::SourceResponse> loadSource(QUrl &resource);
    //    dap::array<dap::Source> getLoadedSources();
    //    dap::optional<dap::CompletionsResponse> completions(
    //            dap::optional<number> frameId,
    //            dap::optional<number> threadId,
    //            dap::string &text,
    //            dap::Position &position,
    //            number overwriteBefore);
    virtual dap::optional<dap::StepInTargetsResponse> stepInTargets(dapNumber frameId) = 0;
    virtual dap::optional<dap::CancelResponse> cancel(dap::string &progressId) = 0;
    //    dap::optional<dap::array<dap::DisassembledInstruction>> disassemble(dap::string &memoryReference, number offset, number instructionOffset, number instructionCount);
    //    dap::optional<dap::ReadMemoryResponse> readMemory(dap::string &memoryReference, number offset, number count);
    //    dap::optional<dap::WriteMemoryResponse> writeMemory(dap::string &memoryReference, number offset, dap::string &data, dap::optional<bool> allowPartial);
    // threads.
    virtual dap::optional<Thread *> getThread(dapNumber threadId) = 0;
    virtual dap::optional<dap::array<IThread *>> getAllThreads() const = 0;
    virtual void rawUpdate(IRawModelUpdate *data) = 0;
    virtual void clearThreads(bool removeThreads, dap::optional<dapNumber> reference) = 0;
    virtual dap::array<IRawStoppedDetails *> &getStoppedDetails() = 0;
    virtual void fetchThreads(dap::optional<IRawStoppedDetails> stoppedDetails) = 0;
    virtual dap::optional<dap::Source> getSourceForUri(QUrl &uri) = 0;
    virtual Source *getSource(dap::optional<dap::Source> raw) = 0;
    virtual dap::string getLabel() const = 0;

    virtual dap::integer getThreadId() = 0;
    virtual void setName(dap::string &name) = 0;

    virtual bool getLocals(dap::integer frameId, IVariables *out) = 0;

    State state = kInactive;
    IConfig *configuration = nullptr;
};

struct ExpressionContainer : public IExpressionContainer
{
};

struct Range : public IRange
{
    Range(dapNumber _startLineNumber, dapNumber _startColumn, dapNumber _endLineNumber, dapNumber _endColumn)
    {
        startLineNumber = _startLineNumber;
        startColumn = _startColumn;
        endLineNumber = _endLineNumber;
        endColumn = _endColumn;
    }
};

struct Source
{
    Source(dap::optional<dap::Source> raw_, dap::string &sessionId)
    {
        dap::string path;
        if (raw_) {
            raw = raw_.value();
            if (raw.path)
                path = raw.path.value();
            else if (raw.name) {
                path = raw.name.value();
            } else {
                path = "";
            }
            available = true;
        } else {
            raw.name = "Unknown Source";
            available = false;
            path = "debug:Unknown Source";
        }
        uri = getUriFromSource(raw, path, sessionId);
    }

    dap::optional<dap::string> name() const
    {
        return raw.name;
    }

    dap::optional<dap::string> origin() const
    {
        return raw.origin;
    }

    dap::optional<dap::string> path() const
    {
        return raw.path;
    }

    dap::optional<dap::string> presentationHint() const
    {
        return raw.presentationHint;
    }

    dap::optional<dap::integer> reference() const
    {
        return raw.sourceReference;
    }

    bool inMemory() const
    {
        return uri.scheme() == "debug";
    }

    QUrl getUriFromSource(dap::Source &raw, dap::optional<dap::string> path, dap::string &sessionId)
    {
        if (raw.sourceReference && raw.sourceReference.value() > 0) {
            QUrl url;
            url.setPath(path->c_str());
            url.setScheme("debug");
            QString query = QString("session=%s&ref=%d").arg(sessionId.c_str()).arg(raw.sourceReference.value());
            url.setQuery(query);
            return url;
        }
        return undefined;
    }

    QUrl uri;
    bool available = false;
    dap::Source raw;
};

// Thread
struct Thread : public IThread
{
    dap::array<dap::StackFrame> callStack;
    dap::array<dap::StackFrame> staleCallStack;
    //    dap::optional<IRawStoppedDetails> stoppedDetails;
    //    bool stopped = false;
    bool reachedEndOfCallStack = false;
    dap::optional<dap::SteppingGranularity> lastSteppingGranularity;
    //    dap::string name;

    Thread(IDebugSession *_session, dap::string _name, dapNumber _threadId)
    {
        name = _name;
        threadId = _threadId;
        session = _session;
        stopped = false;
    }

    virtual ~Thread()
    {
    }

    dap::string getId() override
    {
        return session->getId();
    }

    void clearCallStack()
    {
        if (callStack.size()) {
            staleCallStack = callStack;
        }
        callStack.clear();
    }

    dap::array<dap::StackFrame> getCallStack()
    {
        return callStack;
    }

    ReadonlyArray<dap::StackFrame> getStaleCallStack()
    {
        return staleCallStack;
    }

    dap::optional<IStackFrame> getTopStackFrame()
    {
        // TODO(mozart)
        return undefined;
    }

    dap::string stateLabel()
    {
        if (stoppedDetails) {
            return stoppedDetails.value().description.value();
        }

        return undefined;
    }

    /**
     * Queries the debug adapter for the callstack and returns a promise
     * which completes once the call stack has been retrieved.
     * If the thread is not stopped, it returns a promise to an empty array.
     * Only fetches the first stack frame for performance reasons. Calling this method consecutive times
     * gets the remainder of the call stack.
     */
    void fetchCallStack(dapNumber levels = 20)
    {
//        if (stopped) {
        callStack.clear();
            auto start = callStack.size();
            auto callStack = getCallStackImpl(static_cast<int>(start), levels);
            reachedEndOfCallStack = static_cast<int>(callStack.size()) < levels;
            if (start < this->callStack.size()) {
                size_t endIndex = callStack.size() - start;
                this->callStack.erase(callStack.begin() + static_cast<int>(start), callStack.begin() + static_cast<int>(endIndex));
            }
            this->callStack.insert(this->callStack.end(), callStack.begin(), callStack.end());
//            if (stoppedDetails.value().totalFrames && stoppedDetails.value().totalFrames.value() == static_cast<int>(callStack.size())) {
//                reachedEndOfCallStack = true;
//            }
//        }
    }

private:
    dap::array<dap::StackFrame> getCallStackImpl(dapNumber startFrame, dapNumber levels)
    {
        auto response = session->stackTrace(threadId, startFrame, levels);
        if (!response) {
            return undefined;
        }

        if (stoppedDetails && response && response->totalFrames) {
            stoppedDetails.value().totalFrames = response.value().totalFrames.value();
        }

        auto stackFrames = response.value().stackFrames;
        return stackFrames;
    }

    /**
     * Returns exception info promise if the exception was thrown, otherwise undefined
     */
#if 0   // TODO(mozart):not used.
    dap::optional<IExceptionInfo> exceptionInfo()
    {
        // TODO(mozart)
        return undefined;
    }
#endif

    void next(dap::SteppingGranularity granularity)
    {
        session->next(threadId, granularity);
    }

    void stepIn(dap::SteppingGranularity granularity)
    {
        session->stepIn(threadId, undefined, granularity);
    }

    void stepOut(dap::SteppingGranularity granularity)
    {
        session->stepOut(threadId, granularity);
    }

    void stepBack(dap::SteppingGranularity granularity)
    {
        session->stepBack(threadId, granularity);
    }
#if 0   // TODO(mozart):not used.
    void continue_()
    {
        session->continueDbg(threadId);
    }
#endif

    void pause()
    {
        session->pause(threadId);
    }
#if 0   // TODO(mozart):not used.
    void terminate()
    {
        dap::array<number> threadIds;
        threadIds.push_back(threadId);
        session->terminateThreads(threadIds);
    }
#endif

    void reverseContinue()
    {
        session->reverseContinue(threadId);
    }
};

}   // end namespace.

#endif   // DEBUG_H
