/**
 *  Code is from gdbfront
 *  See on: https://github.com/martinribelotta/gdbfrontend
 *
 *  Copyright (C) 2020 Martin Alejandro Ribelotta <martinribelotta@gmail.com>
 *
 *  Copyright (C) 2022 UnionTech Ltd.
 **/
#include "debugmanager.h"

#include <QProcess>
#include <QVariant>
#include <QMultiMap>
#include <QTextCodec>
#include <QTextStream>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QtDebug>
#include <csignal>
#include <atomic>

namespace mi {

constexpr auto DEFAULT_GDB_COMMAND = "gdb";
constexpr auto EOL = "\n";


QString escapedText(const QString& s)
{
    return QString{s}.replace(QRegularExpression{R"(\\(.))"}, "\1");
}

// Partial code is from pygdbmi
// See on: https://github.com/cs01/pygdbmi/blob/master/pygdbmi/gdbmiparser.py

struct {
    const QRegularExpression::PatternOption DOTALL = QRegularExpression::DotMatchesEverythingOption;
    QRegularExpression compile(const QString& text,
                               QRegularExpression::PatternOption flags=QRegularExpression::NoPatternOption) {
        return QRegularExpression{text, QRegularExpression::MultilineOption | flags};
    }
} re;

// GDB machine interface output patterns to match
// https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI-Stream-Records.html#GDB_002fMI-Stream-Records

// https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI-Result-Records.html#GDB_002fMI-Result-Records
// In addition to a number of out-of-band notifications,
// the response to a gdb/mi command includes one of the following result indications:
// done, running, connected, error, exit
const auto _GDB_MI_RESULT_RE = re.compile(R"(^(\d*)\^(\S+?)(?:,(.*))?$)");

// https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI-Async-Records.html#GDB_002fMI-Async-Records
// Async records are used to notify the gdb/mi client of additional
// changes that have occurred. Those changes can either be a consequence
// of gdb/mi commands (e.g., a breakpoint modified) or a result of target activity
// (e.g., target stopped).
const auto _GDB_MI_NOTIFY_RE = re.compile(R"(^(\d*)[*=](\S+?),(.*)$)");

// https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI-Stream-Records.html#GDB_002fMI-Stream-Records
// "~" string-output
// The console output stream contains text that should be displayed
// in the CLI console window. It contains the textual responses to CLI commands.
const auto _GDB_MI_CONSOLE_RE = re.compile(R"re(~"(.*)")re", re.DOTALL);

// https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI-Stream-Records.html#GDB_002fMI-Stream-Records
// "&" string-output
// The log stream contains debugging messages being produced by gdb's internals.
const auto _GDB_MI_LOG_RE = re.compile(R"re(&"(.*)")re", re.DOTALL);

// https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI-Stream-Records.html#GDB_002fMI-Stream-Records
// "@" string-output
// The target output stream contains any textual output from the
// running target. This is only present when GDB's event loop is truly asynchronous,
// which is currently only the case for remote targets.
const auto _GDB_MI_TARGET_OUTPUT_RE = re.compile(R"re(@"(.*)")re", re.DOTALL);

// Response finished
const auto _GDB_MI_RESPONSE_FINISHED_RE = re.compile(R"(^\(gdb\)\s*$)");

const QString _CLOSE_CHARS{"}]\""};

struct Response {
    enum Type_t {
        unknown,
        notify,
        result,
        console,
        log,
        target,
        promt
    } type;

    QString message;
    QVariant payload;
    int token = 0;

    Response(Type_t t=unknown, const QString& m={}, const QVariant& p={}, int tok=-1) :
        type{t}, message{m}, payload{p}, token(tok) {}

