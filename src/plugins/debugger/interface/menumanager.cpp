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

    startDebugging.reset(new QAction(MWMDA_START_DEBUG));
    ActionManager::getInstance()->registerAction(startDebugging.get(), "Debug.Start.Debugging",
                                                 MWMDA_START_DEBUG, QKeySequence(Qt::Key::Key_F5),
                                                 "debugger_start.png");
    connect(startDebugging.get(), &QAction::triggered, debugManager, &DebugManager::run);
    AbstractAction *actionImpl = new AbstractAction(startDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Start Debugging", startDebugging.get(), "Debug");
    windowService->addToolBarActionItem("Debugger.Start", startDebugging.get(), "Debug");

#if 0 // not used yet.
    detachDebugger.reset(new QAction("Detach Debugger"));
    connect(detachDebugger.get(), &QAction::triggered, debugManager, &DebugManager::detachDebug);
    actionImpl = new AbstractAction(detachDebugger.get());
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);
#endif

    interrupt.reset(new QAction(MWMDA_INTERRUPT));
    ActionManager::getInstance()->registerAction(interrupt.get(), "Debug.Interrupt",
                                                 MWMDA_INTERRUPT, QKeySequence(Qt::Key::Key_F5),
                                                 "debugger_interrupt.png");
    interrupt->setEnabled(false);
    connect(interrupt.get(), &QAction::triggered, debugManager, &DebugManager::interruptDebug);
    actionImpl = new AbstractAction(interrupt.get());
    interrupt->setEnabled(false);
    windowService->addAction(MWM_DEBUG, actionImpl);

    continueDebugging.reset(new QAction(MWMDA_CONTINUE));
    ActionManager::getInstance()->registerAction(continueDebugging.get(), "Debug.Continue",
                                                 MWMDA_CONTINUE, QKeySequence(Qt::Key::Key_F5),
                                                 "debugger_continue.png");
    continueDebugging->setEnabled(false);
    connect(continueDebugging.get(), &QAction::triggered, debugManager, &DebugManager::continueDebug);
    actionImpl = new AbstractAction(continueDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Debugger.Continue", continueDebugging.get(), "Debug");

    abortDebugging.reset(new QAction(MWMDA_ABORT_DEBUGGING));
    ActionManager::getInstance()->registerAction(abortDebugging.get(), "Debug.Abort.Debugging",
                                                 MWMDA_ABORT_DEBUGGING, QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_Backspace),
                                                 "debugger_stop.png");
    abortDebugging->setEnabled(false);
    connect(abortDebugging.get(), &QAction::triggered, debugManager, &DebugManager::abortDebug);
    actionImpl = new AbstractAction(abortDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("abort_debug", abortDebugging.get(), "Debug");
    windowService->addToolBarActionItem("Debugger.Stop", abortDebugging.get(), "Debug");

    restartDebugging.reset(new QAction(MWMDA_RESTART_DEBUGGING));
    ActionManager::getInstance()->registerAction(restartDebugging.get(), "Debug.Restart.Debugging",
                                                 MWMDA_RESTART_DEBUGGING, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B),
                                                 "restart_debug.png");
    restartDebugging->setEnabled(false);
    connect(restartDebugging.get(), &QAction::triggered, debugManager, &DebugManager::restartDebug);
    actionImpl = new AbstractAction(restartDebugging.get());
    windowService->addAction(MWM_DEBUG, actionImpl);

    stepOver.reset(new QAction(MWMDA_STEP_OVER));
    ActionManager::getInstance()->registerAction(stepOver.get(), "Debug.Step.Over",
                                                 MWMDA_STEP_OVER, QKeySequence(Qt::Key::Key_F10),
                                                 "debugger_stepover.png");
    stepOver->setEnabled(false);
    connect(stepOver.get(), &QAction::triggered, debugManager, &DebugManager::stepOver);
    actionImpl = new AbstractAction(stepOver.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Step.Over", stepOver.get(), "Debug");

    stepIn.reset(new QAction(MWMDA_STEP_IN));
    ActionManager::getInstance()->registerAction(stepIn.get(), "Debug.Step.In",
                                                 MWMDA_STEP_IN, QKeySequence(Qt::Key::Key_F11),
                                                 "debugger_stepinto.png");
    stepIn->setEnabled(false);
    connect(stepIn.get(), &QAction::triggered, debugManager, &DebugManager::stepIn);
    actionImpl = new AbstractAction(stepIn.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Step.In", stepIn.get(), "Debug");

    stepOut.reset(new QAction(MWMDA_STEP_OUT));
    ActionManager::getInstance()->registerAction(stepOut.get(), "Debug.Step.Out",
                                                 MWMDA_STEP_OUT, QKeySequence(Qt::Modifier::SHIFT | Qt::Key::Key_F11),
                                                 "debugger_stepout.png");
    stepOut->setEnabled(false);
    connect(stepOut.get(), &QAction::triggered, debugManager, &DebugManager::stepOut);
    actionImpl = new AbstractAction(stepOut.get());
    windowService->addAction(MWM_DEBUG, actionImpl);
    windowService->addToolBarActionItem("Step.Out", stepOut.get(), "Debug");
    windowService->addToolBarSeparator("Debug");
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
    case Debugger::kCustomRunning:
        startDebugging->setEnabled(false);
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(false);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        break;

    default:
        // do nothing.
        break;
    }
}
