// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAPPROXY_H
#define DAPPROXY_H

#include "dap/protocol.h"
#include <QObject>

class DapProxy final: public QObject
{
    Q_OBJECT
public:
    static DapProxy *instance();

Q_SIGNALS:
    void sigStart();
    void sigQuit();
    void sigKill();
    void sigLaunchLocal();
    void sigLaunchRemote(const QString& remoteTarget);
    void sigAttachProcess(const int pid);
    void sigAttachThreadGroup(const QString& gid);
    void sigDetachProcess(const int pid);
    void sigDetachThreadGroup(const QString& gid);
    void sigDetach();
    void sigDisconnect();
    void sigContinue();
    void sigPause();
    void sigNext();
    void sigStepin();
    void sigStepout();
    void sigStepover();
    void sigBreakInsert(const QString& path);
    void sigThreads();
    void sigSelectThread(const int threadId);
    void sigStackTrace();
    void sigSelectStackFrame(const dap::StackFrame& stackFrame);
    void sigScopes(const qint64 frame);
    void sigVariables();
    void sigSource();
    void sigStreamOutput(const QString &sOut);
    void sigBreakRemoveAll();
    void sigUpdateBreakpoints(const QString &file, const QList<dap::SourceBreakpoint> &sourceBps);

private:
    explicit DapProxy(QObject *parent = nullptr);
    virtual ~DapProxy();
};

#endif //DAPPROXY_H
