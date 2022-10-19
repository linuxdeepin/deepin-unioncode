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

#include "services/project/projectservice.h"
#include "common/supportfile/dapconfig.h"

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
class QComboBox;
class Debugger : public QObject
{
    Q_OBJECT
public:
    enum RunState
    {
        kNoRun,
        kPreparing, // e.g. build preparation
        kStart,
        kRunning,
        kStopped,
    };

    explicit Debugger(QObject *parent = nullptr);
    ~Debugger();

    AppOutputPane *getOutputPane() const;
    QWidget *getStackPane() const;
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
    void runStateChanged(RunState state);
    void sigJavaLSPPluginReady(bool succeed);
    void sigJavaDAPPluginReady(bool succeed);
    void sigJavaDAPPort(int port, const QString &mainClass, const QString &projectName, const QStringList &classPaths);
    void sigPythonDAPPort(int port);
    void sigStartDebugPython();

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
    void message(QString msg);
    void currentThreadChanged(const QString &text);

    void slotJavaLSPPluginReady(bool succeed);
    void slotJavaDAPPluginReady(bool succeed);
    void slotReceiveJavaDAPPort(int port, const QString &mainClass, const QString &projectName, const QStringList &classPaths);
    void slotHandleJavaDAPPort(int port, const QString &mainClass, const QString &projectName, const QStringList &classPaths);

    void slotReceivePythonDAPPort(int port);
    void slotHandlePythonDAPPort(int port);

    void slotStartDebugPython();
private:
    void initializeView();
    void handleFrames(const StackFrames &stackFrames);
    void updateThreadList(int curr, const dap::array<dap::Thread> &threads);
    void switchCurrentThread(int curThreadID);

    void addBreakpoint(const QString &filepath, int lineNumber);
    void removeBreakpoint(const QString &filepath, int lineNumber);
    bool getLocals(dap::integer frameId, IVariables *out);
    void exitDebug();
    void updateRunState(Debugger::RunState state);
    bool checkTargetIsReady();
    void requestBuild();
    void start();

    bool isCMakeProject(const QString &kitName);
    bool isMavenProject(const QString &kitName);
    bool isGradleProject(const QString &kitName);
    bool isJavaProject(const QString &kitName);
    bool isPythonProject(const QString &kitName);

    void launchSession(const int port, const QString &mainClass = "",
                       const QString &projectName = "", const QStringList &classPaths = QStringList{});
    void stopDAP();
    void checkJavaLSPPlugin();
    void checkJavaDAPPlugin();

    QSharedPointer<RunTimeCfgProvider> rtCfgProvider;
    QSharedPointer<DEBUG::DebugSession> session;

    dap::integer threadId = 0;

    /**
     * @brief interface objects.
     */
    AppOutputPane *outputPane = nullptr;

    QWidget *stackPane = nullptr;
    StackFrameView *stackView = nullptr;
    StackFrameModel stackModel;
    QComboBox *threadSelector = nullptr;

    QTreeView *localsView = nullptr;
    LocalTreeModel localsModel;

    StackFrameView *breakpointView = nullptr;
    BreakpointModel breakpointModel;

    QPointer<QWidget> alertBox;

    QString targetPath;

    RunState runState = kNoRun;

    QString activeProjectKitName;
    dpfservice::ProjectInfo projectInfo;
    QString currentOpenedFileName;

    QString currentBuildUuid;
    QTimer *timer = nullptr;
    std::atomic_bool waitHandleJavaDAPPort = false;
    support_file::JavaDapPluginConfig javaDapPluginConfig;
    std::atomic_bool javaDapPluginFileReady = false;

    std::atomic_bool waitHandlePythonDAPPort = false;
};

#endif   // DEBUGGER_H
