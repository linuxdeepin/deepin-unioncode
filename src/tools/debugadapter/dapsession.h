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

#include "serverinfo.h"

#include "dap/session.h"
#include "dap/protocol.h"
#include "dap/network.h"

#include <QObject>

class DapSessionPrivate;
class DapSession : public QObject
{
    Q_OBJECT
public:
    explicit DapSession(QObject *parent = nullptr);
    virtual ~DapSession();

    bool start();
    void stop();

signals:
    void sigSendToClient(const QString &uuid, int port);

public slots:
    void initialize(std::shared_ptr<dap::ReaderWriter>);
    void slotReceiveClientInfo(const QString &uuid);

private:
    void initializeDebugMgr();
    void registerHanlder();
    void registerDBusConnect();

    void handleOutputTextEvent(const QStringList &textList);
    void handleStreamConsole(const QString &text);
    void handleAsyncStopped(const dap::StoppedEvent &stoppedevent);
    void handleAsyncExited(const dap::ExitedEvent &exitedEvent);
    void handleLibraryLoaded(const dap::ModuleEvent &moduleEvent);
    void handleLibraryUnloaded(const dap::ModuleEvent &moduleEvent);
    void handleThreadExited(const int threadId, const QString &groupId);

    dap::SetBreakpointsResponse handleBreakpointReq(const dap::SetBreakpointsRequest &request);
    dap::InitializeResponse handleInitializeReq(const dap::InitializeRequest &request);

    DapSessionPrivate *const d;
};

#endif   // DAPSESSION_H
