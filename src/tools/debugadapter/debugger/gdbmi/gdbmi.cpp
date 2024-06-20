// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gdbmi.h"

#include <QRegularExpression>

namespace gdbmi {
// code is from https://github.com/martinribelotta/gdbfrontend
// https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI.html#GDB_002fMI

// GDB/MI Result Records: ^done, ^running, ^connected, ^error, exit
const auto GDB_MI_RESULT = QRegularExpression(R"(^(\d*)\^(\S+?)(?:,(.*))?$)",
                                              QRegularExpression::MultilineOption |
                                              QRegularExpression::NoPatternOption);

// GDB/MI Stream Records: "~" string-output "@" string-output "&" string-output
const auto GDB_MI_CONSOLE = QRegularExpression(R"re(~"(.*)")re",
                                               QRegularExpression::MultilineOption |
                                               QRegularExpression::DotMatchesEverythingOption);
const auto GDB_MI_TARGET = QRegularExpression(R"re(@"(.*)")re",
                                              QRegularExpression::MultilineOption |
                                              QRegularExpression::DotMatchesEverythingOption);
const auto GDB_MI_LOG = QRegularExpression(R"re(&"(.*)")re",
                                           QRegularExpression::MultilineOption |
                                           QRegularExpression::DotMatchesEverythingOption);

// GDB/MI Async Records:a consequence of commands (e.g., a breakpoint modified) or a result of target activity (e.g., target stopped).
const auto GDB_MI_NOTIFY = QRegularExpression(R"(^(\d*)[*=](\S+?),(.*)$)",
                                              QRegularExpression::MultilineOption |
                                              QRegularExpression::NoPatternOption);
// GDB/MI Finished
const auto GDB_MI_FINISHED = QRegularExpression(R"(^\(gdb\)\s*$)",
                                              QRegularExpression::MultilineOption |
                                                QRegularExpression::NoPatternOption);

const QString CLOSE_CHARS{"}]\""};

const auto DISASSEMBLE_DATA_REG = QRegularExpression(R"(~\".+0x.+<\+.+>:.+\")",
                                                QRegularExpression::MultilineOption |
                                                QRegularExpression::NoPatternOption);
const auto DISASSEMBLE_END_REG = QRegularExpression(R"(~\"End of assembler dump.)",
                                                QRegularExpression::NoPatternOption);

QString escapedText(const QString& s)
{
    return QString{s}.replace(QRegularExpression{R"(\\(.))"}, "\1");
}

template<typename T>
static T strmap(const QMap<QString, T>& map, const QString& key, T defaultValue)
{
    return map.value(key, defaultValue);
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
        if (it >= str.cend())
            break;
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
    if (CLOSE_CHARS.contains(*it))
        return {};
    return parseKeyVal(str, it, terminator);
}

} //namespace prvi

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

gdbmi::Variable *gdbmi::Variable::parseMap(const QVariantMap &data)
{
    gdbmi::Variable *v = new gdbmi::Variable;
    v->evaluateName = data.value("name").toString();
    v->name = data.value("name").toString();
    v->numChild = data.value("numchild", 0).toInt();
    v->value = QString::fromUtf8(data.value("value").toString().toUtf8());
    v->type = data.value("type").toString();
    v->threadId = data.value("thread-id").toString();
    v->hasMore = data.value("has_more", false).toBool();
    v->dynamic = data.value("dynamic", false).toBool();
    v->displayhint = data.value("displayhint").toString();
    return v;
}

gdbmi::Frame gdbmi::Frame::parseMap(const QVariantMap &data)
{
    gdbmi::Frame f;
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

gdbmi::Breakpoint gdbmi::Breakpoint::parseMap(const QVariantMap &data)
{
    gdbmi::Breakpoint bp;
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

gdbmi::Thread gdbmi::Thread::parseMap(const QVariantMap &data)
{
    gdbmi::Thread t;
    t.id = data.value("id").toInt();
    t.targetId = data.value("target-id").toString();
    t.details = data.value("details").toString();
    t.name = data.value("name").toString();
    t.state = strmap({{ "stopped", Stopped }, { "running", Running }}, data.value("state").toString(), Unknown);
    t.frame = Frame::parseMap(data.value("frame").toMap());
    t.core = data.value("core").toInt();
    return t;
}

gdbmi::AsyncContext::Reason gdbmi::AsyncContext::textToReason(const QString &s)
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

QString gdbmi::AsyncContext::reasonToText(gdbmi::AsyncContext::Reason r)
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

gdbmi::Library gdbmi::Library::parseMap(const QVariantMap& data) {
    gdbmi::Library l;
    l.id = data.value("id").toString();
    l.targetName = data.value("target-name").toString();
    l.hostName = data.value("host-name").toString();
    l.symbolsLoaded = data.value("symbols-loaded").toString();
    auto ranges = data.value("ranges").toMap();
    l.ranges.fromRange = ranges.value("from").toString();
    l.ranges.toRange = ranges.value("to").toString();
    return l;
}

gdbmi::Record gdbmi::Record::parseRecord(const QString& outputText)
{
    QRegularExpressionMatch regMatch;
    if ((regMatch = DISASSEMBLE_DATA_REG.match(outputText)).hasMatch()) {
        return Record(RecordType::disassemble,
                      "data",
                      outputText,
                      -1);
    } else if ((regMatch = DISASSEMBLE_END_REG.match(outputText)).hasMatch()) {
        return Record(RecordType::disassemble,
                      "end",
                      QVariant(),
                      -1);
    } else if ((regMatch = GDB_MI_NOTIFY.match(outputText)).hasMatch()) {
        return Record(RecordType::notify,
                      regMatch.captured(2),
                      parseElements(regMatch.captured(3)),
                      strToInt(regMatch.captured(1), -1));
    } else if ((regMatch = GDB_MI_RESULT.match(outputText)).hasMatch()) {
        return Record(RecordType::result,
                      regMatch.captured(2),
                      parseElements(regMatch.captured(3)),
                      strToInt(regMatch.captured(1), -1));
    } else if ((regMatch = GDB_MI_CONSOLE.match(outputText)).hasMatch()) {
        return Record(RecordType::console,
                      outputText,
                      outputText,
                      -1);
    } else if ((regMatch = GDB_MI_LOG.match(outputText)).hasMatch()) {
        return Record(RecordType::log,
                      regMatch.captured(1),
                      regMatch.captured(0),
                      -1);
    } else if ((regMatch = GDB_MI_TARGET.match(outputText)).hasMatch()) {
        return Record(RecordType::target,
                      regMatch.captured(1),
                      regMatch.captured(0),
                      -1);
    } else if ((regMatch = GDB_MI_FINISHED.match(outputText)).hasMatch()) {
        return Record(RecordType::promt, {}, {}, -1);
    } else {
        return Record(RecordType::program, {}, outputText, -1);
    }
}

} //namespace gdbmi

