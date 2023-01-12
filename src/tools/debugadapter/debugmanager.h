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
#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include "debugger/gdbmi/gdbdebugger.h"

#include "dap/protocol.h"

#include <QHash>
#include <QObject>

class DebugManagerPrivate;
class DebugManager : public QObject
{
    Q_OBJECT

public:
    static DebugManager *instance();

    struct ResponseEntry {
        enum class ResponseAction_t {
            Permanent,
            Temporal
        }action;

        using ResponseHandler_t = std::function<void (const QVariant& v)>;
        ResponseHandler_t handler;
    };

    void initDebugger(const QString &program, const QStringList &arguments);

    void quit();
    void kill();
    void execute();
    void launchLocal();

    qint64 getProcessId();

    void breakRemoveAll();
    void breakInsert(const QString& path);
    void removeBreakpointInFile(const QString &filePath);
    void breakRemove(int bpid);

    void commandPause();
    void commandContinue();
    void commandNext();
    void commandStep();
    void commandFinish();

    void threadInfo();
    void threadSelect(const int threadId);

    void stackListFrames();
    void stackListVariables();

    void listSourceFiles();

    dap::array<dap::StackFrame> allStackframes();
    dap::array<dap::Thread> allThreadList();
    dap::array<dap::Variable> allVariableList();

    void disassemble(const QString &address);

signals:
    void streamConsole(const QString& text);
    void streamDebugInternal(const QStringList &textList);
    void asyncStopped(const dap::StoppedEvent &stoppedEvent);
    void asyncExited(const dap::ExitedEvent &exitedEvent);
    void asyncRunning(const QString& processName, const QString& theadId);
    void libraryLoaded(const dap::ModuleEvent &moduleEvent);
    void libraryUnloaded(const dap::ModuleEvent &moduleEvent);
    void terminated();

    void assemblerData(const QStringList &data);
    void dbgProcessStarted();
    void dbgProcessFinished();

public slots:
    void updateExceptResponse(const int token, const QVariant& payload);
    void fireLocker();
    void fireStackLocker();

private:
    explicit DebugManager(QObject *parent = nullptr);
    virtual ~DebugManager();

    void initProcess();

    bool command(const QString &cmd);
    void commandAndResponse(const QString& cmd,
                            const ResponseEntry::ResponseHandler_t& handler,
                            ResponseEntry::ResponseAction_t action = ResponseEntry::ResponseAction_t::Temporal);
    bool isExecuting() const;
    void waitLocker();

    DebugManagerPrivate *const d;
};


#endif // DEBUGMANAGER_H
