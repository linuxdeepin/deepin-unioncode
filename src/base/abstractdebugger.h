// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTDEBUGGER_H
#define ABSTRACTDEBUGGER_H

#include <DWidget>

#include <QObject>

DWIDGET_USE_NAMESPACE

//use to remote debug
struct RemoteInfo {
    QString ip;
    int port = 4711;

    QString executablePath;
    QString projectPath;
};

class AbstractDebugger : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDebugger(QObject *parent = nullptr);

public:
    enum RunState
    {
        kNoRun,
        kPreparing, // e.g. build preparation
        kStart,
        kRunning,
        kStopped,
        kCustomRunning
    };
    virtual ~AbstractDebugger(){}

    virtual DWidget *getOutputPane() const = 0;
    virtual DWidget *getStackPane() const = 0;
    virtual DWidget *getLocalsPane() const = 0;
    virtual DWidget *getBreakpointPane() const = 0;

    virtual DWidget *getDebugMainPane() const = 0;

    virtual void startDebug() = 0;
    virtual void startDebugRemote(const RemoteInfo &info) = 0;
    virtual void attachDebug(const QString &processId) = 0;
    virtual void detachDebug() = 0;

    virtual void interruptDebug() = 0;
    virtual void continueDebug() = 0;
    virtual void abortDebug() = 0;
    virtual void restartDebug() = 0;

    virtual void stepOver() = 0;
    virtual void stepIn() = 0;
    virtual void stepOut() = 0;

    virtual RunState getRunState() const = 0;
    virtual bool runCoredump(const QString &target, const QString &core, const QString &kit) = 0;

signals:
    void runStateChanged(RunState state);
};

#endif // ABSTRACTDEBUGGER_H
