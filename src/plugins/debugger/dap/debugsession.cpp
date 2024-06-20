// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debugsession.h"

#include "rawdebugsession.h"
#include "runtimecfgprovider.h"
#include "debugservice.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "debugmodel.h"
#include "stackframe.h"
#include "interface/variable.h"

#include "dap/io.h"
#include "dap/protocol.h"
#include "dap/session.h"
#include "dap/network.h"

#include <QDebug>
#include <QUuid>

#include <chrono>
#include <thread>
#include <algorithm>

namespace DEBUG_NAMESPACE {

static constexpr const char *kLocals = "locals";

using namespace dap;
DebugSession::DebugSession(DebugModel *_model, QObject *parent)
    : QObject(parent),
      id(QUuid::createUuid().toString().toStdString()),
      model(_model)
{
}

DebugSession::~DebugSession()
{
    for (auto it : stoppedDetails) {
        if (it) {
            delete it;
            it = nullptr;
        }
    }
    stoppedDetails.clear();

    for (auto it : threads) {
        if (it.second) {
            delete it.second;
            it.second = nullptr;
        }
    }
    threads.clear();

    delete alertBox;
}

const Capabilities &DebugSession::capabilities() const
{
    return raw->capabilities();
}

bool DebugSession::initialize(const char *ip, int port, dap::InitializeRequest &iniRequest)
{
    shutdown();

    if (!raw) {
        // if there was already a connection make sure to remove old listeners
        // TODO(mozart):crashed when re-start debug.
        rtCfgProvider.reset(new RunTimeCfgProvider(/*this*/));

        constexpr int kMaxAttempts = 10;
        bool connected = false;
        // The socket might take a while to open - retry connecting.
        for (int attempt = 0; attempt < kMaxAttempts; attempt++) {
            auto connection = net::connect(ip, port);
            if (!connection) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            // Socket opened. Create the debugger session and bind.
            session.reset();
            session = dap::Session::create();
            session->bind(connection);

            raw.reset(new RawDebugSession(session /*, this*/));

            connected = true;
            break;
        }

        if (!connected) {
            return false;
        }
    }

    auto init_res = raw->initialize(iniRequest).get();
    if (init_res.error) {
        initialized = false;
        shutdown();
        qDebug() << init_res.error.message.c_str();
        return false;
    } else {
        initialized = true;
    }

    // Notify outter register handlers.
    emit sigRegisterHandlers();

    return initialized;
}

bool DebugSession::launch(dap::LaunchRequest &config)
{
    if (!raw)
        return false;

    auto ret = raw->launch(config);
    return ret.valid();
}

bool DebugSession::attach(dap::AttachRequest &config)
{
    if (!raw)
        return false;

    auto ret = raw->attach(config);
    return ret.valid();
}

void DebugSession::restart()
{
    if (!raw)
        return;

    raw->restart({});
}

void DebugSession::terminate(bool restart)
{
    if (!raw)
        return;

    raw->terminate(restart);
}

void DebugSession::disconnect(bool terminateDebuggee, bool restart)
{
    if (!raw)
        return;

    DisconnectRequest request;
    request.terminateDebuggee = terminateDebuggee;
    request.restart = restart;
    raw->disconnect(request);
}

void DebugSession::continueDbg(integer threadId)
{
    if (!raw)
        return;

    ContinueRequest request;
    request.threadId = threadId;
    raw->continueDbg(request);
}

void DebugSession::reverseContinue(integer threadId)
{
    if (!raw)
        return;

    dap::ReverseContinueRequest request;
    request.threadId = threadId;
    raw->reverseContinue(request).wait();
}

void DebugSession::pause(integer threadId)
{
    if (!raw)
        return;

    PauseRequest request;
    request.threadId = threadId;
    raw->pause(request).wait();
}

void DebugSession::terminateThreads(dap::array<integer> &threadIds)
{
    if (!raw)
        return;

    dap::TerminateThreadsRequest request;
    request.threadIds = threadIds;
    raw->terminateThreads(request);
}

dap::optional<SetVariableResponse> DebugSession::setVariable(
        integer variablesReference, string &name, string &value)
{
    if (!raw)
        return undefined;

    dap::SetVariableRequest request;
    request.variablesReference = variablesReference;
    request.name = name;
    request.value = value;
    auto response = raw->setVariable(request);
    if (response.valid())
        return response.get().response;
    return undefined;
}

dap::optional<SetExpressionResponse> DebugSession::setExpression(
        integer frameId, string &expression, string &value)
{
    if (!raw)
        return undefined;

    dap::SetExpressionRequest request;
    request.frameId = frameId;
    request.expression = expression;
    request.value = value;
    auto response = raw->setExpression(request);
    if (response.valid())
        return response.get().response;

    return undefined;
}

dap::optional<GotoTargetsResponse> DebugSession::gotoTargets(
        dap::Source &source, integer line, integer column)
{
    if (!raw)
        return undefined;

    dap::GotoTargetsRequest request;
    request.source = source;
    request.line = line;
    request.column = column;
    auto response = raw->gotoTargets(request);
    if (response.valid())
        return response.get().response;

    return undefined;
}

dap::optional<GotoResponse> DebugSession::goto_(integer threadId, integer targetId)
{
    if (!raw)
        return undefined;

    dap::GotoRequest request;
    request.threadId = threadId;
    request.targetId = targetId;
    auto response = raw->goto_(request);
    if (response.valid())
        return response.get().response;

    return undefined;
}

dap::optional<StepInTargetsResponse> DebugSession::stepInTargets(integer frameId)
{
    if (!raw)
        return undefined;

    dap::StepInTargetsRequest request;
    request.frameId = frameId;
    auto response = raw->stepInTargets(request);
    if (response.valid())
        return response.get().response;

    return undefined;
}

dap::optional<CancelResponse> DebugSession::cancel(string &progressId)
{
    if (!raw)
        return undefined;

    dap::CancelRequest request;
    request.progressId = progressId;
    auto response = raw->cancel(request);
    if (response.valid())
        return response.get().response;

    return undefined;
}

dap::optional<Thread *> DebugSession::getThread(integer threadId)
{
    auto it = threads.find(threadId);
    if (it != threads.end())
        return it->second;
    return undefined;
}

dap::optional<dap::array<IThread *>> DebugSession::getAllThreads() const
{
    dap::array<IThread *> result;
    for (auto id : threadIds) {
        auto it = threads.find(id);
        if (it != threads.end()) {
            result.push_back(it->second);
        }
    }
    dap::optional<dap::array<IThread *>> ret;
    ret = result;
    return ret;
}

void DebugSession::clearThreads(bool removeThreads, dap::optional<integer> reference)
{
    if (reference) {
        auto thread = threads.find(reference.value());
        if (thread != threads.end()) {
            auto threadptr = thread->second;
            threadptr->clearCallStack();
            threadptr->stoppedDetails = undefined;
            threadptr->stopped = false;

            if (removeThreads) {
                delete threadptr;
                threadptr = nullptr;
            }
        }
    } else {
        for (auto thread : threads) {
            auto threadptr = thread.second;
            threadptr->clearCallStack();
            threadptr->stoppedDetails = undefined;
            threadptr->stopped = false;
        }

        if (removeThreads) {
            threads.clear();
            threadIds.clear();
            //                    ExpressionContainer.allValues.clear();
        }
    }
}

void DebugSession::rawUpdate(IRawModelUpdate *data)
{
    threadIds.clear();

    for (auto thread : data->threads) {
        threadIds.push_back(thread.id);
        if (threads.find(thread.id) == threads.end()) {
            // save the new thread.
            threads.insert(std::pair<dapNumber, Thread *>(thread.id, new Thread(this, thread.name, thread.id)));
        } else if (!thread.name.empty()) {
            // update thread name.
            auto oldThread = threads.find(thread.id);
            if (oldThread != threads.end()) {
                oldThread->second->name = thread.name;
            }
        }
    }

    // Remove all old threads which are no longer part of the update
    for (auto t = threads.begin(); t != threads.end();) {
        auto it = std::find(threadIds.begin(), threadIds.end(), t->first);
        if (it == threadIds.end()) {
            t = threads.erase(t);
        } else {
            ++t;
        }
    }

    auto stoppedDetails = data->stoppedDetails;
    if (stoppedDetails) {
        if (stoppedDetails->allThreadsStopped) {
            for (auto thread : threads) {
                if (thread.second->threadId == stoppedDetails->threadId.value()) {
                    thread.second->stoppedDetails = stoppedDetails;
                    thread.second->stopped = true;
                    thread.second->clearCallStack();
                }
            }
        } else {
            if (stoppedDetails->threadId) {
                auto thread = threads.find(stoppedDetails->threadId.value());
                if (thread != threads.end()) {
                    thread->second->stoppedDetails = stoppedDetails;
                    thread->second->clearCallStack();
                    thread->second->stopped = true;
                }
            }
        }
    }
}

dap::array<IRawStoppedDetails *> &DebugSession::getStoppedDetails()
{
    return stoppedDetails;
}

void DebugSession::fetchThreads(dap::optional<IRawStoppedDetails> stoppedDetails)
{
    if (raw) {
        auto response = raw->threads();
        if (response.valid()) {
            IRawModelUpdate args { getId(), response.get().response.threads, stoppedDetails };
            model->rawUpdate(&args);
        }
    }
}

dap::optional<dap::Source> DebugSession::getSourceForUri(QUrl &uri)
{
    Q_UNUSED(uri)
    return undefined;
}

Source *DebugSession::getSource(dap::optional<dap::Source> raw)
{
    DEBUG::Source *source = new DEBUG::Source(raw, id);
    dap::string uriKey = source->uri.toString().toStdString();
    auto found = sources.find(uriKey);
    if (found != sources.end()) {
        found->second->raw.presentationHint = source->presentationHint();
        delete source;
        source = nullptr;
    } else {
        sources.insert(std::pair<dap::string, Source *>(uriKey, source));
    }

    return source;
}

void DebugSession::stepIn(dap::integer threadId,
                          dap::optional<integer> targetId,
                          dap::optional<dap::SteppingGranularity> granularity)
{
    if (!raw)
        return;

    StepInRequest request;
    request.threadId = threadId;
    request.targetId = targetId;
    request.granularity = granularity;
    setLastSteppingGranularity(threadId, granularity);
    raw->stepIn(request);
}

void DebugSession::stepOut(integer threadId, dap::optional<dap::SteppingGranularity> granularity)
{
    StepOutRequest request;
    request.threadId = threadId;
    request.granularity = granularity;
    setLastSteppingGranularity(threadId, granularity);
    raw->stepOut(request);
}

void DebugSession::stepBack(integer threadId, dap::optional<SteppingGranularity> granularity)
{
    if (!raw)
        return;

    setLastSteppingGranularity(threadId, granularity);
    dap::StepBackRequest request;
    request.threadId = threadId;
    request.granularity = granularity;
    raw->stepBack(request).wait();
}

void DebugSession::next(integer threadId, dap::optional<dap::SteppingGranularity> granularity)
{
    NextRequest request;
    request.threadId = threadId;
    request.granularity = granularity;
    raw->next(request);
}

void DebugSession::sendBreakpoints(const QString &sourcePath, dap::array<IBreakpoint> &breakpointsToSend)
{
    if (!raw || !raw->readyForBreakpoints())
        return;

    SetBreakpointsRequest request;
    request.source.path = sourcePath.toStdString();
    dap::array<SourceBreakpoint> breakpoints;
    for (auto it : breakpointsToSend) {
        dap::Source source;
        source.path = it.uri.toString().toStdString();
        source.name = undefined;
        request.source = source;
        SourceBreakpoint bt;
        bt.line = it.lineNumber;
        breakpoints.push_back(bt);
    }

    request.breakpoints = breakpoints;
    auto response = raw->setBreakpoints(request);
    if (response.valid()) {
        response.wait();

        auto data = std::map<dap::string, dap::Breakpoint>();
        for (size_t i = 0; i < breakpointsToSend.size(); ++i) {
            if (response.get().response.breakpoints.size() > i)
                data.insert(std::pair<dap::string, dap::Breakpoint>(breakpointsToSend[i].getId(), response.get().response.breakpoints[i]));
        }

        model->setBreakpointSessionData(id, capabilities(), data);
    }
}

void DebugSession::sendFunctionBreakpoints(dap::array<IFunctionBreakpoint> &fbpts)
{
    if (!raw)
        return;

    if (!raw->readyForBreakpoints()) {
        qInfo() << "break point not ready!";
        return;
    }
    SetFunctionBreakpointsRequest request;
    if (raw->readyForBreakpoints()) {
        auto response = raw->setFunctionBreakpoints(request);
        if (response.valid()) {
            auto data = std::map<dap::string, dap::Breakpoint>();
            for (size_t i = 0; i < fbpts.size(); ++i) {
                data.insert(std::pair<dap::string, dap::Breakpoint>(fbpts[i].getId(), response.get().response.breakpoints[i]));
            }

            model->setBreakpointSessionData(id, capabilities(), data);
        }
    }
}

void DebugSession::sendExceptionBreakpoints(dap::array<IExceptionBreakpoint> &exbpts) {
    Q_UNUSED(exbpts)
    // TODO(mozart)
}

dap::optional<DataBreakpointInfoResponse> DebugSession::dataBreakpointInfo(
        string &name, dap::optional<integer> variablesReference)
{
    if (!raw)
        return undefined;

    if (!raw->readyForBreakpoints()) {
        qInfo() << "break point not ready!";
        return undefined;
    }

    DataBreakpointInfoRequest request;
    request.name = name;
    request.variablesReference = variablesReference;
    auto response = raw->dataBreakpointInfo(request);
    return response.get().response;
}

void convertTodbp(const array<IDataBreakpoint> &dataBreakpoints, array<DataBreakpoint> &result)
{
    for (auto bp : dataBreakpoints) {
        DataBreakpoint dbp;
        dbp.accessType = bp.accessType;
        // An optional expression for conditional breakpoints.
        dbp.condition = bp.condition;
        // An id representing the data. This id is returned from the
        // dataBreakpointInfo request.
        dbp.dataId = bp.dataId;
        // An optional expression that controls how many hits of the breakpoint are
        // ignored. The backend is expected to interpret the expression as needed.
        dbp.hitCondition = bp.hitCondition;
        result.push_back(dbp);
    }
}

void DebugSession::sendDataBreakpoints(dap::array<IDataBreakpoint> dataBreakpoints)
{
    if (!raw)
        return;

    if (!raw->readyForBreakpoints()) {
        qInfo() << "break point not ready!";
        return;
    }

    dap::SetDataBreakpointsRequest request;
    convertTodbp(dataBreakpoints, request.breakpoints);
    auto response = raw->setDataBreakpoints(request);
    if (response.valid()) {
        auto data = std::map<dap::string, dap::Breakpoint>();
        for (size_t i = 0; i < dataBreakpoints.size(); ++i) {
            data.insert(std::pair<dap::string, dap::Breakpoint>(dataBreakpoints[i].getId(), response.get().response.breakpoints[i]));
        }
        model->setBreakpointSessionData(id, capabilities(), data);
    }
}

void DebugSession::sendInstructionBreakpoints(dap::array<IInstructionBreakpoint> instructionBreakpoints)
{
    if (!raw)
        return;

    if (!raw->readyForBreakpoints()) {
        qInfo() << "break point not ready!";
        return;
    }

    if (raw->readyForBreakpoints()) {
        SetInstructionBreakpointsRequest request;
        auto response = raw->setInstructionBreakpoints(request);
        if (response.valid()) {
            auto data = std::map<dap::string, dap::Breakpoint>();
            for (size_t i = 0; i < instructionBreakpoints.size(); ++i) {
                data.insert(std::pair<dap::string, dap::Breakpoint>(instructionBreakpoints[i].getId(), response.get().response.breakpoints[i]));
            }

            model->setBreakpointSessionData(id, capabilities(), data);
        }
    }
}

dap::optional<dap::Breakpoint> DebugSession::getDebugProtocolBreakpoint(string &breakpointId)
{
    auto id = getId();
    return model->getDebugProtocolBreakpoint(breakpointId, id);
}

dap::optional<StackTraceResponse> DebugSession::stackTrace(integer threadId, integer startFrame, integer levels)
{
    if (!raw)
        return undefined;

    dap::StackTraceRequest request;
    request.levels = levels;
    request.startFrame = startFrame;
    request.threadId = threadId;
    return raw->stackTrace(request).get().response;
}

dap::optional<IExceptionInfo> DebugSession::exceptionInfo(integer threadId)
{
    if (!raw)
        return undefined;

    dap::ExceptionInfoRequest request;
    request.threadId = threadId;

    dap::optional<IExceptionInfo> ret;
    auto response = raw->exceptionInfo(request);
    if (response.valid()) {
        auto body = response.get().response;

        IExceptionInfo info;
        info.id = body.exceptionId;
        info.description = body.description;
        info.breakMode = body.breakMode;
        info.details = body.details;
        ret = info;

        return ret;
    }
    return undefined;
}

dap::optional<ScopesResponse> DebugSession::scopes(integer frameId, integer threadId)
{
    Q_UNUSED(threadId)

    if (!raw)
        return undefined;

    dap::ScopesRequest request;
    request.frameId = frameId;
    return raw->scopes(request).get().response;
}

dap::optional<VariablesResponse> DebugSession::variables(
        integer variablesReference,
        dap::optional<integer> threadId,
        dap::optional<string> filter,
        dap::optional<integer> start,
        dap::optional<integer> count)
{
    Q_UNUSED(threadId)

    if (!raw)
        return undefined;

    VariablesRequest request;
    request.count = count;
    request.filter = filter;
    request.start = start;
    request.variablesReference = variablesReference;

    auto response = raw->variables(request);
    if (response.valid()) {
        return response.get().response;
    }
    return undefined;
}

dap::optional<EvaluateResponse> DebugSession::evaluate(string &expression, integer frameId, dap::optional<string> context)
{
    if (!raw)
        return undefined;

    dap::EvaluateRequest request;
    request.context = context;
    request.expression = expression;
    request.frameId = frameId;

    auto response = raw->evaluate(request);
    if (response.valid()) {
        return response.get().response;
    }
    return undefined;
}

void DebugSession::restartFrame(integer frameId, integer threadId)
{
    Q_UNUSED(threadId)

    if (!raw)
        return;

    dap::RestartFrameRequest request;
    request.frameId = frameId;
    raw->restartFrame(request);
}

void DebugSession::setLastSteppingGranularity(integer threadId,
                                              dap::optional<SteppingGranularity> granularity)
{
    auto thread = getThread(threadId);
    if (thread) {
        thread.value()->lastSteppingGranularity = granularity;
    }
}

string DebugSession::getId()
{
    return id;
}

integer DebugSession::getThreadId()
{
    return threadId;
}

string DebugSession::getLabel() const
{
    return "session";
}

void DebugSession::setName(string &_name)
{
    name = _name;
    // fire event.
}

void DebugSession::shutdown()
{
    if (raw) {
        raw->disconnect({});
        raw.reset(nullptr);
    }
}

// GetVariables fetches the fully traversed set of Variables from the debugger
// for the given reference identifier.
// Returns true on success, false on error.
bool DebugSession::getVariables(dap::integer variablesRef, IVariables *out, dap::integer depth/* = 0*/)
{
    if (depth > 5)
        return false;

    dap::VariablesRequest request;
    request.variablesReference = variablesRef;
    auto response = raw->variables(request);
    if (!response.valid()) {
        return false;
    }

    array<Variable> &&variables = response.get().response.variables;

    for (auto var : variables) {
        IVariable v;
        v.name = var.name;
        v.var = var;
        v.depth = depth + 1;
        out->push_back(v);
    }
    return true;
}

// GetLocals fetches the fully traversed set of local Variables from the
// debugger for the given stack frame.
// Returns true on success, false on error.
// todo: get different frame locals: select-frame and then getlocals
bool DebugSession::getLocals(dap::integer frameId, IVariables *out)
{
    dap::ScopesRequest scopeReq;
    scopeReq.frameId = frameId;

    if (!raw->scopes(scopeReq).valid()) {
        return false;
    }
    auto scopeRes = raw->scopes(scopeReq).get().response;
    for (auto scope : scopeRes.scopes) {
        if (scope.presentationHint.value("") == kLocals
                || scope.name == "Local") {
            return getVariables(0, out);
        }
    }

    qInfo() << "Locals scope not found";
    return false;
}

Session *DebugSession::getDapSession() const
{
    return session.get();
}

RawDebugSession *DebugSession::getRawSession() const
{
    return raw.get();
}

dap::array<dap::Thread> DebugSession::fetchThreads(IRawStoppedDetails *stoppedDetails)
{
    if (raw) {
        auto response = raw->threads();
        if (response.valid() && response.get().response.threads.size()) {
            dap::optional<IRawStoppedDetails> details;
            if (stoppedDetails) {
                details = *stoppedDetails;
            }
            IRawModelUpdate args { getId(), response.get().response.threads, details };
            model->rawUpdate(&args);
            return response.get().response.threads;
        }
    }
    return {};
}

void DebugSession::onBreakpointHit(const StoppedEvent &event)
{
}

void DebugSession::onStep(const StoppedEvent &event)
{
}

void DebugSession::closeSession()
{
    shutdown();
}

void DebugSession::disassemble(const dap::string &address)
{
    DisassembleRequest request;
    request.memoryReference = address;
    raw->disassemble(request);
}

}   // endnamespace
