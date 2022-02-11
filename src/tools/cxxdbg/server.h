#ifndef SERVER_H
#define SERVER_H

#include "dap/io.h"
#include "dap/protocol.h"
#include "dap/session.h"
#include "dap/network.h"

#include <QObject>

#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace  cxxdbg {
    class ServerPrivate;
    class Server;
    class CxxDebugger;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server();

    virtual ~Server();

    bool start();
    bool stop();

    std::shared_ptr<dap::net::Server> getServer();
    std::shared_ptr<dap::Session> getSession();
    std::shared_ptr<cxxdbg::CxxDebugger> getDebugger();

signals:
    // signal send to debugger to launcher gdb
    void ReadyLaunch(const QString &process,
                 const QStringList &arguments,
                 const QByteArray &writeData);
    // signal send to debugger to run
    void ReadyRun();
    // signal sent to debugger to continue
    void ReadyContinue();
    // signal sent to debugger to pause
    void ReadyPause();
    // signal sent to debugger to step in
    void ReadyStepIn();
    // signal sent to debugger to step out
    void ReadyStepOut();
    // signal sent to debugger to step over
    void ReadyStepOver();
    // signal sent to debugger to set breakpoints
    void ReadySetBreakpoints();

public slots:
    void onLaunch(const QString &process,
                   const QStringList &arguments,
                   const QByteArray &writeData);
    void onRun();
    void onContinue();
    void onPause();
    void onStepin();
    void onStepout();
    void onStepover();
    void onSetBreakpoints();

private:
    ServerPrivate *d = nullptr;
};

} // namespace cxxdbg


#endif // SERVER_H
