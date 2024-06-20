// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gdbdebugger.h"

#include "debugmanager.h"
#include "common/util/custompaths.h"

#include <QTextStream>
#include <QProcess>

class GDBDebuggerPrivate {
    friend class GDBDebugger;

    QMap<int, gdbmi::Breakpoint> breakpoints;
    QList<gdbmi::Frame> stackFrames;
    QList<gdbmi::Thread> threadList;

    QMultiMap<int64_t, gdbmi::Variable *> variableListByReference;
    QList<QString> createdValue;

    std::atomic_bool inferiorRunning{false};
    std::atomic_bool firstPromt{true};
    QStringList assemblers;

    int64_t reference = childVariablesReferenceBegin;
    int64_t watchVariableCounter = 0;

    int runningCommand = 0;
};

bool isPointer(const gdbmi::Variable *variable)
{
    if(variable->value.startsWith("0x") && variable->type.endsWith("*"))
        return true;
    return false;
}

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
    connect(this, &GDBDebugger::fireStackLocker, DebugManager::instance(), &DebugManager::fireStackLocker);
    connect(this, &GDBDebugger::fireVariablesLocker, DebugManager::instance(), &DebugManager::fireVariablesLocker);
    connect(this, &GDBDebugger::updateExceptResponse, DebugManager::instance(), &DebugManager::updateExceptResponse);
    connect(this, &GDBDebugger::terminated, DebugManager::instance(), &DebugManager::terminated);
    connect(this, &GDBDebugger::assemblerData, DebugManager::instance(), &DebugManager::assemblerData);
}

GDBDebugger::~GDBDebugger()
{

}

