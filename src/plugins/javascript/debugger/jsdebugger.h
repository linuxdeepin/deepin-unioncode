// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSDebugger_H
#define JSDebugger_H

#include "base/abstractdebugger.h"

#include <QObject>
#include <QtScript/QScriptEngine>
#include <QScriptEngineDebugger>

class AbstractWidget;
class AbstractCentral;
class JSDebugger : public AbstractDebugger
{
    Q_OBJECT
public:
    explicit JSDebugger(QObject *parent = nullptr);
    ~JSDebugger() override;

    DWidget *getOutputPane() const override;
    DWidget *getStackPane() const override;
    DWidget *getLocalsPane() const override;
    DWidget *getBreakpointPane() const override;

    DWidget *getDebugMainPane() const override;

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

signals:
    void execCommand(QScriptEngineDebugger::DebuggerAction debuggerAction);

public slots:
    void slotEvaluationSuspended();
    void slotEvaluationResumed();
    void setupDebugEnv();

private:
    void runCommand(QScriptEngineDebugger::DebuggerAction command);
    QWidget *debuggerWidget(QScriptEngineDebugger::DebuggerWidget widget) const;
    QScriptValue evaluateFile(QScriptEngine &engine, const QString &filePath);
    void addPagesToContext(const QScriptEngineDebugger &debugger);
    void removePagesFromContext();

    RunState runState = kNoRun;

    QWidget *oldWidgetEdit = nullptr;
    QWidget *oldWidgetWatch = nullptr;

    AbstractWidget *stackPane = nullptr;
    AbstractWidget *breakpointsPane = nullptr;
    AbstractWidget *scriptPane = nullptr;
    AbstractWidget *errorPane = nullptr;
    AbstractWidget *localsPane = nullptr;
    AbstractWidget *codeEditor = nullptr;
};

#endif // JSDebugger_H
