// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "dap/protocol.h"
#include <QObject>

class Debugger : public QObject
{
    Q_OBJECT
public:
    explicit Debugger(QObject *parent = nullptr);
    virtual ~Debugger() = 0;

    virtual QString program() = 0;
    virtual QStringList preArguments() = 0;
    virtual QString quit() = 0;
    virtual QString kill() = 0;
    virtual QString launchLocal() = 0;

    virtual QString breakRemoveAll() = 0;
    virtual QString breakInsert(const QString& path) = 0;
    virtual QString breakRemove(int bpid) = 0;

    virtual QString commandPause() = 0;
    virtual QString commandContinue() = 0;
    virtual QString commandNext() = 0;
    virtual QString commandStep() = 0;
    virtual QString commandFinish() = 0;

    virtual QString threadInfo() = 0;
    virtual QString threadSelect(const int threadId) = 0;

    virtual QString stackListFrames() = 0;
    virtual QString stackListVariables() = 0;
    virtual bool fetchChildVariables(int64_t ref) = 0;

    virtual QString listSourceFiles() = 0;

    virtual dap::array<dap::StackFrame> allStackframes() = 0;
    virtual dap::array<dap::Thread> allThreadList() = 0;
    virtual dap::array<dap::Variable> allVariableList() = 0;
    virtual dap::array<dap::Variable> getVariableListByRef(int64_t ref) = 0;

    virtual void handleOutputRecord(const QString &text) = 0;
    virtual void handleOutputStreamText(const QString &streamText) = 0;

    virtual void parseBreakPoint(const QVariant& var) = 0;
    virtual void removeBreakPoint(const int bpid) = 0;
    virtual void clearBreakPoint() = 0;
    virtual QList<int> breakpointsForFile(const QString &filePath) = 0;

    virtual bool isInferiorRunning() = 0;

    virtual QString disassemble(const QString &address) { Q_UNUSED(address) return "";}

signals:

public slots:

private:

};

#endif // DEBUGGER_H
