#ifndef CXXDEBUGGER_H
#define CXXDEBUGGER_H

#include "dap/protocol.h"
#include "dap/session.h"
#include "server.h"

#include <QProcess>
//#include <QThread>

#include <memory>
#include <mutex>
#include <functional>
#include <unordered_set>

namespace cxxdbg {
class Server;

// CxxDebugger holds the gdb debugger state
class CxxDebugger : public QObject
{
    Q_OBJECT
public:
    enum class StoppedEvent
    {
      BreakpointHit,
      Stepped,
      Paused
    };

    using StoppedEventHandler = std::function<void(StoppedEvent)>;

public:
    explicit CxxDebugger(Server *server);

    StoppedEventHandler stoppedEventHandler();
    static std::shared_ptr<CxxDebugger> create(Server *server);

    // launch gdb
    void ReadyLaunch(const QString &program, const QStringList &arguments, const QByteArray &writeData);

    // instructs the debugger to run debuggee
    void ReadyRun();

    // instructs the debugger to continue execution
    void ReadyContinue();

    // instructs the debugger to pause debuggee
    void ReadyPause();

    // instructs the debugger to step into
    void ReadyStepIn();

    // instructs the debugger to stop out
    void ReadyStepOut();

    // instructs the debugger to step over
    void ReadyStepOver();

    // instructs the debugger to set breakpoints
    void ReadySetBreakpoints();

    int64_t getCurrentLine();
    void addBreakpoint(int64_t line);
    void clearAllBreakpoints();

signals:


private slots:
    void onReadStdOut();

private:
    std::mutex m_mutex;
    bool stopped = true;
    qint64 m_pid = 0;
    QString m_program;
    QStringList m_arguments;
    QByteArray m_writeData;
    QProcess *m_process = nullptr;
    StoppedEventHandler m_handler;
    int64_t m_line = 1;
    std::unordered_set<int64_t> m_breakpoints;
    Server *m_server;
    std::shared_ptr<dap::Session> m_session = nullptr;
};

}

#endif // CXXDEBUGGER_H
