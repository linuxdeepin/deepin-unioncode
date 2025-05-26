// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "debug.h"
#include "interface/stackframemodel.h"
#include "interface/localtreemodel.h"
#include "interface/localtreeitem.h"
#include "interface/breakpointmodel.h"
#include "base/abstractdebugger.h"

#include "services/project/projectservice.h"
#include "common/supportfile/dapconfig.h"
#include "common/widget/outputpane.h"

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

class DebuggerPrivate;
class DAPDebugger;
class OutputPane;
class StackFrameView;
class RunTimeCfgProvider;
class QComboBox;
class DAPDebugger : public AbstractDebugger
{
    Q_OBJECT
public:
    explicit DAPDebugger(QObject *parent = nullptr);
    ~DAPDebugger() override;

    enum debugState {
        Normal,
        Attaching,
        Reverse
    };

    DWidget *getOutputPane() const override;
    DWidget *getStackPane() const override;
    DWidget *getLocalsPane() const override;
    DWidget *getBreakpointPane() const override;
    DWidget *getDebugMainPane() const override;

    void startDebug() override;
    void startDebugRemote(const RemoteInfo &info) override;
    void startRerverseDebug(const QString &target);
    void attachDebug(const QString &processId) override;
    void detachDebug() override;

    void interruptDebug() override;
    void continueDebug() override;
    void reverseContinue() override;
    void abortDebug() override;
    void restartDebug() override;

    void stepOver() override;
    void stepIn() override;
    void stepOut() override;
    void stepBack() override;

    bool supportStepBack() override;
    RunState getRunState() const override;
    bool runCoredump(const QString &target, const QString &core, const QString &kit) override;

signals:
    void childVariablesUpdated(LocalTreeItem* parentItem, IVariables vars);
    void processingVariablesDone();

public slots:
    void registerDapHandlers();
    void handleEvents(const dpf::Event &event);
    void printOutput(const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);
    void synPrintOutput(const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);
    /**
     * interface triggered.
     */
    void slotFrameSelected();
    void slotBreakpointSelected(const QModelIndex &index);
    bool showStoppedBySignalMessageBox(QString meaning, QString name);
    void currentThreadChanged(const QString &text);

    void slotReceivedDAPPort(const QString &uuid, int port, const QString &kitName, const QMap<QString, QVariant> &param);
    void slotOutputMsg(const QString &title, const QString &msg);
    void slotGetChildVariable(const QModelIndex &index);
    void slotEvaluateWatchVariable();
    void slotRemoveEvaluator();

private:
    void launchBackend();
    void initializeView();
    void initializeVairablesPane();
    void updateWatchingVariables();
    void handleFrames(const StackFrames &stackFrames);
    void handleUpdateDebugLine();
    void updateThreadList(int curr, const dap::array<dap::Thread> &threads);
    void switchCurrentThread(int curThreadID);
    QHBoxLayout *initFrameTitle(const QString &frameName);

    void addBreakpoint(const QString &filepath, int lineNumber);
    void removeBreakpoint(const QString &filepath, int lineNumber);
    void removeAllBreakpoints();
    void switchBreakpointsStatus(const QString &filePath, int lineNumber, bool enabled);
    void setBreakpointCondition(const QString &filePath, int lineNumber, const QString &expression);
    void jumpToLine(const QString &filePath, int lineNumber);
    void runToLine(const QString &filePath, int lineNumber);
    void evaluateWatchVariable(const QString &expression);

    bool getLocals(dap::integer frameId, IVariables *out);
    void exitDebug();
    void updateRunState(DAPDebugger::RunState state);
    QString requestBuild();
    void start();
    void prepareDebug();
    bool requestDebugPort(const QMap<QString, QVariant> &param, const QString &kitName, bool customDap);
    void stopWaitingDebugPort();
    void stopDAP();
    void launchSession(int port, const QMap<QString, QVariant> &param, const QString &kitName);
    void disassemble(const QString &address);
    void handleAssemble(const QString &content);

    QString transformRemotePath(const QString &remotePath);
    
    dpfservice::ProjectInfo getActiveProjectInfo() const;

    DebuggerPrivate *const d;
};

#endif   // DEBUGGER_H
