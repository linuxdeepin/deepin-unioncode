/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "session.h"
#include "protocol.h"
#include "objects.hpp"

#include <QDebug>

namespace dap {

// Similar with the promise in ts
//#define Promise rawDebugSession::promiseEx
// Make promise,T is type and R is future.
#define MakePromise(T, F)  \
    Promise<T> p; \
    p.set_value(F);

/**
 * @brief rawDebugSession implimented by original dap
 * protocol.TODO(mozart):Decide whether to use synchronous
 * or asynchronous.
 * @param parent
 */
rawDebugSession::rawDebugSession(QObject *parent) : QObject(parent)
{
}

bool rawDebugSession::initialize()
{
    bool bRet = false;
    auto tmpSession = dap::Session::create();
    if (tmpSession.get()) {
        session = std::move(tmpSession);
        bRet = true;
    }
    return bRet;
}

Promise<InitializeRequest> rawDebugSession::initialize(const InitializeRequest &args)
{
    auto response = session->send(args);
    if (response.valid()) {
        response.wait();
        auto capabilities = response.get().response;
        mergeCapabilities(capabilities);
    }
    MakePromise(InitializeRequest, response.get());
    return p;
}

bool rawDebugSession::disconnect(const DisconnectRequest &args)
{
    optional<boolean> terminateDebuggee;
    if (capabilities().supportTerminateDebuggee)
        terminateDebuggee = args.terminateDebuggee;
    return shutdown(terminateDebuggee, args.restart);
}

void rawDebugSession::mergeCapabilities(const InitializeResponse &capabilities)
{
    _capabilities = objects::mixin(_capabilities, capabilities);
}

void rawDebugSession::start()
{
    // All the handlers we care about have now been registered.
    // We now bind the session to stdin and stdout to connect to the client.
    // After the call to bind() we should start receiving requests, starting with
    // the Initialize request.
    std::shared_ptr<dap::Reader> in = dap::file(stdin, false);
    std::shared_ptr<dap::Writer> out = dap::file(stdout, false);

    // connect to server.
    session->connect(in, out);
}

Promise<LaunchRequest> rawDebugSession::launch()
{
    LaunchRequest request;
    auto response = session->send(request);
    response.wait();
    MakePromise(LaunchRequest, response.get());
    return p;
}

Promise<AttachRequest> rawDebugSession::attach()
{
    AttachRequest request;
    auto response = session->send(request);
    response.wait();
    MakePromise(AttachRequest, response.get());
    return p;
}

bool rawDebugSession::terminate(bool restart)
{
    if (capabilities().supportsTerminateRequest) {
        if (!terminated) {
            terminated = true;

            TerminateRequest request;
            auto response = session->send(request);
            response.wait();
            return true;
        }
        DisconnectRequest request;
        request.terminateDebuggee = true;
        request.restart = restart;
        disconnect(request);
        return true;
    }
    qDebug() << "terminated not supported";
    return false;
}

bool rawDebugSession::restart(const RestartRequest &request)
{
    if (capabilities().supportsRestartRequest) {
        session->send(request);
        return true;
    }
    qDebug() << "restart not supported";
    return false;
}

Promise<NextRequest> rawDebugSession::next(const NextRequest &request)
{
    auto response = session->send(request);
    response.wait();
    MakePromise(NextRequest, response.get());
    // fire notify event.
    return p;
}

Promise<StepInRequest> rawDebugSession::stepIn(const StepInRequest &request)
{
    auto response = session->send(request);
    response.wait();
    MakePromise(StepInRequest, response.get());
    // fire notify event.
    return p;
}

Promise<StepOutRequest> rawDebugSession::stepOut(const StepOutRequest &request)
{
    auto response = session->send(request);
    response.wait();
    MakePromise(StepOutRequest, response.get());
    // fire notify event.
    return p;
}

Promise<ContinueRequest> rawDebugSession::continueDbg(const ContinueRequest &request)
{
    auto response = session->send(request);
    response.wait();
    allThreadsContinued = static_cast<bool>(response.get().response.allThreadsContinued);

    MakePromise(ContinueRequest, response.get());

    return p;
}

Promise<PauseRequest> rawDebugSession::pause(PauseRequest &request)
{
    auto response = session->send(request);
    MakePromise(PauseRequest, response.get());

    return p;
}

const Capabilities &rawDebugSession::capabilities() const
{
    return _capabilities;
}

bool rawDebugSession::shutdown(optional<boolean> terminateDebuggee, optional<boolean> restart)
{
    if (!inShutdown) {
        inShutdown = true;
        DisconnectRequest request;
        request.restart = restart;
        request.terminateDebuggee = terminateDebuggee;
        session->send(request);
    }
    return true;
}
} // end namespace.
