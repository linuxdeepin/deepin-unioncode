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

    virtual QString listSourceFiles() = 0;

    virtual dap::array<dap::StackFrame> allStackframes() = 0;
    virtual dap::array<dap::Thread> allThreadList() = 0;
    virtual dap::array<dap::Variable> allVariableList() = 0;

    virtual void handleOutputRecord(const QString &text) = 0;
    virtual void handleOutputStreamText(const QString &streamText) = 0;

    virtual void parseBreakPoint(const QVariant& var) = 0;
    virtual void removeBreakPoint(const int bpid) = 0;
    virtual void clearBreakPoint() = 0;
    virtual QList<int> breakpointsForFile(const QString &filePath) = 0;

    virtual bool isInferiorRunning() = 0;

signals:

public slots:

private:

};

#endif // DEBUGGER_H