    bool isValud() const { return type != unknown; }
};

bool response_is_finished(const QString& gdb_mi_text)
{
    // Return true if the gdb mi response is ending
    // Returns: True if gdb response is finished
    return _GDB_MI_RESPONSE_FINISHED_RE.match(gdb_mi_text).hasMatch();
}

namespace priv {

QString::const_iterator& skipspaces(QString::const_iterator& it)
{
    while (it->isSpace())
        ++it;
    return it;
}

QString parseString(const QString& s, QString::const_iterator& it)
{
    QString v;
    while (it != s.cend()) {
        if (*it == '"')
            break;
        if (*it == '\\')
            if (++it == s.cend())
                break;
        v += *it++;
    }
    ++it;
    return v;
}

QString parseKey(const QString& str, QString::const_iterator& it)
{
    QString key;
    while (it != str.cend()) {
        if (*it == '=')
            break;
        if (!it->isSpace())
            key += *it;
        ++it;
    }
    return key;
}

QString consumeTo(QChar c, const QString& str, QString::const_iterator& it)
{
    QString consumed;
    while (it != str.cend()) {
        if (*it == c) {
            ++it;
            break;
        }
        consumed += *it++;
    }
    return consumed;
}

QVariantList parseArray(const QString& str, QString::const_iterator& it);
QVariantMap parseKeyVal(const QString& str, QString::const_iterator& it, QChar terminator='\0');
QVariantMap parseDict(const QString& str, QString::const_iterator& it);
QVariant parseValue(const QString& str, QString::const_iterator& it, QChar terminator);

QVariantList parseArray(const QString& str, QString::const_iterator& it)
{
    QVariantList l;
    while (it != str.cend() && *it != ']') {
        l.append(parseValue(str, it, ']'));
        if (*it == ']') {
            ++it;
            break;
        }
        consumeTo(',', str, it);
    }
    return l;
}

QVariantMap parseDict(const QString& str, QString::const_iterator& it)
{
    QVariantMap m = parseKeyVal(str, it, '}');
    ++it;
    return m;
}

QVariantMap parseKeyVal(const QString& str, QString::const_iterator& it, QChar terminator)
{
    QVariantMap m;
    while (it != str.cend() && *it != terminator) {
        auto k = parseKey(str, skipspaces(it));
        auto v = parseValue(str, skipspaces(++it), terminator);
//        qInfo() << "Key => " << k;
//        qInfo() << "Value => " << k;
        m.insertMulti(k,v);
        //m.insert(k, v); //insertMulti??
        if (*it == terminator) {
            break;
        }
        consumeTo(',', str, it);
    }
    return m;
}

QVariant parseValue(const QString& str, QString::const_iterator& it, QChar terminator)
{
    if (*it == '"') {
        return parseString(str, ++it);
    } else if (*it == '[') {
        return parseArray(str, ++it);
    } else if (*it == '{') {
        return parseDict(str, ++it);
    }
    if (mi::_CLOSE_CHARS.contains(*it))
        return {};
    return parseKeyVal(str, it, terminator);
}

}

QVariantMap parseElements(const QString& str)
{
    auto it = str.cbegin();
    return priv::parseKeyVal(str, it);
}

int strToInt(const QString& s, int def)
{
    bool ok = false;
    int v = s.toInt(&ok);
    return ok? v:def;
}

Response parse_response(const QString& gdb_mi_text)
{
    // Parse gdb mi text and turn it into a dictionary.
    // See https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI-Stream-Records.html#GDB_002fMI-Stream-Records
    // for details on types of gdb mi output.
    // Args:
    //     gdb_mi_text (str): String output from gdb
    // Returns:
    //    dict with the following keys:
    //    type (either 'notify', 'result', 'console', 'log', 'target', 'done'),
    //    message (str or None),
    //    payload (str, list, dict, or None)
    //
    QRegularExpressionMatch m;
    if ((m = _GDB_MI_NOTIFY_RE.match(gdb_mi_text)).hasMatch()) {
        return { Response::notify, m.captured(2), parseElements(m.captured(3)), strToInt(m.captured(1), -1) };
    } else if ((m = _GDB_MI_RESULT_RE.match(gdb_mi_text)).hasMatch()) {
//        qInfo() << "captured =>" << m.captured(3);
//        qInfo() << "parsed =>" << parseElements(m.captured(3));
        return { Response::result, m.captured(2), parseElements(m.captured(3)), strToInt(m.captured(1), -1) };
    } else if ((m = _GDB_MI_CONSOLE_RE.match(gdb_mi_text)).hasMatch()) {
        return { Response::console, m.captured(1), m.captured(0) };
    } else if ((m = _GDB_MI_LOG_RE.match(gdb_mi_text)).hasMatch()) {
        return { Response::log, m.captured(1), m.captured(0) };
    } else if ((m = _GDB_MI_TARGET_OUTPUT_RE.match(gdb_mi_text)).hasMatch()) {
        return { Response::target, m.captured(1), m.captured(0) };
    } else if (_GDB_MI_RESPONSE_FINISHED_RE.match(gdb_mi_text).hasMatch()) {
        return { Response::promt, {}, {}};
    } else {
        // This was not gdb mi output, so it must have just been printed by
        // the inferior program that's being debugged
        return { Response::unknown, {}, gdb_mi_text };
    }
}

}

struct DebugManager::Priv_t
{
    int tokenCounter = 0;
    QProcess *gdb;
    QString buffer;
    struct ResponseEntry {
        DebugManager::ResponseAction_t action;
        DebugManager::ResponseHandler_t handler;
    };

