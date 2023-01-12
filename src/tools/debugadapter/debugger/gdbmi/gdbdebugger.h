/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer:
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
#ifndef GDBDEBUGGER_H
#define GDBDEBUGGER_H

#include "gdbmi.h"
#include "debugger/debugger.h"

#include "dap/protocol.h"

#include <QObject>

class GDBDebuggerPrivate;
class GDBDebugger : public Debugger
{
    Q_OBJECT
public:
    explicit GDBDebugger(QObject *parent = nullptr);
    virtual ~GDBDebugger() override;

    QString program() override;
    QStringList preArguments() override;

    QString quit() override;
    QString kill() override;

    QString breakInsert(const QString& path) override;
    QString breakRemove(int bpid) override;
    QString breakRemoveAll() override;

    QString launchLocal() override;

    QString commandPause() override;
    QString commandContinue() override;
    QString commandNext() override;
    QString commandStep() override;
    QString commandFinish() override;

    QString stackListFrames() override;
    QString stackListVariables() override;

    QString threadInfo() override;
    QString threadSelect(const int threadId) override;

    QString listSourceFiles() override;

    dap::array<dap::StackFrame> allStackframes() override;
    dap::array<dap::Thread> allThreadList() override;
    dap::array<dap::Variable> allVariableList() override;

    void handleOutputRecord(const QString &text) override;
    void handleOutputStreamText(const QString &streamText) override;

    void parseBreakPoint(const QVariant& var) override;
    void removeBreakPoint(const int bpid) override;
    void clearBreakPoint() override;
    QList<int> breakpointsForFile(const QString &filePath) override;

    bool isInferiorRunning() override;

    QString disassemble(const QString &address) override;

signals:
    void streamDebugInternal(const QStringList& textList);
    void streamConsole(const QString& text);
    void asyncStopped(const dap::StoppedEvent &stoppedEvent);
    void asyncExited(const dap::ExitedEvent &exitedEvent);
    void asyncRunning(const QString& processName, const QString& theadId);
    void libraryLoaded(const dap::ModuleEvent &moduleEvent);
    void libraryUnloaded(const dap::ModuleEvent &moduleEvent);
    void fireLocker();
    void fireStackLocker();
    void updateExceptResponse(const int token, const QVariant& payload);

    void breakpointModified(const gdbmi::Breakpoint& bp);
    void breakpointRemoved(const gdbmi::Breakpoint& bp);
    void breakpointInserted(const gdbmi::Breakpoint& bp);

    void threadGroupAdded(const gdbmi::Thread& thid);
    void threadGroupRemoved(const gdbmi::Thread& thid);
    void threadGroupStarted(const gdbmi::Thread& thid, const gdbmi::Thread& pid);
    void threadGroupExited(const gdbmi::Thread& thid, const QString& exitCode);

    void updateThreads(int currentId, const QList<gdbmi::Thread>& threads);
    void updateCurrentFrame(const gdbmi::Frame& frame);
    void updateStackFrame(const QList<gdbmi::Frame>& stackFrames);
    void updateLocalVariables(const QList<gdbmi::Variable>& variableList);

    void targetRemoteConnected();
    void gdbError(const QString& msg);
    void terminated();
    void result(int token, const QString& reason, const QVariant& results);

    void assemblerData(const QStringList &data);

public slots:

private:
    void parseNotifyData(gdbmi::Record &record);
    void parseResultData(gdbmi::Record &record);
    void sendStoppedNotify(const gdbmi::AsyncContext &ctx);
    void sendLibraryLoadedNotify(const gdbmi::Library &library);
    void sendLibraryUnloadedNotify(const gdbmi::Library &library);
    void parseDisassembleData(const gdbmi::Record &record);

    GDBDebuggerPrivate *const d;
};

#endif // GDBDEBUGGER_H
