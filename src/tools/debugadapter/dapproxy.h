/**
 * C/C++ Debug Adaptor Protocol Server drived by google cppdap library
 *
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xiaozaihu<xiaozaihu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
    void sigStreamOutput(const QString sOut);
    void sigBreakRemoveAll();

private:
    explicit DapProxy(QObject *parent = nullptr);
    virtual ~DapProxy();
};

#endif //DAPPROXY_H
