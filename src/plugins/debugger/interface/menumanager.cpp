// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "menumanager.h"
#include "debugmanager.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "base/abstractmenu.h"
#include "common/common.h"
#include "common/actionmanager/actioncontainer.h"
#include "common/widget/appoutputpane.h"
#include "services/window/windowservice.h"
#include "remotedebug/remotedebugdlg.h"

#include <QMenu>

using namespace dpfservice;
MenuManager::MenuManager(QObject *parent)
    : QObject(parent)
{
}

void MenuManager::initialize(WindowService *windowService)
{
    if (!windowService)
        return;

    auto initAction = [&](QAction *action, const QString &id, const QString &description,
                          QKeySequence key, const QString &iconName = {}) -> Command * {
        action->setIcon(QIcon::fromTheme(iconName));
        auto cmd = ActionManager::instance()->registerAction(action, id);
        cmd->setDescription(description);
        if (!key.isEmpty())
            cmd->setDefaultKeySequence(key);
        return cmd;
    };

    auto mDebug = ActionManager::instance()->actionContainer(M_DEBUG);

    startDebugging.reset(new QAction(MWMDA_START_DEBUG));
    connect(startDebugging.get(), &QAction::triggered, debugManager, &DebugManager::run);
    auto actionImpl = initAction(startDebugging.get(), "Debug.Start.Debugging",
                                 MWMDA_START_DEBUG, QKeySequence(Qt::Key::Key_F5),
                                 "debugger_start");
    mDebug->addAction(actionImpl);
    windowService->addTopToolItem(actionImpl, false, Priority::medium);
#if 0   // not used yet.
    detachDebugger.reset(new QAction("Detach Debugger"));
    connect(detachDebugger.get(), &QAction::triggered, debugManager, &DebugManager::detachDebug);
    actionImpl = new AbstractAction(detachDebugger.get());
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);
#endif
    //register action to appoutputpane
    auto appOutPutPane = AppOutputPane::instance();

    interrupt.reset(new QAction(MWMDA_INTERRUPT));
    interrupt->setEnabled(false);
    connect(interrupt.get(), &QAction::triggered, debugManager, &DebugManager::interruptDebug);

    actionImpl = initAction(interrupt.get(), "Debug.Interrupt",
                            MWMDA_INTERRUPT, QKeySequence(Qt::Key::Key_F5),
                            "debugger_interrupt");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), true);

    continueDebugging.reset(new QAction(MWMDA_CONTINUE));
    continueDebugging->setEnabled(false);
    connect(continueDebugging.get(), &QAction::triggered, debugManager, &DebugManager::continueDebug);
    actionImpl = initAction(continueDebugging.get(), "Debug.Continue",
                            MWMDA_CONTINUE, QKeySequence(Qt::Key::Key_F5),
                            "debugger_continue");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), false);

    abortDebugging.reset(new QAction(MWMDA_ABORT_DEBUGGING));
    abortDebugging->setEnabled(false);
    connect(abortDebugging.get(), &QAction::triggered, debugManager, &DebugManager::abortDebug);
    actionImpl = initAction(abortDebugging.get(), "Debug.Abort.Debugging",
                            MWMDA_ABORT_DEBUGGING, QKeySequence(Qt::Modifier::SHIFT | Qt::Key::Key_F5),
                            "debugger_stop");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), false);

    restartDebugging.reset(new QAction(MWMDA_RESTART_DEBUGGING));
    restartDebugging->setEnabled(false);
    connect(restartDebugging.get(), &QAction::triggered, debugManager, &DebugManager::restartDebug);
    actionImpl = initAction(restartDebugging.get(), "Debug.Restart.Debugging",
                            MWMDA_RESTART_DEBUGGING, QKeySequence(Qt::Modifier::SHIFT | Qt::Key::Key_B),
                            "restart_debug");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), false);

    stepOver.reset(new QAction(MWMDA_STEP_OVER));
    stepOver->setEnabled(false);
    connect(stepOver.get(), &QAction::triggered, debugManager, &DebugManager::stepOver);
    actionImpl = initAction(stepOver.get(), "Debug.Step.Over",
                            MWMDA_STEP_OVER, QKeySequence(Qt::Key::Key_F10),
                            "debugger_stepover");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), true);

    stepIn.reset(new QAction(MWMDA_STEP_IN));
    stepIn->setEnabled(false);
    connect(stepIn.get(), &QAction::triggered, debugManager, &DebugManager::stepIn);
    actionImpl = initAction(stepIn.get(), "Debug.Step.In",
                            MWMDA_STEP_IN, QKeySequence(Qt::Key::Key_F11),
                            "debugger_stepinto");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), false);

    stepOut.reset(new QAction(MWMDA_STEP_OUT));
    stepOut->setEnabled(false);
    connect(stepOut.get(), &QAction::triggered, debugManager, &DebugManager::stepOut);
    actionImpl = initAction(stepOut.get(), "Debug.Step.Out",
                            MWMDA_STEP_OUT, QKeySequence(Qt::Modifier::SHIFT | Qt::Key::Key_F11),
                            "debugger_stepout");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), false);

    stepBack.reset(new QAction(MWMDA_STEP_BACK));
    stepBack->setEnabled(false);
    connect(stepBack.get(), &QAction::triggered, debugManager, &DebugManager::stepBack);
    actionImpl = initAction(stepBack.get(), "Debug.Step.Back",
                            MWMDA_STEP_BACK, QKeySequence(),
                            "debugger_stepback");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), false);

    reverseContinue.reset(new QAction(MWMDA_REVERSE_CONTINUE));
    reverseContinue->setEnabled(false);
    connect(reverseContinue.get(), &QAction::triggered, debugManager, &DebugManager::reverseContinue);
    actionImpl = initAction(reverseContinue.get(), "Debug.Reverse.Continue",
                            MWMDA_REVERSE_CONTINUE, QKeySequence(),
                            "debugger_reverse_continue");
    mDebug->addAction(actionImpl);
    appOutPutPane->registerItemToToolBar(debugToolBarName, actionImpl->action(), false);

    remoteDebug.reset(new QAction(MWMDA_REMOTE_DEBUG));
    connect(remoteDebug.get(), &QAction::triggered, debugManager, [=]() {
        auto remoteDlg = new RemoteDebugDlg();
        remoteDlg->setAttribute(Qt::WA_DeleteOnClose);
        remoteDlg->exec();
    });
    actionImpl = initAction(remoteDebug.get(), "Debug.Remote.Debug",
                            MWMDA_REMOTE_DEBUG, QKeySequence(),
                            "debugger_remotedebug");
    mDebug->addAction(actionImpl);

    attachDebugging.reset(new QAction(MWMDA_ATTACH_DEBUG));
    connect(attachDebugging.get(), &QAction::triggered, debugManager, &DebugManager::attachDebug);
    actionImpl = initAction(attachDebugging.get(), "Debug.Attach.Debugging",
                            MWMDA_ATTACH_DEBUG, QKeySequence(),
                            "debugger_start");
    mDebug->addAction(actionImpl);
}

