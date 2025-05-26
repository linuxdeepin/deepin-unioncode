// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GDBMI_H
#define GDBMI_H

#include <QMap>
#include <QVariantMap>

namespace gdbmi {
// code is from https://github.com/martinribelotta/gdbfrontend
struct VariableChange {
    QString name;
    bool inScope;
    bool typeChanged;
    bool hasMore;
};

struct Variable {
    QString evaluateName;
    QString name;
    int numChild = 0;
    int64_t childRefrence = 0;
    QString value;
    QString type;
    QString threadId;
    bool hasMore = false;
    bool dynamic = false;
    QString displayhint;

    bool isValid() const { return !name.isEmpty() && !value.isEmpty(); }
    bool haveType() const { return !type.isEmpty(); }
    bool isSimple() const { return isValid() && !haveType(); }

    static Variable *parseMap(const QVariantMap &data);
};

struct Frame {
    int level = -1;
    QString func;
    quint64 addr;
    QHash<QString, QString> params;
    QString file;
    QString fullpath;
    int line;

    bool isValid() const { return level != -1; }

    static Frame parseMap(const QVariantMap &data);
};

struct Breakpoint {
    int number = -1;
    QString type;
    enum Disp_t { keep, del } disp;
    bool enable;
    quint64 addr;
    QString func;
    QString file;
    QString fullname;
    int line;
    QList<QString> threadGroups;
    int times;
    QString originalLocation;

    bool isValid() const { return number != -1; }

    static Breakpoint parseMap(const QVariantMap &data);
};

struct Thread {
    int id;
    QString targetId;
    QString details;
    QString name;
    enum State_t { Unknown, Stopped, Running } state;
    Frame frame;
    int core;

    static Thread parseMap(const QVariantMap &data);
};

struct AsyncContext {
    enum class Reason {
        Unknown,
        breakpointHhit, // breakpoint Hit
        watchpointTrigger,
        readWatchpointTrigger,
        accessWatchpointTrigger,
        functionFinished, // StepOut
        locationReached,
        watchpointScope,
        endSteppingRange, // StepIn
        exitedSignalled,
        exited,
        exitedNormally,
        signalReceived,
        solibEvent,
        fork,
        vfork,
        syscallEntry,
        syscallReturn,
        exec,
    };

    Reason reason;
    QString threadId;
    int core;
    Frame frame;
    QString text;
    QString description;

    static Reason textToReason(const QString &s);
    static QString reasonToText(Reason r);
};

struct Library {
    QString id;
    QString targetName;
    QString hostName;
    QString symbolsLoaded;
    QString threadGroup;
    struct Ranges {
        QString fromRange;
        QString toRange;
    } ranges;

    static Library parseMap(const QVariantMap& data);
};

struct Record{
    enum RecordType {
        unknown,
        notify,
        result,
        console,
        log,
        target,
        promt,
        program,
        disassemble
    };

    RecordType type;
    QString message;
    QVariant payload;
    int token;

    Record(RecordType t, const QString& msg, const QVariant &p, int tk) {
        type = t;
        message= msg;
        payload = p;
        token = tk;
    }

    static Record parseRecord(const QString& outputText);
};

QString escapedText(const QString& s);

} //namespace gdbmi

#endif // GDBMI_H
