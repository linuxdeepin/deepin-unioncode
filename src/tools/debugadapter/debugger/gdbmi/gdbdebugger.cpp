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
#include "gdbdebugger.h"

#include "debugmanager.h"

#include <QTextStream>
#include <QProcess>

class GDBDebuggerPrivate {
    friend class GDBDebugger;

    QMap<int, gdbmi::Breakpoint> breakpoints;
    QList<gdbmi::Frame> stackFrames;
    QList<gdbmi::Thread> threadList;
    QList<gdbmi::Variable> variableList;
    std::atomic_bool inferiorRunning{false};
    std::atomic_bool firstPromt{true};
};

GDBDebugger::GDBDebugger(QObject *parent)
    : Debugger(parent)
    , d(new GDBDebuggerPrivate())
{
    connect(this, &GDBDebugger::streamConsole, DebugManager::instance(), &DebugManager::streamConsole);
    connect(this, &GDBDebugger::streamDebugInternal, DebugManager::instance(), &DebugManager::streamDebugInternal);
    connect(this, &GDBDebugger::asyncStopped, DebugManager::instance(), &DebugManager::asyncStopped);
    connect(this, &GDBDebugger::asyncExited, DebugManager::instance(), &DebugManager::asyncExited);
    connect(this, &GDBDebugger::asyncRunning, DebugManager::instance(), &DebugManager::asyncRunning);
    connect(this, &GDBDebugger::libraryLoaded, DebugManager::instance(), &DebugManager::libraryLoaded);
    connect(this, &GDBDebugger::libraryUnloaded, DebugManager::instance(), &DebugManager::libraryUnloaded);
    connect(this, &GDBDebugger::fireLocker, DebugManager::instance(), &DebugManager::fireLocker);
    connect(this, &GDBDebugger::updateExceptResponse, DebugManager::instance(), &DebugManager::updateExceptResponse);
    connect(this, &GDBDebugger::terminated, DebugManager::instance(), &DebugManager::terminated);
}

GDBDebugger::~GDBDebugger()
{

}

QString GDBDebugger::program()
{
    return ("gdb");
}

QStringList GDBDebugger::preArguments()
{
    return QStringList{"-q", "-interpreter=mi"};
}

bool GDBDebugger::isInferiorRunning()
{
    return d->inferiorRunning;
}

QString GDBDebugger::quit()
{
    return ("-gdb-exit");
}

QString GDBDebugger::kill()
{
    return ("kill");
}

QString GDBDebugger::breakRemoveAll()
{
    return ("-break-delete");
}

QString GDBDebugger::breakInsert(const QString &path)
{
    return QString{"-break-insert %1"}.arg(path);
}

QString GDBDebugger::breakRemove(int bpid)
{
    return QString{"-break-delete %1"}.arg(bpid);
}

QString GDBDebugger::launchLocal()
{
    return ("-exec-run");
}

QString GDBDebugger::stackListFrames()
{
    return ("-stack-list-frames");
}

QString GDBDebugger::stackListVariables()
{
    return ("-stack-list-variables 1");
}

QString GDBDebugger::commandPause()
{
    return ("-exec-until");
}

QString GDBDebugger::commandContinue()
{
    return ("-exec-continue");
}

QString GDBDebugger::commandNext()
{
    return ("-exec-next");
}

QString GDBDebugger::commandStep()
{
    return ("-exec-step");
}

QString GDBDebugger::commandFinish()
{
    return ("-exec-finish");
}

QString GDBDebugger::threadInfo()
{
    return ("-thread-info");
}

QString GDBDebugger::threadSelect(const int threadId)
{
    return QString{"-thread-select %1"}.arg(threadId);
}

QString GDBDebugger::listSourceFiles()
{
    return ("-file-list-exec-source-files");
}

void GDBDebugger::parseBreakPoint(const QVariant& var)
{
    auto data = var.toMap();
    auto bp = gdbmi::Breakpoint::parseMap(data.value("bkpt").toMap());
    d->breakpoints.insert(bp.number, bp);
    emit breakpointInserted(bp);
}

