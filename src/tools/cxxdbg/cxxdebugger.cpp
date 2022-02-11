#include "cxxdebugger.h"
#include "context.h"
#include "server.h"
#include "dap/protocol.h"

#include <QtConcurrent>
#include <QDebug>
#include <memory>
#include <mutex>
#include <signal.h>

namespace cxxdbg {
class Server;

CxxDebugger::CxxDebugger(Server *server)
    : m_handler(this->stoppedEventHandler())
    , m_server(server)
    , m_process(new QProcess())
{
    QtConcurrent::run([=](){onReadStdOut();});
}

// stopped event handlers
// send stopped reason to client
CxxDebugger::StoppedEventHandler CxxDebugger::stoppedEventHandler()
{
    return [&](CxxDebugger::StoppedEvent stoppedEventHandler) {
        m_session = m_server->getSession();
        switch (stoppedEventHandler) {
            case CxxDebugger::StoppedEvent::Stepped : {
               dap::StoppedEvent event;
               event.reason = "step";
               event.threadId = 1;
               m_session->send(event);
            }
            case CxxDebugger::StoppedEvent::BreakpointHit : {
               dap::StoppedEvent event;
               event.reason = "breakpoint";
               event.threadId = 1;
               m_session->send(event);
            }

            case CxxDebugger::StoppedEvent::Paused : {
               dap::StoppedEvent event;
               event.reason = "pause";
               event.threadId = 1;
               m_session->send(event);
            }
        }
    };
}

// return a smart pointer to Server Object
std::shared_ptr<CxxDebugger> CxxDebugger::create(Server* server)
{
    return std::make_shared<CxxDebugger>(server);
}

// via QProcess to launch gdb and debuggee
void CxxDebugger::ReadyLaunch(const QString &program, const QStringList &arguments, const QByteArray &writeData)
{
    m_process->setProgram(program);
    m_process->setArguments(arguments);
    m_process->start();
    m_process->waitForStarted();
    qInfo() << "launch debugger ";

    m_process->waitForReadyRead();
    ReadySetBreakpoints();

    m_process->waitForReadyRead();
    ReadyRun();
}

// execute exec-run command to begin debugging
void CxxDebugger::ReadyRun()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_writeData.clear();
    m_writeData.push_back("-exec-run");
    if (m_process->isWritable()) {
        m_process->write(m_writeData);
        m_process->waitForBytesWritten();
        qInfo() << "write -exec-run finished";
    }
}

// execute exec-continue to continue debugging session
void CxxDebugger::ReadyContinue()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_writeData.clear();
    m_writeData.push_back("-exec-continue");
    if (m_process->isWritable()) {
        m_process->write(m_writeData);
        m_process->waitForBytesWritten();
        qInfo() << "write -exec-continue finished";
    }
}

// execute exec-pause to pause the debugge session
void CxxDebugger::ReadyPause()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_writeData.clear();
    m_writeData.push_back("-exec-pause");
    if (m_process->isWritable()) {
        m_process->write(m_writeData);
        m_process->waitForBytesWritten();
        qInfo() << "write -exec-pause finished";
    }

    //m_handler(StoppedEvent::Paused);
}

// execute exec-step to stepin
void CxxDebugger::ReadyStepIn()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_writeData.clear();
    m_writeData.push_back("-exec-step");
    if (m_process->isWritable()) {
        m_process->write(m_writeData);
        m_process->waitForBytesWritten();
        qInfo() << "write -exec-step finished";
    }

    //m_handler(StoppedEvent::Stepped);
}

// execute exec-finish to stepout
void CxxDebugger::ReadyStepOut()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_writeData.clear();
    m_writeData.push_back("-exec-finish");
    if (m_process->isWritable()) {
        m_process->write(m_writeData);
        m_process->waitForBytesWritten();
        qInfo() << "write -exec-finished finished";
    }
    //m_handler(StoppedEvent::Stepped);
}

// execute exec-next to step over
void CxxDebugger::ReadyStepOver()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_writeData.clear();
    m_writeData.push_back("-exec-next");
    if (m_process->isWritable()) {
        m_process->write(m_writeData);
        m_process->waitForBytesWritten();
        qInfo() << "write -exec-next finished";
    }

    m_handler(StoppedEvent::Stepped);
}

// exec break-insert to set breakpoints
void CxxDebugger::ReadySetBreakpoints()
{
    m_writeData.clear();
    m_writeData.push_back("-break-insert main");
    if (m_process->isWritable()) {
        m_process->write(m_writeData);
        m_process->waitForBytesWritten();
        qInfo() << "write -break-insert main finished";
    }
}

int64_t CxxDebugger::getCurrentLine()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_line;
}

void CxxDebugger::addBreakpoint(int64_t line)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    this->m_breakpoints.emplace(line);
}

void CxxDebugger::clearAllBreakpoints()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    this->m_breakpoints.clear();
}

// start a thread to read data from gdb
static int readCount = 0;
void CxxDebugger::onReadStdOut()
{
    readCount++;
    qInfo() << "read data from debugger and debuggee: " << readCount;

    QString readData;
    while(m_process->bytesAvailable()) {
        readData = QString::fromUtf8(m_process->readAllStandardOutput());
        if (readData.isEmpty()) {
            qInfo() << "read data empty";
        } else {
            qInfo() << readData;
        }
    }

//    while (m_process->bytesAvailable()) {
//        auto lineData = QString::fromUtf8(m_process->readLine());
//        if (lineData.startsWith('~')) {
//            if (lineData.contains("done")) {
//                qInfo() << "done event ";
//            }
//            if (lineData.contains("Reading symblos from")) {
//                qInfo() << "reading symbols event";
//            }
//            if (lineData.contains("Breakpoint")) {
//                qInfo() << "breakpoint hit event";
//            }
//        }

//        if (lineData.startsWith('=')) {
//            if (lineData.contains("thread-group-added")) {
//                qInfo() << "thread added event";
//            }
//            if (lineData.contains("thread-created")) {
//                qInfo() << "thread create event";
//            }
//            if (lineData.contains("library-loaded")) {
//                qInfo() << "library loaded event";
//            }
//            if (lineData.contains("breakpoint-modified")) {
//                qInfo() << "breakpoint modified event";
//            }
//        }

//        if (lineData.startsWith('^')) {
//            if (lineData.contains("running")) {
//                qInfo() << "debugge running event";
//            }
//            if (lineData.contains("error")) {
//                qInfo() << "debugge error event";
//            }
//        }

//        if (lineData.startsWith('*')) {
//            if (lineData.contains("stopped")) {
//                qInfo() << "stopped event";
//            }
//        }

//        if (lineData.startsWith('-')) {
//            qInfo() << "gdb mi command echo";
//        }

//        if (lineData.startsWith('(')) {
//            qInfo() << "gdb wait next command";
//        }
//    }
}

}

