// Scintilla source code edit control
/** @file ProcessQt.cpp
 ** Class for launching a child process - Qt version
 **/
// Copyright (c) 2019 by Robox s.p.a. <m.lazzarotto@robox.it>
// The License.txt file describes the conditions under which this software may be distributed.

#include "../../lsp/process.h"

#ifdef WIN32
//#include <Windows.h>
//#include <processthreadsapi.h>
#else
#include <QProcess>
#include <QCoreApplication>
#endif

std::size_t Scintilla::Process::GetCurrentProcessId()
{
#ifdef WIN32
    return ::GetCurrentProcessId();
#else
    return static_cast<std::size_t>(QCoreApplication::applicationPid());
#endif
}

#include <QDeadlineTimer>
#include <QEventLoop>
#include <QProcess>
#include <QCoreApplication>

namespace Scintilla {

class ProcessPrivate: public QProcess
{
    friend class Process;
#ifdef WIN32
#else
    void setNativeArguments(QStringList arguments)
    {
        setArguments(arguments);
    }
#endif
};

Process::Process()
    : p(new ProcessPrivate)
{
    QObject::connect(p, &QProcess::started, [this]() {
        onProcessStarted();
    });
    QObject::connect(p, &QProcess::readyReadStandardOutput, [this]() {
        p->setReadChannel(QProcess::StandardOutput);
        const auto avail = p->bytesAvailable();
        outDataAvailable(avail);
    });
    QObject::connect(p, &QProcess::readyReadStandardError, [this]() {
        p->setReadChannel(QProcess::StandardError);
        const auto avail = p->bytesAvailable();
        errDataAvailable(avail);
    });
    QObject::connect(p, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [&](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::CrashExit)
            qWarning("Process crashed with code %d", exitCode);
        else
            qDebug("Process exited with code %d", exitCode);
        onProcessExited(exitCode);
    });
}

Process::~Process()
{
    delete p;
}

void Process::setExecutable(const char *exec, const char *params)
{
    p->setProgram(QString::fromUtf8(exec));
    p->setNativeArguments(QStringList(params));
}

int Process::start()
{
    p->start(QIODevice::ReadWrite);
    return 0;	// TODO: coding of errors for process
}

bool Process::isStarted() const
{
    return (p->state() != QProcess::NotRunning);
}

bool Process::canReadLineStdOut()
{
    p->setReadChannel(QProcess::StandardOutput);
    return p->canReadLine();
}

bool Process::canReadLineStdErr()
{
    p->setReadChannel(QProcess::StandardError);
    return p->canReadLine();
}

int Process::writeToStdin(const char *buf, std::size_t len)
{
    return int(p->write(buf, qint64(len)));
}

int Process::readFromStdout(char *buf, std::size_t maxLen)
{
    p->setReadChannel(QProcess::StandardOutput);
    return int(p->read(buf, qint64(maxLen)));
}

int Process::readLineFromStdout(std::string &line)
{
    // Read single line
    p->setReadChannel(QProcess::StandardOutput);
    QByteArray _line = p->readLine();
    // Eat the line terminator
    while (_line.size() > 0 && (_line.endsWith('\r') || _line.endsWith('\n')))
        _line.chop(1);
    // Convert and return size
    line = _line.toStdString();
    return int(line.size());
}

int Process::readFromStderr(char *buf, std::size_t maxLen)
{
    p->setReadChannel(QProcess::StandardError);
    return int(p->read(buf, qint64(maxLen)));
}

int Process::readLineFromStderr(std::string &line)
{
    // Read single line
    p->setReadChannel(QProcess::StandardError);
    QByteArray _line = p->readLine();
    // Eat the line terminator
    while (_line.size() > 0 && (_line.endsWith('\r') || _line.endsWith('\n')))
        _line.chop(1);
    // Convert and return size
    line = _line.toStdString();
    return int(line.size());
}

int Process::waitProcessFinished(int tout_ms)
{
    QDeadlineTimer stop(tout_ms);
    while ((p->state() != QProcess::NotRunning) && !stop.hasExpired())
    {
        QCoreApplication::processEvents();
        p->waitForFinished(1);
    }
    return p->exitCode();
}

void Process::pollIO()
{
}

void Process::outDataAvailable(std::size_t sz)
{
    Q_UNUSED(sz);
    p->setReadChannel(QProcess::StandardOutput);
    while (p->canReadLine())
        outLineAvailable();
}

void Process::outLineAvailable()
{
    std::string out;
    if (readLineFromStdout(out) > 0)
        qDebug("OUT: '%s'", out.data());
}

void Process::errDataAvailable(std::size_t sz)
{
    Q_UNUSED(sz);
    p->setReadChannel(QProcess::StandardError);
    while (p->canReadLine())
        errLineAvailable();
}

void Process::errLineAvailable()
{
    std::string err;
    if (readLineFromStderr(err) > 0)
        qDebug("LSP: '%s'", err.data());
}

}	// namespace Scintilla