    QHash<int, ResponseEntry> resposeExpected;
    bool m_remote = false;
    bool m_inferiorRunning = false;
    std::atomic_bool m_firstPromt{true};
    QMap<int, gdb::Breakpoint> breakpoints;
    QMap<QString, gdb::Variable> varsWatched;
    QList<gdb::Frame> stackFrames;
    QList<gdb::Thread> threadList;
    QList<gdb::Variable> variableList;

    explicit Priv_t(DebugManager *self) : gdb(new QProcess(self))
    {
    }
};

namespace gdbprivate {
static bool registered = false;
void registerMetatypes() {
    if (registered)
        return;
    qRegisterMetaType<gdb::Frame>();
    qRegisterMetaType<gdb::Breakpoint>();
    qRegisterMetaType<gdb::Variable>();
    qRegisterMetaType<gdb::Thread>();
    qRegisterMetaType<gdb::AsyncContext>();
    qRegisterMetaType<gdb::Library>();
    registered = true;
}
}

template<typename Tret>
static Tret strmap(const QMap<QString, Tret>& map, const QString& key, Tret def)
{
    return map.value(key, def);
}

DebugManager::DebugManager(QObject *parent) :
    QObject(parent),
    self(new Priv_t{this})
{
    gdbprivate::registerMetatypes();
    setGdbCommand(mi::DEFAULT_GDB_COMMAND);
    connect(self->gdb, &QProcess::readyReadStandardOutput, [this]() {
        for (const auto& c: QString{self->gdb->readAllStandardOutput()})
            switch (c.toLatin1()) {
            case '\r': break;
            case '\n':
                processLine(self->buffer);
                self->buffer.clear();
                break;
            default: self->buffer.append(c); break;
            }
    });
    connect(self->gdb, &QProcess::started, [this]() {
        emit gdbProcessStarted();
        self->tokenCounter = 0;
        self->buffer.clear();
        self->resposeExpected.clear();
        self->varsWatched.clear();;
        self->m_remote = false;
        self->m_firstPromt.store(true);
    });
    // update connect
    connect(self->gdb, QOverload<int>::of(&QProcess::finished),
            this, &DebugManager::gdbProcessTerminated);
}

DebugManager::~DebugManager()
{
    delete self;
}

DebugManager *DebugManager::instance()
{
    static DebugManager *self = nullptr;
    if (!self)
        self = new DebugManager;
    return self;
}

QString DebugManager::gdbCommand() const
{
    return self->gdb->program();
}

bool DebugManager::isRemote() const
{
    return self->m_remote;
}

bool DebugManager::isGdbExecuting() const
{
    return self->gdb->state() != QProcess::NotRunning;
}

QList<gdb::Breakpoint> DebugManager::allBreakpoints() const
{
    return self->breakpoints.values();
}

QList<gdb::Breakpoint> DebugManager::breakpointsForFile(const QString &filePath) const
{
    QList<gdb::Breakpoint> list;
    for (auto it = self->breakpoints.cbegin(); it != self->breakpoints.cend(); ++it) {
        auto& bp = it.value();
        if (bp.fullname == filePath)
            list.append(bp);
    }
    return list;
}

gdb::Breakpoint DebugManager::breakpointById(int id) const
{
    return self->breakpoints.value(id);
}

gdb::Breakpoint DebugManager::breakpointByFileLine(const QString &path, int line) const
{
    for (auto it = self->breakpoints.cbegin(); it != self->breakpoints.cend(); ++it) {
        auto& bp = it.value();
        if (bp.fullname == path && bp.line == line)
            return bp;
    }
    return {};
}

bool DebugManager::isInferiorRunning() const
{
    return self->m_inferiorRunning;
}

qint64 DebugManager::getProcessId()
{
    return self->gdb->processId();
}

QList<gdb::Frame> DebugManager::allStackframes()
{
    return self->stackFrames;
}

QList<gdb::Thread> DebugManager::allThreadList()
{
    return self->threadList;
}

QList<gdb::Variable> DebugManager::allVariableList()
{
    return self->variableList;
}

QStringList DebugManager::gdbArgs() const
{
    return self->gdb->arguments();
}

// gdb command line from QCommandLineParser
void DebugManager::execute()
{
    auto a = self->gdb->arguments();
    a.prepend("-q");
    a.prepend("-interpreter=mi");
    self->gdb->setArguments(a);
    self->gdb->setProgram(gdbCommand());
    self->gdb->start();
}

void DebugManager::quit()
{
    command("-gdb-exit");
}

void DebugManager::command(const QString &cmd)
{
    auto tokStr = QString{"%1"}.arg(self->tokenCounter, 6, 10, QChar{'0'});
    auto line = QString{"%1%2%3"}.arg(tokStr, cmd, mi::EOL);
    self->tokenCounter = (self->tokenCounter + 1) % 999999;
    self->gdb->write(line.toLocal8Bit());
    self->gdb->waitForBytesWritten();
    QString sOut;
    QTextStream(&sOut) << "gdbCommand: " << line << "\n";
    emit streamDebugInternal(sOut);
}