void MenuManager::handleRunStateChanged(AbstractDebugger::RunState state)
{
    switch (state) {
    case AbstractDebugger::kNoRun:
    case AbstractDebugger::kPreparing:
    case AbstractDebugger::kStart:
        startDebugging->setEnabled(true);
#if 0   // not used yet.
        detachDebugger->setEnabled(true);
#endif
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(false);
        restartDebugging->setEnabled(false);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        stepBack->setEnabled(false);
        reverseContinue->setEnabled(false);
        attachDebugging->setEnabled(true);
        break;

    case AbstractDebugger::kRunning:
        startDebugging->setEnabled(false);
#if 0   // not used yet.
        detachDebugger->setEnabled(false);
#endif
        interrupt->setEnabled(true);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(true);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        stepBack->setEnabled(false);
        reverseContinue->setEnabled(false);
        attachDebugging->setEnabled(false);
        break;
    case AbstractDebugger::kStopped:
        startDebugging->setEnabled(false);
#if 0   // not used yet.
        detachDebugger->setEnabled(false);
#endif
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(true);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(true);
        stepOver->setEnabled(true);
        stepIn->setEnabled(true);
        stepOut->setEnabled(true);
        stepBack->setEnabled(true & debugManager->supportStepBack());
        reverseContinue->setEnabled(true & debugManager->supportStepBack());
        attachDebugging->setEnabled(false);
        break;
    case AbstractDebugger::kCustomRunning:
        startDebugging->setEnabled(false);
        interrupt->setEnabled(false);
        continueDebugging->setEnabled(false);
        abortDebugging->setEnabled(true);
        restartDebugging->setEnabled(false);
        stepOver->setEnabled(false);
        stepIn->setEnabled(false);
        stepOut->setEnabled(false);
        stepBack->setEnabled(false);
        reverseContinue->setEnabled(false);
        attachDebugging->setEnabled(false);
        break;

    default:
        // do nothing.
        break;
    }
}
