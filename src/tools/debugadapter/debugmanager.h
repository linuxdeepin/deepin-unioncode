// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include "debugger/gdbmi/gdbdebugger.h"
#include "backendglobal.h"

#include "dap/protocol.h"

#include <QHash>
#include <QObject>

class DebugManagerPrivate;
class DebugManager : public QObject
{
    Q_OBJECT
    friend class GDBDebugger;
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
    void terminate();
    void kill();
    void execute();
    void launchLocal();

    qint64 getProcessId();

    void updateBreakpoints(const QString &file, const QList<dap::SourceBreakpoint> &sourceBps);
    void breakRemoveAll();
    void breakInsert(const QString& path, const QString &condition = "");
    void removeBreakpointInFile(const QString &filePath);
    void breakRemove(int bpid);

    void pauseDebugger();
    void commandContinue();
    void commandNext();
    void commandStep();
    void commandFinish();

    void threadInfo();
    void threadSelect(const int threadId);
    void frameSelect(const int frameId);

    void stackListFrames();
    void stackListVariables();
    void fetchChildVariables(int64_t childReference);
    void evaluateWatchingVariable(const QString &expression, int frame);
    dap::Variable getWatchingVariable(const QString &expression);

    void listSourceFiles();

    dap::array<dap::StackFrame> allStackframes();
    dap::array<dap::Thread> allThreadList();
    dap::array<dap::Variable> getVariableList(int64_t reference);

    void disassemble(const QString &address);

signals:
    void streamConsole(const QString& text);
    void streamDebugInternal(const QStringList &textList);
    void asyncContinued(const dap::ContinuedEvent &continuedEvent);
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
    void fireVariablesLocker();

private:
    explicit DebugManager(QObject *parent = nullptr);
    virtual ~DebugManager();

    void initProcess();

    bool command(const QString &cmd, bool interrupt = false);
    void commandAndResponse(const QString& cmd,
                            const ResponseEntry::ResponseHandler_t& handler,
                            bool interrupt = false,
                            ResponseEntry::ResponseAction_t action = ResponseEntry::ResponseAction_t::Temporal);
    bool isExecuting() const;
    void waitLocker();

    DebugManagerPrivate *const d;
};


#endif // DEBUGMANAGER_H