void DebugManager::commandAndResponse(const QString& cmd,
                                      const ResponseHandler_t& handler,
                                      ResponseAction_t action)
{
    self->resposeExpected.insert(self->tokenCounter, { action, handler });
    command(cmd);
}

void DebugManager::breakAfter(int bpid, int count)
{
    command(QString{"-break-after %1 %2"}.arg(bpid, count));
}

void DebugManager::breakCondition(const int bpid, const QString& expr)
{
    command(QString{"-break-condition %1 %2"}.arg(bpid).arg(expr));
}

void DebugManager::breakDisable(int bpid)
{
    command(QString{"-break-disable %1"}.arg(bpid));
}

void DebugManager::breakEnable(int bpid)
{
    command(QString{"-break-enable %1"}.arg(bpid));
}

void DebugManager::breakInfo(int bpid)
{
    auto tokStr = QString{"-break-info %1"}.arg(bpid);
    commandAndResponse(tokStr, [this, bpid](const QVariant&) {
        auto bp = self->breakpoints.value(bpid);
        emit breakpointInfo(bp);
    });
}

void DebugManager::breakList()
{
    command(QString{"-break-list"});
}

void DebugManager::breakRemove(int bpid)
{
    commandAndResponse(QString{"-break-delete %1"}.arg(bpid), [this, bpid](const QVariant&) {
        auto bp = self->breakpoints.value(bpid);
        self->breakpoints.remove(bpid);
        emit breakpointRemoved(bp);
    });
}

void DebugManager::breakInsert(const QString &path)
{
    commandAndResponse(QString{"-break-insert %1"}.arg(path), [this](const QVariant& r) {
        auto data = r.toMap();
        auto bp = gdb::Breakpoint::parseMap(data.value("bkpt").toMap());
        self->breakpoints.insert(bp.number, bp);
        emit breakpointInserted(bp);
    });
}

void DebugManager::loadExecutable(const QString &file)
{
    command(QString{"-file-exec-and-symbols %1",}.arg(file));
}

void DebugManager::launchLocal()
{
    command("-exec-run");
    self->m_remote = false;
}

void DebugManager::attachProcess(const int pid)
{
    command(QString{"-target-attach %1"}.arg(pid));
}

void DebugManager::attachThreadGroup(const QString &gid)
{
    command(QString{"-target-attach %1"}.arg(gid));
}

void DebugManager::detach()
{
    command(QString{"-target-detach"});
}

void DebugManager::detachProcess(const int pid)
{
    command(QString{"-target-detach %1"}.arg(pid));
}

void DebugManager::detachThreadGroup(const QString &gid)
{
    command(QString{"-target-detach %1"}.arg(gid));
}

void DebugManager::disconnect()
{
    command(QString{"-target-disconnect"});
}

void DebugManager::enableFrameFilters()
{
    command("-enable-frame-filters");
}

void DebugManager::stackInfoDepth(const gdb::Thread &thid, const int depth)
{
    const int maxDepth = 1000;
    Q_UNUSED(thid);
    if (depth <= maxDepth) {
        command(QString{"-stack-info-depth %1"}.arg(depth));
    } else {
        command(QString{"-stack-info-depth %1"}.arg(maxDepth));
    }
}

void DebugManager::stackListFrames()
{
//    command(QString{"-stack-select-frame"}.arg(startFrame));
    command(QString{"-stack-list-frames"});
}

void DebugManager::stackInfoFrame()
{
    command("-stack-info-frame");
}

void DebugManager::stackListLocals(const gdb::Thread& thid, const int frameLevel)
{
    // select threadid
    command(QString{"-thread-select %1"}.arg(thid.id));
    // select frame level
    command(QString{"-stack-select-frame %1"}.arg(frameLevel));
    command("-stack-list-locals 1");
    // find locals in results
}

void DebugManager::stackListVariables(/*const gdb::Thread& thid, const int frameLevel*/)
{
//    // select threadid
//    command(QString{"-thread-select %1"}.arg(thid.id));
//    // select frame level
//    command(QString{"-stack-select-frame %1"}.arg(frameLevel));
    command("-stack-list-variables 1");
    // find variables in results
}

void DebugManager::launchRemote(const QString &remoteTarget)
{
    command(QString{"-target-select remote %1"}.arg(remoteTarget));
    self->m_remote = true;
}

void DebugManager::commandPause()
{
    command("-exec-until");
}

void DebugManager::commandContinue()
{
    command("-exec-continue");
}

void DebugManager::commandContinueReverse()
{
    command("-exec-continue --reverse");
}

