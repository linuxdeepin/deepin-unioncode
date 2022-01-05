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
RawDebugSession::RawDebugSession(std::shared_ptr<Session> &_session, QObject *parent)
    : QObject(parent)
    , session(_session)
{
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
    auto response = session->send(args);
    auto capabilities = response.get().response;
    mergeCapabilities(capabilities);
    MakePromise(InitializeRequest, response.get());
    return p;
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
    qDebug() << "Error :" << error.data();
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
    auto response = session->send(request);
    MakePromise(LaunchRequest, response.get());
    return p;
}

Promise<AttachRequest> RawDebugSession::attach(const AttachRequest &request)
{
    auto response = session->send(request);
    MakePromise(AttachRequest, response.get());
    return p;
}

bool RawDebugSession::terminate(bool restart)
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

bool RawDebugSession::restart(const RestartRequest &request)
{
    if (capabilities().supportsRestartRequest) {
        session->send(request);
        return true;
    }
    qDebug() << "restart not supported";
    return false;
}

Promise<NextRequest> RawDebugSession::next(const NextRequest &request)
{
    auto response = session->send(request);
    response.wait();
    MakePromise(NextRequest, response.get());
    // fire notify event.
    return p;
}

Promise<StepInRequest> RawDebugSession::stepIn(const StepInRequest &request)
{
    auto response = session->send(request);
    response.wait();
    MakePromise(StepInRequest, response.get());
    // fire notify event.
    return p;
}

Promise<StepOutRequest> RawDebugSession::stepOut(const StepOutRequest &request)
{
    auto response = session->send(request);
    response.wait();
    MakePromise(StepOutRequest, response.get());
    // fire notify event.
    return p;
}

Promise<ContinueRequest> RawDebugSession::continueDbg(const ContinueRequest &request)
{
    auto response = session->send(request);
    response.wait();
    allThreadsContinued = static_cast<bool>(response.get().response.allThreadsContinued);

    MakePromise(ContinueRequest, response.get());

    return p;
}

Promise<PauseRequest> RawDebugSession::pause(PauseRequest &request)
{
    auto response = session->send(request);
//    MakePromise(PauseRequest, response.get());

    return {};
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
        send(request);
    }
    return true;
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
bool RawDebugSession::send(const REQUEST &request)
{
    using RESPONSE = typename REQUEST::Response;
    RESPONSE response;
    return send(request, &response);
}

} // end namespace.
