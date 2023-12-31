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

signals:
    void asyncStopped(const dap::StoppedEvent &stoppedEvent);

public slots:

private:
    JSDebuggerPrivate *const d;
};

#endif // JSDEBUGGER_H