void DebugManager::commandNext()
{
    command("-exec-next");
}

void DebugManager::commandNextReverse()
{
    command("-exec-next --reverse");
}

void DebugManager::commandNexti()
{
    command("-exec-next-instruction");
}

void DebugManager::commandNextiReverse()
{
    command("-exec-next-instruction --reverse");
}

void DebugManager::commandStep()
{
    command("-exec-step");
}

void DebugManager::commandStepReverse()
{
    command("-exec-step --reverse");
}

void DebugManager::commandStepi()
{
    command("-exec-step-instruction");
}

void DebugManager::commandStepiReverse()
{
    command("-exec-step-instruction --reverse");
}

void DebugManager::commandReturn()
{
    command("-exec-return");
}

void DebugManager::commandFinish()
{
    command("-exec-finish");
}

void DebugManager::commandFinishReverse()
{
    command("-exec-finish --reverse");
}

void DebugManager::commandInterrupt()
{
    ::kill(self->gdb->processId(), SIGINT);
    //command(QString{"-exec-interrupt"});
}

void DebugManager::commandUntil(const QString& location)
{
    command(QString{"-exec-until %1"}.arg(location));
}

void DebugManager::commandJump(const QString &location)
{
    command(QString{"-exec-jump %1"}.arg(location));
}

void DebugManager::threadInfo()
{
    command("-thread-info");
    // find current-thread-id in results
}

void DebugManager::threadListIds()
{
    command("-thread-list-ids");
}

void DebugManager::threadSelect(const gdb::Thread &thread)
{
    auto id = thread.id;
    command(QString{"-thread-select %1"}.arg(id));
}

void DebugManager::listSourceFiles()
{
    command("-file-list-exec-source-files");
}

void DebugManager::traceAddVariable(const QString& expr, const QString& name, int frame)
{
    auto frameId = frame==-1? "@" : QString{"%1"}.arg(frame);
    commandAndResponse(QString{"-var-create \"%1\" %2 \"%3\""}.arg(name, frameId, expr), [this](const QVariant& r) {
        auto m = r.toMap();
        auto v = gdb::Variable::parseMap(m);
        self->varsWatched.insert(v.name, v);
        emit variableCreated(v);
    });
}

void DebugManager::traceDelVariable(const QString &name)
{
    commandAndResponse(QString{"-var-delete %1"}.arg(name), [this, name](const QVariant&) {
        emit variableDeleted(self->varsWatched.value(name));
        self->varsWatched.remove(name);
    });
}

void DebugManager::traceUpdateVariable(const QString &name)
{
    commandAndResponse(QString{"-var-update --all-values %1"}.arg(name), [this](const QVariant& r) {
        auto changeList = r.toMap().value("changelist").toList();
        QStringList changedNames;
        for(const auto& e: changeList) {
            auto m = e.toMap();
            auto name = m.value("name").toString();
            changedNames += name;
            auto var = self->varsWatched.value(name);
            if (m.contains("value"))
                var.value = m.value("value").toString();
            if (m.value("type_changed", false).toBool())
                var.type = m.value("new_type").toString();
            self->varsWatched.insert(name, var);
        }
        emit variablesChanged(changedNames);
    });
}

void DebugManager::setGdbCommand(QString gdbCommand)
{
    self->gdb->setProgram(gdbCommand);
}

void DebugManager::setGdbArgs(QStringList gdbArgs)
{
    self->gdb->setArguments(gdbArgs);
}

const QMap<QString, gdb::Variable> &DebugManager::vatchVars() const
{
    return self->varsWatched;
}

