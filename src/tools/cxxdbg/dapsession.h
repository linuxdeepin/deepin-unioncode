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
#ifndef DAPSESSION_H
#define DAPSESSION_H

#include "dap/session.h"
#include "dap/protocol.h"
#include "dap/network.h"

#include <QObject>

class DebugManager;
class DapSession : public QObject
{
    Q_OBJECT
public:
    DapSession();

    bool start();

    void stop();

public slots:
    void initialize(std::shared_ptr<dap::ReaderWriter>);

private:

    void initializeDebugMgr();
    void registerHanlder();
    void handleEvent(const QString &sOut);
    dap::SetBreakpointsResponse handleBreakpointReq(const dap::SetBreakpointsRequest &request);
    dap::InitializeResponse handleInitializeReq(const dap::InitializeRequest &request);

    std::unique_ptr<dap::Session> session;
    std::unique_ptr<dap::net::Server> server;

    bool isConfiguratedDone = false;

    bool isDebuggeIsStartWithLaunchRequest = false;
    bool isDebuggeIsStartWithAttachRequest = false;
    bool isSupportsTerminateDebuggee = true;
    bool isGdbProcessStarted = false;
    bool isGDbProcessTerminated = false;

    DebugManager *debugger = nullptr;
};

#endif   // DAPSESSION_H
