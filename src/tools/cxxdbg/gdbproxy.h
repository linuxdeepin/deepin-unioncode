/**
 * C/C++ Debug Adaptor Protocol Server drived by google cppdap library
 *
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xiaozaihu<xiaozaihu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             xiaozaihu<xiaozaihu@uniontech.com>
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

#ifndef GDBPROXY_H
#define GDBPROXY_H

#include "debugmanager.h"
#include <QObject>

class GDBProxy : public QObject
{
    Q_OBJECT
public:
    // return GDBProxy instance
    static GDBProxy *instance();

    // signals to control debugger and debuggee.
Q_SIGNALS:
    void sigStart();
    void sigQuit();
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
    void sigSelectThread(const gdb::Thread& thread);
    void sigStackTrace(/*const int lowFrameLevel, const int highFrameLevel*/);
    void sigSelectStackFrame(const gdb::Frame& frame);
    void sigScopes(const qint64 frame);
    void sigVariables(/*const gdb::Thread& thid, const int frameLevel*/);
    void sigSource();
    void sigStreamOutput(const QString sOut);
private:
    explicit GDBProxy();
};



#endif
