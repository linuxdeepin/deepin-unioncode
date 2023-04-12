/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "jsdebugger.h"

#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QAction>

using namespace dpfservice;
JSDebugger::JSDebugger(QObject *parent)
    : AbstractDebugger(parent)
{
    connect(&debugger, &QScriptEngineDebugger::evaluationResumed, this, &JSDebugger::slotEvaluationResumed);
    connect(&debugger, &QScriptEngineDebugger::evaluationSuspended, this, &JSDebugger::slotEvaluationSuspended);

    debugger.setAutoShowStandardWindow(false);
}

QWidget *JSDebugger::getOutputPane() const
{
    return getWidget(QScriptEngineDebugger::DebugOutputWidget);
}

QWidget *JSDebugger::getStackPane() const
{
    return getWidget(QScriptEngineDebugger::StackWidget);
}

QWidget *JSDebugger::getLocalsPane() const
{
    return getWidget(QScriptEngineDebugger::LocalsWidget);
}

QWidget *JSDebugger::getBreakpointPane() const
{
    return getWidget(QScriptEngineDebugger::BreakpointsWidget);
}

void JSDebugger::startDebug()
{
    // initialize debug interface.
    QMetaObject::invokeMethod(this, "setupDebugEnv");
}

void JSDebugger::detachDebug()
{
    debugger.detach();
}

void JSDebugger::interruptDebug()
{
    runCommand(QScriptEngineDebugger::InterruptAction);
    runState = kStopped;
}

void JSDebugger::continueDebug()
{
    runCommand(QScriptEngineDebugger::ContinueAction);
    runState = kRunning;
    emit runStateChanged(kRunning);
}

void JSDebugger::abortDebug()
{
}

void JSDebugger::restartDebug()
{
    abortDebug();
    startDebug();
}

void JSDebugger::stepOver()
{
    runCommand(QScriptEngineDebugger::StepOverAction);
}

void JSDebugger::stepIn()
{
    runCommand(QScriptEngineDebugger::StepIntoAction);
}

void JSDebugger::stepOut()
{
    runCommand(QScriptEngineDebugger::StepOverAction);
}

AbstractDebugger::RunState JSDebugger::getRunState() const
{
    return runState;
}

bool JSDebugger::runCoredump(const QString &target, const QString &core, const QString &kit)
{
    Q_UNUSED(target)
    Q_UNUSED(core)
    Q_UNUSED(kit)

    // no need this function.
    return false;
}

void JSDebugger::slotEvaluationSuspended()
{
    runState = kStopped;
    emit runStateChanged(kStopped);
}

void JSDebugger::slotEvaluationResumed()
{

}

void JSDebugger::runCommand(QScriptEngineDebugger::DebuggerAction command)
{
    debugger.action(command)->trigger();
}

QWidget *JSDebugger::getWidget(QScriptEngineDebugger::DebuggerWidget widget) const
{
    return debugger.widget(widget);
}

QScriptValue JSDebugger::evaluateFile(const QString &filePath)
{
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    QString byteArray = file.readAll();
    file.close();

    // check syntax.
    QScriptSyntaxCheckResult syntaxCheck = engine.checkSyntax(byteArray);
    if (syntaxCheck.state() != QScriptSyntaxCheckResult::Valid) {
        qInfo() << syntaxCheck.errorMessage() + " at line " + QString::number(syntaxCheck.errorLineNumber());
        return {};
    }
    return engine.evaluate(byteArray, filePath);
}

void JSDebugger::setupDebugEnv()
{
    debugger.attachTo(&engine);

    auto windowService = dpfGetService(WindowService);
    oldWidgetEdit = windowService->setWidgetEdit(new AbstractCentral(debugger.widget(QScriptEngineDebugger::CodeWidget)));

    // instert output pane to window.
    windowService->addContextWidget(tr("Stac&kFrame"), new AbstractWidget(getStackPane()), "Application");
    oldWidgetWatch = windowService->setWidgetWatch(new AbstractWidget(getLocalsPane()));
    getLocalsPane()->show();
    windowService->addContextWidget(tr("Break&points"), new AbstractWidget(getBreakpointPane()), "Application");
    windowService->addContextWidget(tr("ScriptWidget"), new AbstractWidget(debugger.widget(QScriptEngineDebugger::ScriptsWidget)), "Application");
    windowService->addContextWidget(tr("ConsoleWidget"), new AbstractWidget(debugger.widget(QScriptEngineDebugger::ConsoleWidget)), "Application");
    windowService->addContextWidget(tr("ErrorLogWidget"), new AbstractWidget(debugger.widget(QScriptEngineDebugger::ErrorLogWidget)), "Application");

    auto prjService = dpfGetService(ProjectService);
    auto sourceFiles = prjService->projectView.getActiveProjectInfo().sourceFiles();

    runState = kStopped;

    interruptDebug();
    for (auto sourceFile : sourceFiles) {
        evaluateFile(sourceFile);
    }

    runState = kNoRun;
    emit runStateChanged(kNoRun);

    windowService->setWidgetEdit(new AbstractCentral(oldWidgetEdit));
    windowService->setWidgetWatch(new AbstractWidget(oldWidgetWatch));
}
