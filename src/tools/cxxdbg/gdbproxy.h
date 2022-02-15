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

extern bool isGdbProcessStarted;
extern bool isGDbProcessTerminated;

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
    void sigAttach(const QString& cmd);
    void sigDetach(const QString& cmd);
    void sigContinue();
    void sigPause(const QString &cmd);
    void sigNext();
    void sigStepin();
    void sigStepout();
    void sigStepover();
    void sigBreakInsert(const QString& path);
    void sigThreads(const QString& cmd);
    void sigSelectThread(const QString& cmd);
    void sigStackTrace(const QString& cmd);
    void sigSelectStackFrame(const QString& cmd);
    void sigScopes(const QString& cmd);
    void sigVariables(const QString& cmd);
    void sigSource(const QString& cmd);

public slots:
    void gdbProcessStarted();
    void gdbProcessTerminated();
    void asyncRunning(/*const QString& thid*/);
    void asyncStopped(/*const gdb::AsyncContext& ctx*/);
    void breakpointModified(/*const gdb::Breakpoint& bp*/);
    void librayLoaded();
    void libraryUnloaded();
private:
    explicit GDBProxy();
};



#endif