void DebugManager::processLine(const QString &line)
{
    using dispatcher_t = std::function<void(const mi::Response&)>;
//    qInfo() << "GDB => " << line;
    auto r = mi::parse_response(line);
//    qInfo() << "Type =>" << r.type;
//    qInfo() << "Payload => " << r.payload;
    QString sOut;
    QTextStream(&sOut) << "gdbResponse: " << line << "\n";
    emit streamDebugInternal(sOut);

    switch (r.type) {
    case mi::Response::notify:
        static const QMap<QString, dispatcher_t> responseDispatcher{
            {  // *stopped, reason="reason",thread-id="id",stopped-threads="stopped",core="core"
                "stopped", [this](const mi::Response& r) {
                auto data = r.payload.toMap();
                gdb::AsyncContext ctx;
                ctx.reason = gdb::AsyncContext::textToReason(data.value("reason").toString());
                ctx.threadId = data.value("thread-id").toString();
                ctx.core = data.value("core").toInt();
                ctx.frame = gdb::Frame::parseMap(data.value("frame").toMap());
                self->m_inferiorRunning = false;
                emit asyncStopped(ctx);
             } },
             {  // *running,thread-id="thread"
                "running", [this](const mi::Response& r) {
                 auto data = r.payload.toMap();
                 auto thid = data.value("thread-id").toString();
                 self->m_inferiorRunning = true;
                 emit asyncRunning(thid);
             } },
            {   // =breakpoint-modified,bkpt={...}
                "breakpoint-modified", [this](const mi::Response& r) {
                 auto data = r.payload.toMap();
                 auto bp = gdb::Breakpoint::parseMap(data.value("bkpt").toMap());
                 self->breakpoints.insert(bp.number, bp);
                 emit breakpointModified(bp);
             } },
            {   // =breakpoint-created,bkpt={...}
                "breakpoint-created", [this](const mi::Response& r) {
                 auto data = r.payload.toMap();
                 auto bp = gdb::Breakpoint::parseMap(data.value("bkpt").toMap());
                 self->breakpoints.insert(bp.number, bp);
                 emit breakpointModified(bp);
             } },
            {    // =breakpoint-deleted,id=number
                 "breakpoint-deleted", [this](const mi::Response& r) {
                 auto data = r.payload.toMap();
                 auto id = data.value("id").toInt();
                 auto bp = self->breakpoints.value(id);
                 self->breakpoints.remove(id);
                 emit breakpointRemoved(bp);
             } },
            {
                // =thread-group-added,id="id"
                "thread-group-added", [this](const mi::Response& r) {
                auto data = r.payload.toMap();
                auto id = data.value("id").toInt();
                gdb::Thread thid;
                thid.id = id;
                emit threadGroupAdded(thid);
            }},
            {
                // =thread-group-removed,id="id"
                "thread-group-removed", [this](const mi::Response& r) {
                auto data = r.payload.toMap();
                auto id = data.value("id").toInt();
                gdb::Thread thid;
                thid.id = id;
                emit threadGroupRemoved(thid);
            }},
            {
                // =thread-group-started,id="id",pid="pid"
                "thread-group-started", [this](const mi::Response& r) {
                 auto data = r.payload.toMap();
                 auto id = data.value("id").toInt();
                 auto pid = data.value("pid").toInt();
                 gdb::Thread threadId;
                 gdb::Thread processId;
                 threadId.id = id;
                 processId.id = pid;
                 emit threadGroupStarted(threadId, processId);
            }},
            {
                // =thread-gorup-exited,id="id"[,exit-code="code"]
                "thread-group-exited", [this](const mi::Response& r) {
                 auto data = r.payload.toMap();
                 auto id = data.value("id").toInt();
                 gdb::Thread threadId;
                 threadId.id = id;
                 auto exitCode = data.value("exit-code").toString();
                 emit threadGroupExited(threadId, exitCode);
            }},
            {
                // =library-loaded,...
                "library-loaded", [this](const mi::Response& r) {
                 auto data = r.payload.toMap();
                 auto id = data.value("id").toInt();
                 auto targetName = data.value("target-name").toString();
                 auto hostName = data.value("host-name").toString();
                 auto symbolsLoaded = data.value("symbols-loaded").toString();
                 auto ranges = data.value("ranges").toMap();
                 auto fromRange = ranges.value("fromRange").toString();
                 auto toRange = ranges.value("toRange").toString();
                 gdb::Library library;
                 library.id = id;
                 library.targetName = targetName;
                 library.hostName = hostName;
                 library.symbolsLoaded = symbolsLoaded;
                 library.ranges.fromRange = fromRange;
                 library.ranges.toRange = toRange;
                 emit libraryLoaded(library);
            }},
            {
                // =library-unloaded,...
                "library-unloaded", [this](const mi::Response& r) {
                 auto data = r.payload.toMap();
                 auto id = data.value("id").toInt();
                 auto targetName = data.value("target-name").toString();
                 auto hostName = data.value("host-name").toString();
                 gdb::Library library;
                 library.id = id;
                 library.targetName = targetName;
                 library.hostName = hostName;
                 emit libraryUnloaded(library);
            }},
        };
        responseDispatcher.value(r.message, [](const mi::Response&){})(r);
        break;
    case mi::Response::result:
        if (r.message == "done" || r.message == "") {
            static const QMap<QString, dispatcher_t> doneDispatcher{
                { "frame", [this](const mi::Response& r) {
                     auto f = gdb::Frame::parseMap(r.payload.toMap().value("frame").toMap());
                     emit updateCurrentFrame(f);
                 }}, // -stack-list-varablbes => Scopes Request
                { "variables", [this](const mi::Response& r) {
                     QList<gdb::Variable> variableList;
                     auto locals = r.payload.toMap().value("variables").toList();
                     for (const auto& e: locals)
                         variableList.append(gdb::Variable::parseMap(e.toMap()));
                     self->variableList = variableList;
                     emit updateLocalVariables(variableList);
                 }}, // -thread-info => Thread Request
                { "threads", [this](const mi::Response& r) {
                     QList<gdb::Thread> threadList;
                     auto data =  r.payload.toMap();
                     auto threads = data.value("threads").toList();
                     auto currId = data.value("current-thread-id").toInt();
                     for (const auto& e: threads)
                         threadList.append(gdb::Thread::parseMap(e.toMap()));
                     self->threadList = threadList;
                     emit updateThreads(currId, threadList);
                 }}, // -stack-list-frames => StackTrace Reqeust
                { "stack", [this](const mi::Response& r) {
                     QList<gdb::Frame> stackFrames;
//                     qInfo() << "stackFrames => " << r.payload.toMap().value("stack").toList();
                     auto stackTrace = r.payload.toMap().value("stack").toList().first().toMap().values("frame");
                     for (const auto& e: stackTrace) {
                         auto frame = gdb::Frame::parseMap(e.toMap());
                         stackFrames.append(frame);
                     }
                     self->stackFrames = stackFrames;
                     emit updateStackFrame(stackFrames);
                 }}, // -break-insert location
                { "bkpt", [this](const mi::Response& r) {
                     auto data = r.payload.toMap();
                     auto bp = gdb::Breakpoint::parseMap(data.value("bkpt").toMap());
                     self->breakpoints.insert(bp.number, bp);
                     emit breakpointInserted(bp);
                }},
            };
            for (const auto& k: r.payload.toMap().keys())
                doneDispatcher.value(k, [](const mi::Response&){})(r);
            if (self->resposeExpected.contains(r.token)) {
                auto& e = self->resposeExpected.value(r.token);
                e.handler(r.payload);
                if (e.action == DebugManager::ResponseAction_t::Temporal)
                    self->resposeExpected.remove(r.token);
            }
        } else if (r.message == "connected") {
            self->m_remote = true;
            emit targetRemoteConnected();
        } else if (r.message == "error") {
            emit gdbError(mi::escapedText(r.payload.toMap().value("msg").toString()));
        } else if (r.message == "exit") {
            self->m_remote = false;
            self->m_firstPromt.store(false);
            emit terminated();
        }
        emit result(r.token, r.message, r.payload);
        break;
    case mi::Response::console:
    case mi::Response::target:
        emit streamConsole(mi::escapedText(r.message));
        break;
    case mi::Response::promt:
        if (self->m_firstPromt.exchange(false))
            emit started();
        emit gdbPromt();
        break;
    case mi::Response::log:
    case mi::Response::unknown:
    default:
        emit streamGdb(mi::escapedText(r.message));
        break;
    }
}