void GDBDebugger::removeBreakPoint(const int bpid)
{
    auto bp = d->breakpoints.value(bpid);
    d->breakpoints.remove(bpid);
    emit breakpointRemoved(bp);
}

void GDBDebugger::clearBreakPoint()
{
    d->breakpoints.clear();
}

QList<int> GDBDebugger::breakpointsForFile(const QString &filePath)
{
    QList<int> list;
    for (auto it = d->breakpoints.cbegin(); it != d->breakpoints.cend(); ++it) {
        auto& bp = it.value();
        if (bp.fullname == filePath)
            list.append(bp.number);
    }
    return list;
}

dap::array<dap::StackFrame> GDBDebugger::allStackframes()
{
    dap::array<dap::StackFrame> stackFrames;
    for (const auto &frame : d->stackFrames) {
        dap::Source source;
        dap::StackFrame stackframe;
        stackframe.id = frame.level;
        stackframe.line = frame.line;
        stackframe.column = 1;
        stackframe.name = frame.func.toStdString();
        auto address = "0x" + QString::number(frame.addr, 16);
        stackframe.instructionPointerReference = address.toStdString();
        source.name = frame.func.toStdString();
        source.path = frame.fullpath.toStdString();
        stackframe.source = source;
        stackFrames.push_back(stackframe);
    }
    return stackFrames;
}

dap::array<dap::Thread> GDBDebugger::allThreadList()
{
    std::vector<dap::Thread> retThreads;
    for (auto it : d->threadList) {
        dap::Thread thread;
        thread.id = it.id;
        thread.name = it.name.toStdString();
        retThreads.push_back(thread);
    }
    return retThreads;
}

dap::array<dap::Variable> GDBDebugger::allVariableList()
{
    dap::array<dap::Variable> variables;
    for (const auto &var : d->variableList) {
        dap::Variable variable;
        variable.name = var.name.toStdString();
        variable.type = var.type.toStdString();
        variable.value = var.value.toStdString();
        variables.push_back(variable);
    }

    return variables;
}

void GDBDebugger::handleOutputStreamText(const QString &streamText)
{
    QStringList textList;
    QString temp;
    do {
        if (!streamText.contains("Command")) {
            temp += streamText.split(":").last();
        }

        if (temp.contains("=thread-group-added") && temp.contains("\(gdb\)")) {
            textList.append(temp);
            temp.clear();
        }
    } while (streamText.contains("~\"done.\n"));

    if (textList.size() > 0) {
        emit streamDebugInternal(textList);
    }
}

void GDBDebugger::handleOutputRecord(const QString &outputRecord)
{
    auto record = gdbmi::Record::parseRecord(outputRecord);

    QString sOut;
    QTextStream(&sOut) << "Response:" << outputRecord << "\n";
    handleOutputStreamText(sOut);

    switch (record.type) {
    case gdbmi::Record::RecordType::notify:
    {
        parseNotifyData(record);
        break;
    }
    case gdbmi::Record::RecordType::result:
    {
        parseResultData(record);
        break;
    }
    case gdbmi::Record::RecordType::console:
    case gdbmi::Record::RecordType::target:
    {
        emit streamConsole(gdbmi::escapedText(record.message));
        break;
    }
    case gdbmi::Record::RecordType::program:
    {
        emit streamConsole(record.payload.toString());
        break;
    }
    default:
        break;
    }
}

