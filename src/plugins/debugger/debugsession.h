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
class DebugService;

class DebugSession : public QObject
{
    Q_OBJECT
public:
    explicit DebugSession(QObject *parent = nullptr);

    dap::Capabilities capabilities() const;

    bool initialize(const char *ip, int port, dap::InitializeRequest &iniRequest);

    bool launch(const char *config, bool noDebug = false);
    bool attach(dap::AttachRequest &config);

    void restart();
    void terminate(bool restart = false);
    void disconnect(bool terminateDebuggee = true, bool restart = false);

    void continueDbg(dap::integer threadId);
    void pause(dap::integer threadId);

    void stepIn(dap::integer threadId, dap::optional<dap::integer> targetId, dap::optional<dap::SteppingGranularity> granularity);
    void stepOut(dap::integer threadId, dap::optional<dap::SteppingGranularity> granularity);
    void next(dap::integer threadId, dap::optional<dap::SteppingGranularity> granularity);

    void sendBreakpoints(dap::array<IBreakpoint> &breakpointsToSend);

    dap::string getId();
    dap::integer getThreadId();
signals:

public slots:

private:
    void shutdown();
    void registerHandlers();
    void fetchThreads(IRawStoppedDetails stoppedDetails);
    void onBreakpointHit(const dap::StoppedEvent &event);
    void onStep(const dap::StoppedEvent &event);

    QSharedPointer<dap::RawDebugSession> raw;
    QSharedPointer<RunTimeCfgProvider> rtCfgProvider;

    bool initialized = false;

    std::shared_ptr<dap::Session> session;

    std::string id;

    dap::integer threadId = 0;
};

#endif   // DEBUGSESSION_H
