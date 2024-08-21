// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include "debuggerglobals.h"
#include "interface/stackframemodel.h"
#include "interface/localtreemodel.h"
#include "interface/variable.h"
#include "interface/breakpointmodel.h"
#include "dap/dapdebugger.h"
#include "runner/runner.h"
#include "base/abstractdebugger.h"

#include <DFrame>

#include <QTreeView>
#include <QSharedPointer>
#include <QObject>

class AppOutputPane;
class StackFrameView;
class MenuManager;
namespace dpfservice {
    class WindowService;
    class DebuggerService;
}
class DebugManager : public QObject
{
    Q_OBJECT
public:
    explicit DebugManager(QObject *parent = nullptr);
    bool initialize(dpfservice::WindowService *windowService,
                    dpfservice::DebuggerService *debuggerService);
    DWidget *getStackPane() const;
    DWidget *getLocalsPane() const;
    DWidget *getBreakpointPane() const;
    DWidget *getDebugMainPane() const;
    AbstractDebugger::RunState getRunState() const;
    void registerDebugger(const QString &kit, AbstractDebugger *debugger);

signals:
    void debugStarted();
    void debugStopped();

public slots:
    /**
     * UI triggered.
     */
    void remoteDebug(RemoteInfo info);
    void run();
    void attachDebug();
    void detachDebug();

    void interruptDebug();
    void continueDebug();
    void reverseContinue();
    void abortDebug();
    void restartDebug();

    void stepOver();
    void stepIn();
    void stepOut();
    void stepBack();
    bool supportStepBack();

    void handleRunStateChanged(AbstractDebugger::RunState state);
    void handleEvents(const dpf::Event &event);

private:
    bool runCoredump(const QString &target, const QString &core, const QString &kit);
    void rrReplay(const QString &target);

    QMap<QString, AbstractDebugger *> debuggers;
    AbstractDebugger *currentDebugger = nullptr;
    Runner *runner = nullptr;
    QProcess backend;
    QString activeProjectKitName;

    QSharedPointer<MenuManager> menuManager;
};

#endif   // DEBUGMANAGER_H