void GDBDebugger::parseNotifyData(gdbmi::Record &record)
{
    // code is from https://github.com/martinribelotta/gdbfrontend
    if (record.message == "stopped") {
        // *stopped, reason="reason",thread-id="id",stopped-threads="stopped",core="core"
        auto data = record.payload.toMap();
        gdbmi::AsyncContext ctx;
        ctx.reason = gdbmi::AsyncContext::textToReason(data.value("reason").toString());
        ctx.threadId = data.value("thread-id").toString();
        ctx.core = data.value("core").toInt();
        ctx.frame = gdbmi::Frame::parseMap(data.value("frame").toMap());
        d->inferiorRunning.store(false);
        sendStoppedNotify(ctx);
    } else if (record.message == "running") {
        // *running,thread-id="thread"
        auto data = record.payload.toMap();
        auto thid = data.value("thread-id").toString();
        d->inferiorRunning.store(true);
        emit asyncRunning("gdb", thid);
    } else if (record.message == "breakpoint-modified") {
        // =breakpoint-modified,bkpt={...}
        auto data = record.payload.toMap();
        auto bp = gdbmi::Breakpoint::parseMap(data.value("bkpt").toMap());
        d->breakpoints.insert(bp.number, bp);
        emit breakpointModified(bp);
    } else if (record.message == "breakpoint-created") {
        // =breakpoint-created,bkpt={...}
        auto data = record.payload.toMap();
        auto bp = gdbmi::Breakpoint::parseMap(data.value("bkpt").toMap());
        d->breakpoints.insert(bp.number, bp);
        emit breakpointModified(bp);
    } else if (record.message == "breakpoint-deleted") {
        // =breakpoint-deleted,id=number
        auto data = record.payload.toMap();
        auto id = data.value("id").toInt();
        auto bp = d->breakpoints.value(id);
        d->breakpoints.remove(id);
        emit breakpointRemoved(bp);
    } else if (record.message == "thread-group-added") {
        // =thread-group-added,id="id"
        auto data = record.payload.toMap();
        auto id = data.value("id").toInt();
        gdbmi::Thread thid;
        thid.id = id;
        emit threadGroupAdded(thid);
    } else if (record.message == "thread-group-removed") {
        // =thread-group-removed,id="id"
        auto data = record.payload.toMap();
        auto id = data.value("id").toInt();
        gdbmi::Thread thid;
        thid.id = id;
        emit threadGroupRemoved(thid);
    } else if (record.message == "thread-group-started") {
        // =thread-group-started,id="id",pid="pid"
        auto data = record.payload.toMap();
        auto id = data.value("id").toInt();
        auto pid = data.value("pid").toInt();
        gdbmi::Thread threadId;
        gdbmi::Thread processId;
        threadId.id = id;
        processId.id = pid;
        emit threadGroupStarted(threadId, processId);
    } else if (record.message == "thread-group-exited") {
        // =thread-gorup-exited,id="id"[,exit-code="code"]
        auto data = record.payload.toMap();
        gdbmi::Thread threadId;
        threadId.id = data.value("id").toInt();
        auto exitCode = data.value("exit-code").toString();
        emit threadGroupExited(threadId, exitCode);
    } else if (record.message == "library-loaded") {
        // =library-loaded,...
        auto data = record.payload.toMap();
        auto ranges = data.value("ranges").toMap();
        gdbmi::Library library;
        library.id = data.value("id").toString();
        library.targetName = data.value("target-name").toString();
        library.hostName = data.value("host-name").toString();
        library.symbolsLoaded = data.value("symbols-loaded").toString();
        library.ranges.fromRange = ranges.value("fromRange").toString();
        library.ranges.toRange = ranges.value("toRange").toString();
        sendLibraryUnloadedNotify(library);
    } else if (record.message == "library-unloaded") {
        // =library-unloaded,...
        auto data = record.payload.toMap();
        gdbmi::Library library;
        library.id = data.value("id").toString();
        library.targetName = data.value("target-name").toString();
        library.hostName = data.value("host-name").toString();
        sendLibraryUnloadedNotify(library);
    }
}

void GDBDebugger::sendStoppedNotify(const gdbmi::AsyncContext &ctx)
{
    if (gdbmi::AsyncContext::Reason::exitedNormally == ctx.reason
            || gdbmi::AsyncContext::Reason::exitedSignalled == ctx.reason
            || gdbmi::AsyncContext::Reason::Unknown == ctx.reason
            || gdbmi::AsyncContext::Reason::exited == ctx.reason) {
        dap::ExitedEvent exitedEvent;
        emit asyncExited(exitedEvent);
    }

    dap::StoppedEvent stoppedEvent;
    stoppedEvent.reason = ctx.reasonToText(ctx.reason).toStdString();
    stoppedEvent.threadId = ctx.threadId.toInt();
    stoppedEvent.allThreadsStopped = true;
    stoppedEvent.line = ctx.frame.line;
    dap::Source source;
    source.name = ctx.frame.file.toStdString();
    source.path = ctx.frame.fullpath.toStdString();
    stoppedEvent.source = source;

    emit asyncStopped(stoppedEvent);
}