gdb::Frame gdb::Frame::parseMap(const QVariantMap &data)
{
    gdb::Frame f;
    f.level = data.value("level").toInt();
    f.addr = data.value("addr").toString().toULongLong(nullptr, 16);
    f.func = data.value("func").toString();
    f.file = data.value("file").toString();
    auto args = data.value("args").toMap();
    for (auto it = args.cbegin(); it != args.cend(); ++it)
        f.params.insert(it.key(), it.value().toString());
    f.fullpath = data.value("fullname").toString();
    f.line = data.value("line").toInt();
    return f;
}

gdb::Breakpoint gdb::Breakpoint::parseMap(const QVariantMap &data)
{
    gdb::Breakpoint bp;
    bp.number = data.value("number").toInt();
    bp.type = data.value("type").toString();
    bp.disp = strmap({{ "keep", keep }, { "del", del }}, data.value("disp").toString(), keep);
    bp.enable = strmap({{ "y", true }, { "n", false }}, data.value("enable").toString(), true);
    bp.addr = data.value("addr").toString().toULongLong(nullptr, 16);
    bp.func = data.value("func").toString();
    bp.file = data.value("file").toString();
    bp.fullname = data.value("fullname").toString();
    bp.line = data.value("line").toInt();
    bp.threadGroups = data.value("thread-groups").toStringList();
    bp.times = data.value("times").toInt();
    bp.originalLocation = data.value("original-location").toString();
    return bp;
}

gdb::Variable gdb::Variable::parseMap(const QVariantMap &data)
{
    gdb::Variable v;
    v.name = data.value("name").toString();
    v.numChild = data.value("numchild", 0).toInt();
    v.value = data.value("value").toString();
    v.type = data.value("type").toString();
    v.threadId = data.value("thread-id").toString();
    v.hasMore = data.value("has_more", false).toBool();
    v.dynamic = data.value("dynamic", false).toBool();
    v.displayhint = data.value("displayhint").toString();
    return v;
}

