/**
 *  Code is from gdbfront
 *  See on: https://github.com/martinribelotta/gdbfrontend
 *
 *  Copyright (C) 2020 Martin Alejandro Ribelotta <martinribelotta@gmail.com>
 *
 *  Copyright (C) 2022 UnionTech Ltd.
 **/

#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include <QHash>
#include <QObject>

#include <functional>

namespace gdb {

struct VariableChange {
    QString name;
    bool inScope;
    bool typeChanged;
    bool hasMore;
};

struct Variable {
    QString name;
    int numChild = 0;
    QString value;
    QString type;
    QString threadId;
    bool hasMore = false;
    bool dynamic = false;
    QString displayhint;

    bool isValid() const { return !name.isEmpty() && !value.isEmpty(); }
    bool haveType() const { return !type.isEmpty(); }
    bool isSimple() const { return isValid() && !haveType(); }

    static Variable parseMap(const QVariantMap& data);
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

    static Frame parseMap(const QVariantMap& data);
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

    static Breakpoint parseMap(const QVariantMap& data);
};

struct Thread {
    int id;
    QString targetId;
    QString details;
    QString name;
    enum State_t { Unknown, Stopped, Running } state;
    Frame frame;
    int core;

    static Thread parseMap(const QVariantMap& data);
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
    } reason;
    QString threadId;
    int core;
    Frame frame;

    static Reason textToReason(const QString& s);
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

} // namespace gdb

class DebugManager : public QObject
{
    Q_OBJECT

public:
    enum class ResponseAction_t { Permanent, Temporal };
    using ResponseHandler_t = std::function<void (const QVariant& v)>;

    Q_PROPERTY(QString gdbCommand READ gdbCommand WRITE setGdbCommand)
    Q_PROPERTY(bool remote READ isRemote)
    Q_PROPERTY(bool gdbExecuting READ isGdbExecuting)
    Q_PROPERTY(QStringList gdbArgs READ gdbArgs WRITE setGdbArgs)
    Q_PROPERTY(bool inferiorRunning READ isInferiorRunning)

    static DebugManager *instance();
    QStringList gdbArgs() const;
    QString gdbCommand() const;
    bool isRemote() const;
    bool isGdbExecuting() const;

    QList<gdb::Breakpoint> allBreakpoints() const;
    QList<gdb::Breakpoint> breakpointsForFile(const QString& filePath) const;
    gdb::Breakpoint breakpointById(int id) const;
    gdb::Breakpoint breakpointByFileLine(const QString& path, int line) const;
    bool isInferiorRunning() const;
    qint64 getProcessId();

public slots:
    void execute();
    void quit();

    void command(const QString& cmd);
    void commandAndResponse(const QString& cmd,
                            const ResponseHandler_t& handler,
                            ResponseAction_t action = ResponseAction_t::Temporal);
    void breakAfter(int bpid, int count);
    void breakCondition(const int bpid, const QString &expr);
    void breakDisable(int bpid);
    void breakEnable(int bpid);
    void breakInfo(int bpid);
    void breakList();
    void breakRemove(int bpid);
    void breakInsert(const QString& path);

    void loadExecutable(const QString& file);
    void launchRemote(const QString& remoteTarget);
    void launchLocal();
    void attachProcess(const int pid);
    void attachThreadGroup(const QString& gid);
    void detach();
    void detachProcess(const int pid);
    void detachThreadGroup(const QString& gid);
    void disconnect();

    void enableFrameFilters();
    void stackListFrames();
    void stackListLocals();
    void stackListVariables();
    void stackListFrame(const gdb::Frame& frame);

    void commandPause();
    void commandContinue();
    void commandContinueReverse();
    void commandNext();
    void commandNextReverse();
    void commandNexti();
    void commandNextiReverse();
    void commandStep();
    void commandStepReverse();
    void commandStepi();
    void commandStepiReverse();
    void commandReturn();
    void commandFinish();
    void commandFinishReverse();
    void commandInterrupt();
    void commandUntil(const QString& location);
    void commandJump(const QString& location);

    void threadInfo();
    void threadListIds();
    void threadSelect(const gdb::Thread& thread);

    void listSourceFiles();

    void traceAddVariable(const QString& expr, const QString& name="-", int frame=-1);
    void traceDelVariable(const QString& name);
    void traceUpdateVariable(const QString& name);
    void traceUpdateAll() { traceUpdateVariable("*"); }

    void setGdbCommand(QString gdbCommand);
    void setGdbArgs(QStringList gdbArgs);

    const QMap<QString, gdb::Variable> &vatchVars() const;

signals:
    void gdbProcessStarted();
    void gdbProcessTerminated();

    void started();
    void terminated();
    void gdbPromt();
    void targetRemoteConnected();
    void gdbError(const QString& msg);

    void asyncRunning(const QString& thid);
    void asyncStopped(const gdb::AsyncContext& ctx);
    void threadGroupAdded(const gdb::Thread& thid);
    void threadGroupRemoved(const gdb::Thread& thid);
    void threadGroupStarted(const gdb::Thread& thid, const gdb::Thread& pid);
    void threadGroupExited(const gdb::Thread& thid, const QString& exitCode);
    void threadCreated(const gdb::Thread& thid, const QString& groupId);
    void threadExited(const gdb::Thread& thid, const QString& groupId);
    void threadSelected(const gdb::Thread& thid, const gdb::Frame& frame);
    void updateThreads(int currentId, const QList<gdb::Thread>& threads);
    void libraryLoaded(const gdb::Library& library);
    void libraryUnloaded(const gdb::Library& library);

    void updateCurrentFrame(const gdb::Frame& frame);
    void updateStackFrame(const QList<gdb::Frame>& stackFrames);
    void updateLocalVariables(const QList<gdb::Variable>& variableList);

    void breakpointInfo(const gdb::Breakpoint& bp);
    void breakpointInserted(const gdb::Breakpoint& bp);
    void breakpointModified(const gdb::Breakpoint& bp);
    void breakpointRemoved(const gdb::Breakpoint& bp);

    void variableCreated(const gdb::Variable& v);
    void variableDeleted(const gdb::Variable& v);
    void variablesChanged(const QStringList& changedNames);

    void result(int token, const QString& reason, const QVariant& results); // <token>^...
    void streamConsole(const QString& text);
    void streamTarget(const QString& text);
    void streamGdb(const QString& text);
    void streamDebugInternal(const QString& text);

private slots:
    void processLine(const QString& line);

private:
    explicit DebugManager(QObject *parent = nullptr);
    virtual ~DebugManager();

    struct Priv_t;
    Priv_t *self;
};

Q_DECLARE_METATYPE(gdb::Variable)
Q_DECLARE_METATYPE(gdb::Frame)
Q_DECLARE_METATYPE(gdb::Breakpoint)
Q_DECLARE_METATYPE(gdb::Thread)
Q_DECLARE_METATYPE(gdb::AsyncContext)
Q_DECLARE_METATYPE(gdb::Library)

#endif // DEBUGMANAGER_H