void GDBDebugger::init()
{
    //use to debugging adapter
//    DebugManager::instance()->command("set logging file /tmp/log.txt");
//    DebugManager::instance()->command("set logging on");

    QString prettyPrintersPath = CustomPaths::CustomPaths::global(CustomPaths::Scripts) + "/prettyprinters";
    DebugManager::instance()->command(QString("python sys.path.insert(0, \"%1\")").arg(prettyPrintersPath));

    DebugManager::instance()->command("python from qt import register_qt_printers");
    DebugManager::instance()->command("python register_qt_printers(None)");

    DebugManager::instance()->command("set print sevenbit-strings off");
    DebugManager::instance()->command("set breakpoint pending on");
    DebugManager::instance()->command("-enable-pretty-printing");

    DebugManager::instance()->command("set width 0");
    DebugManager::instance()->command("set height 0");
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
    return QString{"-break-insert -f %1"}.arg(path);
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
    return ("-stack-list-variables 1"); //0 or \"--no-values\", 1 or \"--all-values\", 2 or \"--simple-values\"
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

QString GDBDebugger::frameSelect(const int frameId)
{
    return QString{"-stack-select-frame %1"}.arg(frameId);
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
    return getVariableListByRef(rootVariablesReference);
}

dap::array<dap::Variable> GDBDebugger::getVariableListByRef(int64_t ref)
{
    if(!d->variableListByReference.contains(ref))
        return {};

    dap::array<dap::Variable> variables;
    for (const auto &var : d->variableListByReference.values(ref)) {
        dap::Variable variable;
        variable.name = var->name.toStdString();
        variable.evaluateName = var->evaluateName.toStdString();
        variable.type = var->type.toStdString();
        variable.value = var->value.toStdString();
        variable.variablesReference = var->childRefrence;
        variables.push_back(variable);
    }

    return variables;
}

bool GDBDebugger::fetchChildVariables(int64_t ref)
{
    if(d->variableListByReference.contains(ref))  //has alreay fetch children
        return true;

    gdbmi::Variable *parent = nullptr;
    foreach (auto var, d->variableListByReference.values()) {
        if(var->childRefrence == ref) {
            parent = var;
            break;
        }
    }

    if(!parent)
        return false;

    parseChildVariable(parent->evaluateName, parent);
    return true;
}

void GDBDebugger::handleOutputStreamText(const QString &streamText)
{
    QStringList textList;
    QString temp;
    do {
        if (!streamText.contains("Command")) {
            temp += streamText.split(":").last();
        }

        if (temp.contains("=thread-group-added") && temp.contains("\\(gdb\\)")) {
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
        break;
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
    case gdbmi::Record::RecordType::disassemble:
    {
        parseDisassembleData(record);
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
        sendLibraryUnloadedNotify(library, false);
    } else if (record.message == "library-unloaded") {
        // =library-unloaded,...
        auto data = record.payload.toMap();
        gdbmi::Library library;
        library.id = data.value("id").toString();
        library.targetName = data.value("target-name").toString();
        library.hostName = data.value("host-name").toString();
        sendLibraryUnloadedNotify(library, false);
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

void GDBDebugger::sendLibraryLoadedNotify(const gdbmi::Library &library, bool print)
{
    if (!print)
        return;
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

void GDBDebugger::sendLibraryUnloadedNotify(const gdbmi::Library &library, bool print)
{
    if (!print)
        return;
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
                QList<gdbmi::Variable *> variableList;
                auto locals = record.payload.toMap().value("variables").toList();
                for (const auto& e: locals)
                    variableList.append(gdbmi::Variable::parseMap(e.toMap()));
                resetVariables();
                addVariablesWatched(variableList, rootVariablesReference);
                if (variableList.size() == 0)
                    emit fireVariablesLocker();
                //emit updateLocalVariables(variableList); //unused
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
                emit fireStackLocker();
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
        emit updateExceptResponse(record.token, record.payload);
    } else if (record.message == "exit") {
        d->firstPromt.store(false);
        emit terminated();
    }

    emit result(record.token, record.message, record.payload);
}

QString GDBDebugger::disassemble(const QString &address)
{
    return QString{"disassemble /m %1"}.arg(address);
}

void GDBDebugger::parseDisassembleData(const gdbmi::Record &record)
{
    if (record.message == "data") {
        d->assemblers.push_back(gdbmi::escapedText(record.payload.toString()));
    } else if (record.message == "end") {
        emit assemblerData(d->assemblers);
        d->assemblers.clear();
    }
}

void GDBDebugger::traceAddVariable(gdbmi::Variable *variable, int reference, int frame)
{
    auto frameId = frame == -1 ? "@" : QString { "%1" }.arg(frame);
    auto varName = QString("var%1").arg(d->watchVariableCounter++);

    d->variableListByReference.insert(reference, variable);

    DebugManager::instance()->commandAndResponse(QString { "-var-create \"%1\" %2 \"%3\"" }.arg(varName, frameId, variable->name),
                                                 [=](const QVariant &r) {
                                                     auto m = r.toMap();

                                                     if (m.value("msg").toString().isEmpty()) {  //if msg has value means accept error
                                                         auto v = gdbmi::Variable::parseMap(m); // created variable, used to check input variable
                                                         variable->type = v->type;
                                                         variable->numChild = v->numChild;
                                                         variable->hasMore = (v->dynamic && v->hasMore) || v->numChild;
                                                         variable->evaluateName = varName;
                                                     }

                                                     d->createdValue.append(varName);

                                                     if(variable->hasMore)
                                                        variable->childRefrence = ++d->reference;

                                                     checkVariablesLocker();
                                                 });
}

void GDBDebugger::delAllTraceVariable()
{
    for (auto varName : d->createdValue)
        DebugManager::instance()->command(QString { "-var-delete %1" }.arg(varName));
    d->createdValue.clear();
}

void GDBDebugger::traceUpdateVariable(const QString &expression)
{
//    DebugManager::instance()->commandAndResponse(QString{"-var-update --all-values %1"}.arg(expression), [this](const QVariant& r) {
//        auto changeList = r.toMap().value("changelist").toList();
//        QStringList changedNames;
//        for(const auto& e: changeList) {
//            auto m = e.toMap();
//            auto name = m.value("name").toString();
//            changedNames += name;
//            auto var = self->varsWatched.value(name);
//            if (m.contains("value"))
//                var.value = m.value("value").toString();
//            if (m.value("type_changed", false).toBool())
//                var.type = m.value("new_type").toString();
//            self->varsWatched.insert(name, var);
//        }
//        emit variablesChanged(changedNames);
//    });
}

void GDBDebugger::addVariablesWatched(const QList<gdbmi::Variable *> &variableList, int reference)
{
    d->runningCommand += variableList.size();
    foreach (auto variable, variableList) {
        traceAddVariable(variable, reference);
    }
}

void GDBDebugger::parseChildVariable(const QString &evaluateName, const gdbmi::Variable *parentVariable)
{
    d->runningCommand++;
    DebugManager::instance()->commandAndResponse(QString { "-var-list-children --all-values %1" }.arg(evaluateName), [=](const QVariant &r) {
        auto m = r.toMap();
        //children: [QMap(, QMap()), QMap(, QMap()) ···];
        if((m.value("numchild").toInt() > 0) && (m.value("children").toList().size() > 0)) {
            auto childList = m.value("children").toList()[0];
            foreach (auto child , childList.toMap()) {
                auto childMap = child.toMap();
                if(childMap.value("exp").toString() == "public"
                        || childMap.value("exp").toString() == "private"
                        || childMap.value("exp").toString() == "protected"){
                    parseChildVariable(childMap.value("name").toString(), parentVariable);
                } else {
                    auto var = gdbmi::Variable::parseMap(childMap);
                    var->name = childMap.value("exp").toString();
                    var->hasMore = (var->dynamic && var->hasMore) || var->numChild;
                    if(var->hasMore)
                        var->childRefrence = ++d->reference;
                    d->variableListByReference.insert(parentVariable->childRefrence, var);
                }
            }
        }

        checkVariablesLocker();
    });
}

void GDBDebugger::evaluateValue(gdbmi::Variable *variable)
{
//    auto exp = variable->evaluateName;
//    d->runningCommand++;
//    DebugManager::instance()->command(QString { "-var-set-visualizer %1 gdb.default_visualizer" }.arg(exp));
//    DebugManager::instance()->commandAndResponse(QString { "-var-evaluate-expression %1" }.arg(exp), [=](const QVariant &r) {
//        auto m = r.toMap();
//        auto value = m.value("value").toString();

//        if (value != "{...}")
//            variable->value = value;
//        else
//            variable->value.clear();  // do more parser

//        d->runningCommand--;
//        if (d->runningCommand == 0)
//            emit fireVariablesLocker();
//        DebugManager::instance()->command(QString { "-var-set-visualizer %1 None" }.arg(exp));  //restore it  or -var-list-children will return unexpected result
//    });
}

void GDBDebugger::resetVariables()
{
    foreach (auto var, d->variableListByReference.values())
        delete var;
    d->variableListByReference.clear();
    d->runningCommand = 0;
    d->reference = childVariablesReferenceBegin;
    d->watchVariableCounter = 0;
    if(!d->createdValue.isEmpty())
        delAllTraceVariable();
}

void GDBDebugger::checkVariablesLocker()
{
    d->runningCommand--;
    if (d->runningCommand == 0)
        emit fireVariablesLocker();
}
