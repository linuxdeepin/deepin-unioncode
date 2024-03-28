// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void sigSendToClient(const QString &uuid, int port, const QString &kit, const QMap<QString, QVariant> &param);

public slots:
    void initialize(std::shared_ptr<dap::ReaderWriter>);
    void slotReceiveClientInfo(const QString &uuid, const QString &kit, const QString &targetPath, const QStringList &arguments);

private:
    void initializeDebugMgr();
    void registerHanlder();
    void registerDBusConnect();

    void handleOutputTextEvent(const QStringList &textList);
    void handleStreamConsole(const QString &text);
    void handleAsyncContinued(const dap::ContinuedEvent &continuedEvent);
    void handleAsyncStopped(const dap::StoppedEvent &stoppedevent);
    void handleAsyncExited(const dap::ExitedEvent &exitedEvent);
    void handleLibraryLoaded(const dap::ModuleEvent &moduleEvent);
    void handleLibraryUnloaded(const dap::ModuleEvent &moduleEvent);
    void handleThreadExited(const int threadId, const QString &groupId);
    void handleTerminateEvent();
    void handleAssembleData(const QStringList &data);

    dap::SetBreakpointsResponse handleBreakpointReq(const dap::SetBreakpointsRequest &request);
    dap::InitializeResponse handleInitializeReq(const dap::InitializeRequest &request);

    DapSessionPrivate *const d;
};

#endif   // DAPSESSION_H
