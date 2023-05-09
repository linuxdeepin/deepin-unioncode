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

    QWidget *getOutputPane() const override;
    QWidget *getStackPane() const override;
    QWidget *getLocalsPane() const override;
    QWidget *getBreakpointPane() const override;

    void startDebug() override;
    void detachDebug() override;

    void interruptDebug() override;
    void continueDebug() override;
    void abortDebug() override;
    void restartDebug() override;

    void stepOver() override;
    void stepIn() override;
    void stepOut() override;

    RunState getRunState() const override;
    bool runCoredump(const QString &target, const QString &core, const QString &kit) override;

public slots:
    void registerDapHandlers();
    void handleEvents(const dpf::Event &event);
    void printOutput(const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);
    void synPrintOutput(const QString &content, OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage);
    /**
     * interface triggered.
     */
    void slotFrameSelected(const QModelIndex &index);
    void slotBreakpointSelected(const QModelIndex &index);
    bool showStoppedBySignalMessageBox(QString meaning, QString name);
    void currentThreadChanged(const QString &text);

    void slotReceivedDAPPort(const QString &uuid, int port, const QString &kitName, const QMap<QString, QVariant> &param);
    void slotOutputMsg(const QString &title, const QString &msg);

private:
    void launchBackend();
    void killBackend();
    void initializeView();
    void handleFrames(const StackFrames &stackFrames);
    void updateThreadList(int curr, const dap::array<dap::Thread> &threads);
    void switchCurrentThread(int curThreadID);

    void addBreakpoint(const QString &filepath, int lineNumber);
    void removeBreakpoint(const QString &filepath, int lineNumber);
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
    dpfservice::ProjectInfo getActiveProjectInfo() const;

    DebuggerPrivate *const d;
};

#endif   // DEBUGGER_H