gdb::Thread gdb::Thread::parseMap(const QVariantMap &data)
{
    gdb::Thread t;
    t.id = data.value("id").toInt();
    t.targetId = data.value("target-id").toString();
    t.details = data.value("details").toString();
    t.name = data.value("name").toString();
    t.state = strmap({{ "stopped", Stopped }, { "running", Running }}, data.value("state").toString(), Unknown);
    t.frame = Frame::parseMap(data.value("frame").toMap());
    t.core = data.value("core").toInt();
    return t;
}

gdb::AsyncContext::Reason gdb::AsyncContext::textToReason(const QString &s)
{
    static const QMap<QString, Reason> map{
        { "breakpoint-hit"              ,Reason::breakpointHhit          }, // A breakpoint was reached.
        { "watchpoint-trigger"          ,Reason::watchpointTrigger       }, // A watchpoint was triggered.
        { "read-watchpoint-trigger"     ,Reason::readWatchpointTrigger   }, // A read watchpoint was triggered.
        { "access-watchpoint-trigger"   ,Reason::accessWatchpointTrigger }, // An access watchpoint was triggered.
        { "function-finished"           ,Reason::functionFinished        }, // An -exec-finish or similar CLI command was accomplished.
        { "location-reached"            ,Reason::locationReached         }, // An -exec-until or similar CLI command was accomplished.
        { "watchpoint-scope"            ,Reason::watchpointScope         }, // A watchpoint has gone out of scope.
        { "end-stepping-range"          ,Reason::endSteppingRange        }, // An -exec-next, -exec-next-instruction, -exec-step, -exec-step-instruction or similar CLI command was accomplished.
        { "exited-signalled"            ,Reason::exitedSignalled         }, // The inferior exited because of a signal.
        { "exited"                      ,Reason::exited                  }, // The inferior exited.
        { "exited-normally"             ,Reason::exitedNormally          }, // The inferior exited normally.
        { "signal-received"             ,Reason::signalReceived          }, // A signal was received by the inferior.
        { "solib-event"                 ,Reason::solibEvent              }, // The inferior has stopped due to a library being loaded or unloaded. This can happen when stop-on-solib-events (see Files) is set or when a catch load or catch unload catchpoint is in use (see Set Catchpoints).
        { "fork"                        ,Reason::fork                    }, // The inferior has forked. This is reported when catch fork (see Set Catchpoints) has been used.
        { "vfork"                       ,Reason::vfork                   }, // The inferior has vforked. This is reported in when catch vfork (see Set Catchpoints) has been used.
        { "syscall-entry"               ,Reason::syscallEntry            }, // The inferior entered a system call. This is reported when catch syscall (see Set Catchpoints) has been used.
        { "syscall-return"              ,Reason::syscallReturn           }, // The inferior returned from a system call. This is reported when catch syscall (see Set Catchpoints) has been used.
        { "exec"                        ,Reason::exec                    }, // The inferior called exec. This is reported when catch exec (see Set Catchpoints) has been used.
    };
    return map.value(s, Reason::Unknown);
}

QString gdb::AsyncContext::reasonToText(gdb::AsyncContext::Reason r)
{
    switch (r) {
    case Reason::breakpointHhit          : return "breakpoint-hit";
    case Reason::watchpointTrigger       : return "watchpoint-trigger";
    case Reason::readWatchpointTrigger   : return "read-watchpoint-trigger";
    case Reason::accessWatchpointTrigger : return "access-watchpoint-trigger";
    case Reason::functionFinished        : return "function-finished";
    case Reason::locationReached         : return "location-reached";
    case Reason::watchpointScope         : return "watchpoint-scope";
    case Reason::endSteppingRange        : return "end-stepping-range";
    case Reason::exitedSignalled         : return "exited-signalled";
    case Reason::exited                  : return "exited";
    case Reason::exitedNormally          : return "exited-normally";
    case Reason::signalReceived          : return "signal-received";
    case Reason::solibEvent              : return "solib-event";
    case Reason::fork                    : return "fork";
    case Reason::vfork                   : return "vfork";
    case Reason::syscallEntry            : return "syscall-entry";
    case Reason::syscallReturn           : return "syscall-return";
    case Reason::exec                    : return "exec";
    default: return "unknown";
    }
}

gdb::Library gdb::Library::parseMap(const QVariantMap &data)
{
    gdb::Library l;
    l.id = data.value("id").toInt();
    l.targetName = data.value("target-name").toString();
    l.hostName = data.value("host-name").toString();
    l.symbolsLoaded = data.value("symbols-loaded").toString();
    auto ranges = data.value("ranges").toMap();
    l.ranges.fromRange = ranges.value("from").toString();
    l.ranges.toRange = ranges.value("to").toString();
    return l;
}

