// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSDEBUGGER_H
#define JSDEBUGGER_H

#include "debugger/debugger.h"

#include <QObject>

class JSDebuggerPrivate;
class JSDebugger : public Debugger
{
    Q_OBJECT
public:
    explicit JSDebugger(QObject *parent = nullptr);
    ~JSDebugger();

    void init() override;
    QString program() override;
    QStringList preArguments() override;

    QString quit() override;
    QString kill() override;

    void updateBreakpoints(const QString &file, const QList<dap::SourceBreakpoint> &sourceBps) override;
    QString breakInsert(const QString& path, const QString &condition = "") override;
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
    QString frameSelect(const int threadId) override;

    QString listSourceFiles() override;

    dap::array<dap::StackFrame> allStackframes() override;
    dap::array<dap::Thread> allThreadList() override;
    dap::array<dap::Variable> allVariableList() override;
    dap::array<dap::Variable> getVariableListByRef(int64_t ref) override;
    bool fetchChildVariables(int64_t ref) override;
    void traceWatchingVariable(const QString &expression, int frame) override;
    dap::Variable getWatchingVariable(const QString &expression) override;

    void handleOutputRecord(const QString &text) override;
    void handleOutputStreamText(const QString &streamText) override;

    void parseBreakPoint(const QVariant& var) override;
    void removeBreakPoint(const int bpid) override;
    void clearBreakPoint() override;
    QList<int> breakpointsForFile(const QString &filePath) override;

    bool isInferiorRunning() override;

signals:
    void asyncStopped(const dap::StoppedEvent &stoppedEvent);

public slots:

private:
    JSDebuggerPrivate *const d;
};

#endif // JSDEBUGGER_H
