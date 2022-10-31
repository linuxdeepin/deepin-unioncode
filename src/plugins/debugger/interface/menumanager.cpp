/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             hongjinchuan<hongjinchuan@uniontech.com>
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
#include "menumanager.h"
#include "debugmanager.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "base/abstractmenu.h"
#include "common/common.h"
#include "services/window/windowservice.h"

#include <QMenu>

using namespace dpfservice;
MenuManager::MenuManager(QObject *parent) : QObject(parent)
{
}

void MenuManager::initialize(WindowService *windowService)
{
    if (!windowService)
        return;

    startDebugging.reset(new QAction("Start Debugging"));
    ActionManager::getInstance()->registerAction(startDebugging.get(), "Debug.Start.Debugging",
                                                 "Start Debugging", QKeySequence(Qt::Key::Key_F5),
                                                 "debug-start.png");
    connect(startDebugging.get(), &QAction::triggered, debugManager, &DebugManager::run);
    AbstractAction *actionImpl = new AbstractAction(startDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Start Debugging", startDebugging.get());

#if 0 // not used yet.
    detachDebugger.reset(new QAction("Detach Debugger"));
    connect(detachDebugger.get(), &QAction::triggered, debugManager, &DebugManager::detachDebug);
    actionImpl = new AbstractAction(detachDebugger.get());
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);
#endif

    interrupt.reset(new QAction("Interrupt"));
    ActionManager::getInstance()->registerAction(interrupt.get(), "Debug.Interrupt",
                                                 "Interrupt", QKeySequence(Qt::Key::Key_F5),
                                                 "interrupt.png");
    interrupt->setEnabled(false);
    connect(interrupt.get(), &QAction::triggered, debugManager, &DebugManager::interruptDebug);
    actionImpl = new AbstractAction(interrupt.get());
    interrupt->setEnabled(false);
    windowService->addAction(MWM_DEBUG, actionImpl);

    continueDebugging.reset(new QAction("Continue"));
    ActionManager::getInstance()->registerAction(continueDebugging.get(), "Debug.Continue",
                                                 "Continue", QKeySequence(Qt::Key::Key_F5),
                                                 "continue.png");
    continueDebugging->setEnabled(false);
    connect(continueDebugging.get(), &QAction::triggered, debugManager, &DebugManager::continueDebug);
    actionImpl = new AbstractAction(continueDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);

    abortDebugging.reset(new QAction("Abort Debugging"));
    ActionManager::getInstance()->registerAction(abortDebugging.get(), "Debug.Abort.Debugging",
                                                 "Abort Debugging", QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_Backspace),
                                                 "abort_debug.png");
    abortDebugging->setEnabled(false);
    connect(abortDebugging.get(), &QAction::triggered, debugManager, &DebugManager::abortDebug);
    actionImpl = new AbstractAction(abortDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);

    restartDebugging.reset(new QAction("Restart Debugging"));
    ActionManager::getInstance()->registerAction(restartDebugging.get(), "Debug.Restart.Debugging",
                                                 "Restart Debugging", QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B),
                                                 "restart_debug.png");
    restartDebugging->setEnabled(false);
    connect(restartDebugging.get(), &QAction::triggered, debugManager, &DebugManager::restartDebug);
    actionImpl = new AbstractAction(restartDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);

    stepOver.reset(new QAction("Step Over"));
    ActionManager::getInstance()->registerAction(stepOver.get(), "Debug.Step.Over",
                                                 "Step Over", QKeySequence(Qt::Key::Key_F10),
                                                 "step_over.png");
    stepOver->setEnabled(false);
    connect(stepOver.get(), &QAction::triggered, debugManager, &DebugManager::stepOver);
    actionImpl = new AbstractAction(stepOver.get());
    windowService->addAction(MWM_DEBUG, actionImpl);

    stepIn.reset(new QAction("Step In"));
    ActionManager::getInstance()->registerAction(stepIn.get(), "Debug.Step.In",
                                                 "Step In", QKeySequence(Qt::Key::Key_F11),
                                                 "step_in.png");
    stepIn->setEnabled(false);
    connect(stepIn.get(), &QAction::triggered, debugManager, &DebugManager::stepIn);
    actionImpl = new AbstractAction(stepIn.get());
    windowService->addAction(MWM_DEBUG, actionImpl);

    stepOut.reset(new QAction("Step Out"));
    ActionManager::getInstance()->registerAction(stepOut.get(), "Debug.Step.Out",
                                                 "Step Out", QKeySequence(Qt::Modifier::SHIFT | Qt::Key::Key_F11),
                                                 "step_out.png");
    stepOut->setEnabled(false);
    connect(stepOut.get(), &QAction::triggered, debugManager, &DebugManager::stepOut);
    actionImpl = new AbstractAction(stepOut.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
}

void MenuManager::handleRunStateChanged(Debugger::RunState state)
{
    switch (state) {
    case Debugger::kNoRun:
    case Debugger::kPreparing:
    case Debugger::kStart:
        startDebugging->setEnabled(true);
#if 0 // not used yet.
        detachDebugger->setEnabled(true);
#endif
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(false);
        restartDebugging->setEnabled(false);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        break;

    case Debugger::kRunning:
        startDebugging->setEnabled(false);
#if 0 // not used yet.
        detachDebugger->setEnabled(false);
#endif
        interrupt->setEnabled(true);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(true);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        break;
    case Debugger::kStopped:
        startDebugging->setEnabled(false);
#if 0 // not used yet.
        detachDebugger->setEnabled(false);
#endif
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(true);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(true);
        stepOver->setEnabled(true);
        stepIn->setEnabled(true);
        stepOut->setEnabled(true);
        break;
    default:
        // do nothing.
        break;
    }
}