void GDBDebugger::sendLibraryLoadedNotify(const gdbmi::Library &library)
{
    dap::ModuleEvent moduleEvent;
    moduleEvent.reason = "new";
    dap::Module module;
    module.id = library.id.toStdString();
    module.name = library.targetName.toStdString();
    module.path = library.hostName.toStdString();
    module.symbolFilePath = library.hostName.toStdString();
    moduleEvent.module = module;

    emit libraryLoaded(moduleEvent);
}

void GDBDebugger::sendLibraryUnloadedNotify(const gdbmi::Library &library)
{
    dap::ModuleEvent moduleEvent;
    moduleEvent.reason = "remove";
    dap::Module module;
    module.id = library.id.toStdString();
    module.name = library.targetName.toStdString();
    module.path = library.hostName.toStdString();
    module.symbolFilePath = library.hostName.toStdString();
    moduleEvent.module = module;

    emit libraryUnloaded(moduleEvent);
}

void GDBDebugger::parseResultData(gdbmi::Record &record)
{
    // code is from https://github.com/martinribelotta/gdbfrontend
    if (record.message == "done" || record.message == "") {
        foreach (QString key, record.payload.toMap().keys()) {
            if (key == "frame") {
                auto f = gdbmi::Frame::parseMap(record.payload.toMap().value("frame").toMap());
                emit updateCurrentFrame(f);
            } else if(key == "variables") {
                // -stack-list-varablbes => Scopes Request
                QList<gdbmi::Variable> variableList;
                auto locals = record.payload.toMap().value("variables").toList();
                for (const auto& e: locals)
                    variableList.append(gdbmi::Variable::parseMap(e.toMap()));
                d->variableList = variableList;
                emit updateLocalVariables(variableList);
                emit fireLocker();
            } else if(key == "threads") {
                 // -thread-info => Thread Request
                QList<gdbmi::Thread> threadList;
                auto data =  record.payload.toMap();
                auto threads = data.value("threads").toList();
                auto currId = data.value("current-thread-id").toInt();
                for (const auto& e: threads)
                    threadList.append(gdbmi::Thread::parseMap(e.toMap()));
                d->threadList = threadList;
                emit updateThreads(currId, threadList);
                emit fireLocker();
            } else if(key == "stack") {
                // -stack-list-frames => StackTrace Reqeust
                if (!d->inferiorRunning) {
                    QList<gdbmi::Frame> stackFrames;
                    auto stackTrace = record.payload.toMap().value("stack").toList().first().toMap().values("frame");
                    for (const auto& e: stackTrace) {
                        auto frame = gdbmi::Frame::parseMap(e.toMap());
                        stackFrames.prepend(frame);
                    }
                    d->stackFrames = stackFrames;
                    emit updateStackFrame(stackFrames);
                }
                emit fireLocker();
            } else if(key == "bkpt") {
                // -break-insert location
                auto data = record.payload.toMap();
                auto bp = gdbmi::Breakpoint::parseMap(data.value("bkpt").toMap());
                d->breakpoints.insert(bp.number, bp);
                emit breakpointInserted(bp);
            }

            emit updateExceptResponse(record.token, record.payload);
        }
    } else if (record.message == "connected") {
        emit targetRemoteConnected();
    } else if (record.message == "error") {
        emit gdbError(gdbmi::escapedText(record.payload.toMap().value("msg").toString()));
    } else if (record.message == "exit") {
        d->firstPromt.store(false);
        emit terminated();
    }

    emit result(record.token, record.message, record.payload);
}

