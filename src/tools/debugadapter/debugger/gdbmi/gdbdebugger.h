// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    void init() override;
    QString program() override;
    QStringList preArguments() override;

    QString quit() override;
    QString kill() override;

    QString breakInsert(const QString& path) override;
    QString breakRemove(int bpid) override;
    QString breakRemoveAll() override;

    QString launchLocal() override;

    void pause() override;
    QString commandContinue() override;
    QString commandNext() override;
    QString commandStep() override;
    QString commandFinish() override;

    QString stackListFrames() override;
    QString stackListVariables() override;

    QString threadInfo() override;
    QString threadSelect(const int threadId) override;
    QString frameSelect(const int frameId) override;

    QString listSourceFiles() override;

    dap::array<dap::StackFrame> allStackframes() override;
    dap::array<dap::Thread> allThreadList() override;
    dap::array<dap::Variable> allVariableList() override;
    dap::array<dap::Variable> getVariableListByRef(int64_t ref) override;
    bool fetchChildVariables(int64_t ref) override;

    void traceAddVariable(gdbmi::Variable *variable, int refrence = 0, int frame = -1);
    void delAllTraceVariable();
    void traceUpdateVariable(const QString &expression);

    void handleOutputRecord(const QString &text) override;
    void handleOutputStreamText(const QString &streamText) override;

    void updateBreakpoints(const QString &file, const QList<int> &lines) override;
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
    void asyncContinued(const dap::ContinuedEvent &continuedEvent);
    void asyncExited(const dap::ExitedEvent &exitedEvent);
    void asyncRunning(const QString& processName, const QString& theadId);
    void libraryLoaded(const dap::ModuleEvent &moduleEvent);
    void libraryUnloaded(const dap::ModuleEvent &moduleEvent);
    void fireLocker();
    void fireStackLocker();
    void fireVariablesLocker();
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
    void updateLocalVariables(const QList<gdbmi::Variable *>& variableList);

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
    void sendLibraryLoadedNotify(const gdbmi::Library &library, bool print);
    void sendLibraryUnloadedNotify(const gdbmi::Library &library, bool print);
    void parseDisassembleData(const gdbmi::Record &record);
    void addVariablesWatched(const QList<gdbmi::Variable *> &variableList, int reference);
    void parseChildVariable(const QString &evaluateName , const gdbmi::Variable *parentVariable);
    void evaluateValue(gdbmi::Variable *variable);
    void resetVariables();
    void checkVariablesLocker();

    GDBDebuggerPrivate *const d;
};

#endif // GDBDEBUGGER_H
