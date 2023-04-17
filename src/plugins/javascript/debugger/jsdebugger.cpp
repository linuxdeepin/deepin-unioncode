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

#define SAFE_DELETE(p) \
if (p) {               \
    delete p;          \
    p = nullptr;       \
}

using namespace dpfservice;
JSDebugger::JSDebugger(QObject *parent)
    : AbstractDebugger(parent)
{
}

JSDebugger::~JSDebugger()
{

}


QWidget *JSDebugger::getOutputPane() const
{
    return debuggerWidget(QScriptEngineDebugger::DebugOutputWidget);
}

QWidget *JSDebugger::getStackPane() const
{
    return debuggerWidget(QScriptEngineDebugger::StackWidget);
}

QWidget *JSDebugger::getLocalsPane() const
{
    return debuggerWidget(QScriptEngineDebugger::LocalsWidget);
}

QWidget *JSDebugger::getBreakpointPane() const
{
    return debuggerWidget(QScriptEngineDebugger::BreakpointsWidget);
}

void JSDebugger::startDebug()
{
    // initialize debug interface.
    QMetaObject::invokeMethod(this, "setupDebugEnv");
}

void JSDebugger::detachDebug()
{
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
    emit execCommand(command);
}

QWidget *JSDebugger::debuggerWidget(QScriptEngineDebugger::DebuggerWidget widget) const
{
//    return debugger.widget(widget);
    return {};
}

QScriptValue JSDebugger::evaluateFile(QScriptEngine &engine, const QString &filePath)
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

void JSDebugger::addPagesToContext(const QScriptEngineDebugger &debugger)
{
    codeEditor = new AbstractCentral(debugger.widget(QScriptEngineDebugger::CodeWidget));
    stackPane = new AbstractWidget(debugger.widget(QScriptEngineDebugger::StackWidget));
    breakpointsPane = new AbstractWidget(debugger.widget(QScriptEngineDebugger::BreakpointsWidget));
    scriptPane = new AbstractWidget(debugger.widget(QScriptEngineDebugger::ScriptsWidget));
    consolePane = new AbstractWidget(debugger.widget(QScriptEngineDebugger::ConsoleWidget));
    errorPane = new AbstractWidget(debugger.widget(QScriptEngineDebugger::ErrorLogWidget));
    localsPane = new AbstractWidget(debugger.widget(QScriptEngineDebugger::LocalsWidget));
    auto windowService = dpfGetService(WindowService);
    oldWidgetEdit = windowService->setWidgetEdit(codeEditor);
    // instert output pane to window.
    windowService->addContextWidget(tr("Stac&kFrame"), stackPane, "Application");
    oldWidgetWatch = windowService->setWidgetWatch(localsPane);
    debugger.widget(QScriptEngineDebugger::LocalsWidget)->show();
    windowService->addContextWidget(tr("Break&points"), breakpointsPane, "Application");
    windowService->addContextWidget(tr("ScriptWidget"), scriptPane, "Application");
    windowService->addContextWidget(tr("ConsoleWidget"), consolePane, "Application");
    windowService->addContextWidget(tr("ErrorLogWidget"), errorPane, "Application");
}

void JSDebugger::removePagesFromContext()
{
    auto windowService = dpfGetService(WindowService);
    auto removePage = [windowService](AbstractWidget *page){
        Q_ASSERT(page != nullptr);
        windowService->removeContextWidget(page);
    };

    removePage(stackPane);
    removePage(breakpointsPane);
    removePage(scriptPane);
    removePage(consolePane);
    removePage(errorPane);

    windowService->setWidgetEdit(new AbstractCentral(oldWidgetEdit));
    windowService->setWidgetWatch(new AbstractWidget(oldWidgetWatch));
}

void JSDebugger::setupDebugEnv()
{
    // intialize debugger.
    QScriptEngineDebugger debugger;
    connect(this, &JSDebugger::execCommand, [&](QScriptEngineDebugger::DebuggerAction debuggerAction){
        debugger.action(debuggerAction)->trigger();
    });
    debugger.setAutoShowStandardWindow(false);
    connect(&debugger, &QScriptEngineDebugger::evaluationResumed, this, &JSDebugger::slotEvaluationResumed);
    connect(&debugger, &QScriptEngineDebugger::evaluationSuspended, this, &JSDebugger::slotEvaluationSuspended);

    QScriptEngine engine;
    debugger.attachTo(&engine);

    addPagesToContext(debugger);

    // evaluate js files.
    auto prjService = dpfGetService(ProjectService);
    auto sourceFiles = prjService->projectView.getActiveProjectInfo().sourceFiles();

    interruptDebug();
    for (auto sourceFile : sourceFiles) {
        auto value = evaluateFile(engine, sourceFile);
        qInfo() << value.toString();
    }
    removePagesFromContext();

    disconnect(this, &JSDebugger::execCommand, nullptr, nullptr);

    runState = kNoRun;
    emit runStateChanged(kNoRun);
}
