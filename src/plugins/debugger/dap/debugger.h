/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "debug.h"
#include "event/event.h"
#include "interface/stackframemodel.h"
#include "interface/localtreemodel.h"
#include "interface/breakpointmodel.h"

#include <QSharedPointer>
#include <QTreeView>
#include <QPointer>

/**
 * @brief The Debugger class wrap
 */
namespace DEBUG {
class DebugSession;
}

namespace dap {
class Session;
}

class Debugger;
class AppOutputPane;
class StackFrameView;
class RunTimeCfgProvider;
class Debugger : public QObject
{
    Q_OBJECT
public:
    enum RunState
    {
        kNoRun,
        kRunning,
        kStopped,
    };

    explicit Debugger(QObject *parent = nullptr);
    ~Debugger();

    void initializeView();

    AppOutputPane *getOutputPane() const;
    QTreeView *getStackPane() const;
    QTreeView *getLocalsPane() const;
    QTreeView *getBreakpointPane() const;

    void startDebug();
    void detachDebug();

    void interruptDebug();
    void continueDebug();
    void abortDebug();
    void restartDebug();

    void stepOver();
    void stepIn();
    void stepOut();

    RunState getRunState() const;

signals:

public slots:
    void registerDapHandlers();
    void handleFrameEvent(const dpf::Event &event);
    void printOutput(const QString &content, OutputFormat format = NormalMessageFormat);
    /**
     * interface triggered.
     */
    void slotFrameSelected(const QModelIndex &index);
    void slotBreakpointSelected(const QModelIndex &index);
    bool showStoppedBySignalMessageBox(QString meaning, QString name);

private:
    void handleFrames(const StackFrames &stackFrames);

    void addBreakpoint(const QString &filepath, int lineNumber);
    void removeBreakpoint(const QString &filepath, int lineNumber);
    bool getLocals(dap::integer frameId, IVariables *out);
    void exitDebug();
    void updateRunState(Debugger::RunState state);

    QSharedPointer<RunTimeCfgProvider> rtCfgProvider;
    QSharedPointer<DEBUG::DebugSession> session;

    dap::integer threadId = 0;

    bool started = false;

    /**
     * @brief interface objects.
     */
    QSharedPointer<AppOutputPane> outputPane;

    QSharedPointer<StackFrameView> stackView;
    StackFrameModel stackModel;

    QSharedPointer<QTreeView> localsView;
    LocalTreeModel localsModel;

    QSharedPointer<StackFrameView> breakpointView;
    BreakpointModel breakpointModel;

    QPointer<QWidget> alertBox;

    QString targetPath;

    RunState runState = kNoRun;
};

#endif   // DEBUGGER_H
