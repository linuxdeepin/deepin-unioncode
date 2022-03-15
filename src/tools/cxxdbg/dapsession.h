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
#include "debugmanager.h"
#include "serverinfo.h"

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
    void handleAsyncStopped(const gdb::AsyncContext &ctx);
    void handleThreadGroupAdded(const gdb::Thread &thid);
    void handleThreadGroupRemoved(const gdb::Thread &thid);
    void handleThreadGroupStarted(const gdb::Thread &thid, const gdb::Thread &pid);
    void handleThreadGroupExited(const gdb::Thread &thid, const QString &exitCode);
    void hanleThreadCreated(const gdb::Thread &thid, const QString &groupId);
    void handleThreadExited(const gdb::Thread &thid, const QString &groupId);
    void handleThreadSelected(const gdb::Thread &thid, const gdb::Frame &frame);
    void hanldeUpdateThreads(int currentId, const QList<gdb::Thread> &threads);
    void handleLibraryLoaded(const gdb::Library &library);
    void handleLibraryUnloaded(const gdb::Library &library);
    void handleStreamConsole(const QString &text);

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
    bool isLaunchLocalTarget = false;
    bool isThreadRequestReceived = false;
    bool isInferiorStopped = false;
    bool isStackframesUpdated = false;
    bool isBreakpointHit = false;
    bool isSteppingRangeEnd = false;

    QString currentFile;
    qint64 currentLine;
    QList<gdb::Frame> stackframes;
    QString threadGroupId;
    qint64 processId;
    qint64 threadId;
    QString processName;
    QString debuggerName;
    DebugManager *debugger = nullptr;

    ServerInfo serverInfo;
};

#endif   // DAPSESSION_H
