// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTDEBUGGER_H
#define ABSTRACTDEBUGGER_H

#include <QObject>
#include <QScopedPointer>  

#include <DWidget>

DWIDGET_USE_NAMESPACE

// Structure for remote debugging information
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
    explicit AbstractDebugger(QObject* parent = nullptr);
    virtual ~AbstractDebugger() = default;  // Default virtual destructor 

    // Run state enum
    enum RunState {
        kNoRun,
        kPreparing, 
        kStart,
        kRunning,
        kStopped,
        kCustomRunning
    };

    // Virtual methods for accessing debugger panes (return pointers instead of void*)
    virtual DWidget* getOutputPane() const = 0;
    virtual DWidget* getStackPane() const = 0;
    virtual DWidget* getLocalsPane() const = 0;
    virtual DWidget* getBreakpointPane() const = 0;
    virtual DWidget* getDebugMainPane() const = 0;

    // Virtual methods for debugging actions
    virtual void startDebug() = 0;
    virtual void startDebugRemote(const RemoteInfo& info) = 0;
    virtual void detachDebug() = 0;
    virtual void interruptDebug() = 0;
    virtual void continueDebug() = 0;
    virtual void abortDebug() = 0;
    virtual void restartDebug() = 0;
    virtual void stepOver() = 0;
    virtual void stepIn() = 0;
    virtual void stepOut() = 0;

    virtual RunState getRunState() const = 0;
    virtual bool runCoredump(const QString& target, const QString& core, const QString& kit) = 0;

signals:
    void runStateChanged(RunState state);

private:
    QScopedPointer<class AbstractDebuggerPrivate> d; // Use QScopedPointer for automatic memory management
};

#endif // ABSTRACTDEBUGGER_H
