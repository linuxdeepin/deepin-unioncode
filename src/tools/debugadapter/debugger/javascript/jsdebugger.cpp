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

void JSDebugger::init()
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

void JSDebugger::updateBreakpoints(const QString &file, const QList<int> &lines)
{
    DebugManager::instance()->breakRemoveAll();
    auto filePath = file;
    for (auto line : lines) {
        filePath.append(":");
        filePath.append(QString::number(line));
        DebugManager::instance()->breakInsert(filePath);
    }
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

void JSDebugger::pause()
{
    return;
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

QString JSDebugger::frameSelect(const int frameId)
{
    Q_UNUSED(frameId);
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

dap::array<dap::Variable> JSDebugger::getVariableListByRef(int64_t ref)
{
    return d->variables;
}

bool JSDebugger::fetchChildVariables(int64_t ref)
{
    return true;
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
