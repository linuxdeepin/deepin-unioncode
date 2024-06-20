// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debugmanager.h"
#include "locker.h"

#include "debugger/debugger.h"
#include "debugger/gdbmi/gdbdebugger.h"
#include "debugger/javascript/jsdebugger.h"

#include <QProcess>

class DebugManagerPrivate {
    friend class DebugManager;

    QSharedPointer<Debugger> debugger;
    QSharedPointer<QProcess> process{nullptr};
    QString tempBuffer;
    ConditionLockEx locker;
    ConditionLockEx stacklocker;
    QHash<int, DebugManager::ResponseEntry> resposeExpected;
    QStringList arguments;
    int tokenCounter = 0;
};

DebugManager::DebugManager(QObject *parent)
    : QObject(parent)
    , d(new DebugManagerPrivate())
{

}

DebugManager::~DebugManager()
{
    if (d) {
        delete d;
    }
}

DebugManager *DebugManager::instance()
{
    static DebugManager ins;
    return &ins;
}

void DebugManager::initProcess()
{
    d->process.reset(new QProcess());

    connect(d->process.data(), &QProcess::readyReadStandardOutput, [this]() {
        QString output = d->process->readAllStandardOutput();
        for (const auto& c: output)
            switch (c.toLatin1()) {
            case '\r':
            case '\n':
            {
                d->tempBuffer.append(c);
                d->debugger->handleOutputRecord(d->tempBuffer);
                d->tempBuffer.clear();
                break;
            }
            default:
            {
                d->tempBuffer.append(c);
                break;
            }
            }
    });
    connect(d->process.data(), &QProcess::readyReadStandardError, [this]() {
        QString output = d->process->readAllStandardError();
        for (const auto& c: output)
            switch (c.toLatin1()) {
            case '\r':
            case '\n':
            {
                d->tempBuffer.append(c);
                d->debugger->handleOutputRecord(d->tempBuffer);
                d->tempBuffer.clear();
                break;
            }
            default:
            {
                d->tempBuffer.append(c);
                break;
            }
            }
    });

    connect(d->process.data(), &QProcess::started, [this]() {
        d->tokenCounter = 0;
        d->tempBuffer.clear();
        d->resposeExpected.clear();
    });

    connect(d->process.data(), &QProcess::started,
            this, &DebugManager::dbgProcessStarted);
    connect(d->process.data(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DebugManager::dbgProcessFinished);
}

void DebugManager::initDebugger(const QString &program, const QStringList &arguments)
{
    if (d->process) {
        d->process->terminate();
    }

    d->arguments = arguments;

    if (program.contains("gdb")) {
        d->debugger.reset(new GDBDebugger());
    } else if (program.contains("jsdbg")) {
        d->debugger.reset(new JSDebugger());
    }
}

qint64 DebugManager::getProcessId()
{
    return d->process->processId();
}

void DebugManager::waitLocker()
{
    d->locker.wait();
}

void DebugManager::fireLocker()
{
    d->locker.fire();
}

void DebugManager::fireStackLocker()
{
    d->stacklocker.fire();
}

bool DebugManager::isExecuting() const
{
    return d->process->state() != QProcess::NotRunning;
}

void DebugManager::execute()
{
    initProcess();

    foreach (auto arg, d->debugger->preArguments()) {
        d->arguments.prepend(arg);
    }
    d->process->setArguments(d->arguments);
    d->process->setProgram(d->debugger->program());
    d->process->start();
}

bool DebugManager::command(const QString &cmd)
{
    if (d->debugger->isInferiorRunning())
        return false;

    auto tokStr = QString{"%1"}.arg(d->tokenCounter, 6, 10, QChar{'0'});
    auto line = QString{"%1%2%3"}.arg(tokStr, cmd, "\n");
    d->tokenCounter = (d->tokenCounter + 1) % 999999;
    d->process->write(line.toLocal8Bit());
    d->process->waitForBytesWritten();
    QString sOut;
    QTextStream(&sOut) << "Command:" << line << "\n";
    d->debugger->handleOutputStreamText(sOut);

    return true;
}

void DebugManager::commandAndResponse(const QString& cmd,
                                      const ResponseEntry::ResponseHandler_t& handler,
                                      ResponseEntry::ResponseAction_t action)
{
    d->resposeExpected.insert(d->tokenCounter, { action, handler });
    command(cmd);
}

void DebugManager::launchLocal()
{
    command(d->debugger->launchLocal());
}

void DebugManager::quit()
{
    command(d->debugger->quit());
}

void DebugManager::terminate()
{
    d->process->terminate();
    emit terminated();
}

void DebugManager::kill()
{
    command(d->debugger->kill());
}

void DebugManager::breakRemoveAll()
{
    commandAndResponse(d->debugger->breakRemoveAll(), [this](const QVariant&) {
        d->debugger->clearBreakPoint();
    });
}

void DebugManager::breakInsert(const QString &path)
{
    commandAndResponse(d->debugger->breakInsert(path), [this](const QVariant& r) {
        d->debugger->parseBreakPoint(r);
    });
}

void DebugManager::updateExceptResponse(const int token, const QVariant& payload)
{
    if (d->resposeExpected.contains(token)) {
        auto& expect = d->resposeExpected.value(token);
        expect.handler(payload);
        if (expect.action == ResponseEntry::ResponseEntry::ResponseAction_t::Temporal)
            d->resposeExpected.remove(token);
    }
}

void DebugManager::removeBreakpointInFile(const QString &filePath)
{
    auto breakPointIds = d->debugger->breakpointsForFile(filePath);
    foreach(auto bpid, breakPointIds) {
        breakRemove(bpid);
    }
}

void DebugManager::breakRemove(int bpid)
{
    commandAndResponse(d->debugger->breakRemove(bpid), [this, bpid](const QVariant&) {
        d->debugger->removeBreakPoint(bpid);
    });
}

void DebugManager::stackListFrames()
{
    if (command(d->debugger->stackListFrames()))
        d->stacklocker.wait();
}

void DebugManager::stackListVariables()
{
    if (command(d->debugger->stackListVariables()))
        waitLocker();
}

void DebugManager::threadInfo()
{
    if (command(d->debugger->threadInfo()))
        waitLocker();
}

void DebugManager::commandPause()
{
    command(d->debugger->commandPause());
}

void DebugManager::commandContinue()
{
    command(d->debugger->commandContinue());
}

void DebugManager::commandNext()
{
    command(d->debugger->commandNext()); //step over
}

void DebugManager::commandStep()
{
    command(d->debugger->commandStep()); //step in
}

void DebugManager::commandFinish()
{
    command(d->debugger->commandFinish()); //step out
}

void DebugManager::threadSelect(const int threadId)
{
    command(d->debugger->threadSelect(threadId));
}

void DebugManager::listSourceFiles()
{
    command(d->debugger->listSourceFiles());
}

dap::array<dap::StackFrame> DebugManager::allStackframes()
{
    return d->debugger->allStackframes();
}

dap::array<dap::Thread> DebugManager::allThreadList()
{
    return d->debugger->allThreadList();
}

dap::array<dap::Variable> DebugManager::allVariableList()
{
    return d->debugger->allVariableList();
}

void DebugManager::disassemble(const QString &address)
{
    command(d->debugger->disassemble(address));
}
