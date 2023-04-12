/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
