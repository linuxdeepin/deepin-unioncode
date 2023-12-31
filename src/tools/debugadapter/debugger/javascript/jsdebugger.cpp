// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsdebugger.h"
#include "debugmanager.h"

#include <QDebug>
#include <QRegularExpression>

#define RET_EMPTY return {};
class JSDebuggerPrivate
{
    friend class JSDebugger;

    dap::array<dap::StackFrame> stackframes;
    dap::array<dap::Thread> threads;
    dap::array<dap::Variable> variables;
    std::atomic_bool inferiorRunning{false};
};

JSDebugger::JSDebugger(QObject *parent)
    : Debugger(parent), d(new JSDebuggerPrivate())
{
    connect(this, &JSDebugger::asyncStopped, DebugManager::instance(), &DebugManager::asyncStopped);
}

JSDebugger::~JSDebugger()
{

}

QString JSDebugger::program()
{
    return "qsdbg";
}

QStringList JSDebugger::preArguments()
{
    RET_EMPTY
}

QString JSDebugger::quit()
{
    RET_EMPTY
}

QString JSDebugger::kill()
{
    RET_EMPTY
}

QString JSDebugger::breakInsert(const QString &path)
{
    return ".break " + path;
}

QString JSDebugger::breakRemove(int bpid)
{
    return ".delete " + QString::number(bpid);
}

QString JSDebugger::breakRemoveAll()
{
    return ".delall";
}

QString JSDebugger::launchLocal()
{
    return "";
}

QString JSDebugger::commandPause()
{
    RET_EMPTY
}

QString JSDebugger::commandContinue()
{
    return ".continue";
}

QString JSDebugger::commandNext()
{
    return ".next";
}

QString JSDebugger::commandStep()
{
    return ".step";
}

QString JSDebugger::commandFinish()
{
    RET_EMPTY
}

QString JSDebugger::stackListFrames()
{
    return ".backtrace";
}

QString JSDebugger::stackListVariables()
{
    return ".info locals";
}

QString JSDebugger::threadInfo()
{
    RET_EMPTY
}

QString JSDebugger::threadSelect(const int threadId)
{
    Q_UNUSED(threadId);
    RET_EMPTY
}

QString JSDebugger::listSourceFiles()
{
    return ".list";
}

dap::array<dap::StackFrame> JSDebugger::allStackframes()
{
    return d->stackframes;
}

dap::array<dap::Thread> JSDebugger::allThreadList()
{
    return d->threads;
}

dap::array<dap::Variable> JSDebugger::allVariableList()
{
    return d->variables;
}

void JSDebugger::handleOutputRecord(const QString &text)
{
    qInfo() << text;
}

void JSDebugger::handleOutputStreamText(const QString &streamText)
{
    qInfo() << streamText;
}

void JSDebugger::parseBreakPoint(const QVariant &var)
{
    Q_UNUSED(var)
}

void JSDebugger::removeBreakPoint(const int bpid)
{
    Q_UNUSED(bpid)
}

void JSDebugger::clearBreakPoint()
{
}

QList<int> JSDebugger::breakpointsForFile(const QString &filePath)
{
    Q_UNUSED(filePath)
    RET_EMPTY
}

bool JSDebugger::isInferiorRunning() {
    return d->inferiorRunning;
}
