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
#ifndef RAWDEBUGSESSION_H
#define RAWDEBUGSESSION_H

#include "session.h"
#include "protocol.h"
#include "future.h"

#include <QObject>

#include <memory>
#include <any>

namespace dap {

/**
 * @brief RawDebugSession is implemented by orignal
 * DAP protocol,used by top level class DapDebugger.
 */
class rawDebugSession : public QObject
{
    Q_OBJECT
public:
    template<typename T>
    using promiseEx = promise<ResponseOrError<typename T::Response>>;

    // Similar with the promise in ts.
    #define Promise rawDebugSession::promiseEx

    explicit rawDebugSession(QObject *parent = nullptr);

    bool initialize();

    void start();
    Promise<InitializeRequest> initialize(const InitializeRequest &request);
    bool disconnect(const DisconnectRequest &request);

    Promise<LaunchRequest> launch();
    Promise<AttachRequest> attach();

    bool terminate(bool restart);
    bool restart(const RestartRequest &request);
    Promise<NextRequest> next(const NextRequest &request);
    Promise<StepInRequest> stepIn(const StepInRequest &request);
    Promise<StepOutRequest> stepOut(const StepOutRequest &request);
    Promise<ContinueRequest> continueDbg(const ContinueRequest &request);
    Promise<PauseRequest> pause(PauseRequest &args);

    const dap::Capabilities &capabilities() const;

    bool shutdown(optional<boolean> terminateDebuggee, optional<boolean> restart = false);

signals:

public slots:

private:    
    void mergeCapabilities(const InitializeResponse &capabilities);

    std::unique_ptr<Session> session;

    Capabilities _capabilities;

    bool inShutdown = false;
    bool terminated = false;
    bool allThreadsContinued = true;
};

} // end dap namespace.

#endif // RAWDEBUGSESSION_H
