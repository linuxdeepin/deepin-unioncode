// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debugmanager.h"
#include "dap/dapdebugger.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "interface/menumanager.h"
#include "interface/attachinfodialog.h"
#include "reversedebug/reversedebugger.h"

#include "services/debugger/debuggerservice.h"
#include "services/window/windowservice.h"
#include "services/language/languageservice.h"
#include "services/editor/editorservice.h"
#include "common/util/custompaths.h"
#include "common/project/projectinfo.h"

using namespace DEBUG_NAMESPACE;
using namespace dpfservice;
DebugManager::DebugManager(QObject *parent)
    : QObject(parent)
{
    connect(debuggerSignals, &DebuggerSignals::receivedEvent, this, &DebugManager::handleEvents);
}

bool DebugManager::initialize(dpfservice::WindowService *windowService,
                              dpfservice::DebuggerService *debuggerService)
{
    currentDebugger = new DAPDebugger(this);
    debuggers.insert("dap", currentDebugger);
    runner = new Runner(this);

    menuManager.reset(new MenuManager());
    menuManager->initialize(windowService);

    auto reverseDbg = new ReverseDebugger(this);
    connect(reverseDbg, &ReverseDebugger::startReplay, this, &DebugManager::rrReplay);

    connect(currentDebugger, &AbstractDebugger::runStateChanged, this, &DebugManager::handleRunStateChanged);

    // bind debug services
    using namespace std::placeholders;
    if (!debuggerService->runCoredump) {
        debuggerService->runCoredump = std::bind(&DebugManager::runCoredump, this, _1, _2, _3);
    }

    if (!debuggerService->registerDebugger) {
        debuggerService->registerDebugger = std::bind(&DebugManager::registerDebugger, this, _1, _2);
    }

    if (!debuggerService->getDebugState) {
        debuggerService->getDebugState = std::bind(&DebugManager::getRunState, this);
    }

    return true;
}

DWidget *DebugManager::getStackPane() const
{
    return currentDebugger->getStackPane();
}

DWidget *DebugManager::getLocalsPane() const
{
    return currentDebugger->getLocalsPane();
}

DWidget *DebugManager::getBreakpointPane() const
{
    return currentDebugger->getBreakpointPane();
}

DWidget *DebugManager::getDebugMainPane() const
{
    return currentDebugger->getDebugMainPane();
}

AbstractDebugger::RunState DebugManager::getRunState() const
{
    return currentDebugger->getRunState();
}

void DebugManager::registerDebugger(const QString &kit, AbstractDebugger *debugger)
{
    auto iterator = debuggers.find(kit);
    if (iterator == debuggers.end()) {
        debuggers.insert(kit, debugger);
    }
}

void DebugManager::run()
{
    // save all modified files before debugging.
    dpfGetService(EditorService)->saveAll();

    AbstractDebugger::RunState state = currentDebugger->getRunState();
    switch (state) {
    case AbstractDebugger::RunState::kNoRun:
    case AbstractDebugger::RunState::kPreparing:
    {
        LanguageService *service = dpfGetService(LanguageService);
        if (service) {
            auto generator = service->create<LanguageGenerator>(activeProjectKitName);
            if (generator) {
                QString debugger = generator->debugger();
                if (debuggers.contains(debugger) && currentDebugger != debuggers[debugger]) {
                    disconnect(currentDebugger, &AbstractDebugger::runStateChanged, this, &DebugManager::handleRunStateChanged);
                    currentDebugger = debuggers[debugger];
                    connect(currentDebugger, &AbstractDebugger::runStateChanged, this, &DebugManager::handleRunStateChanged);
                }
            } else {
                auto windowService = dpfGetService(WindowService);
                windowService->notify(1, tr("Warning"), tr("The project does not have an associated build kit. Please reopen the project and select the corresponding build tool."), {});
            }
        }
        AsynInvoke(currentDebugger->startDebug());
        break;
    }
    case AbstractDebugger::RunState::kRunning:
        // TODO(mozart):stop debug
        break;
    case AbstractDebugger::RunState::kStopped:
        continueDebug();
        break;
    default:
        ;// do nothing.
    }
}

void DebugManager::remoteDebug(RemoteInfo info)
{
    if (!currentDebugger) {
        LanguageService *service = dpfGetService(LanguageService);
        if (service) {
            auto generator = service->create<LanguageGenerator>(activeProjectKitName);
            if (generator) {
                QString debugger = generator->debugger();
                if (debuggers.contains(debugger) && currentDebugger != debuggers[debugger]) {
                    disconnect(currentDebugger, &AbstractDebugger::runStateChanged, this, &DebugManager::handleRunStateChanged);
                    currentDebugger = debuggers[debugger];
                    connect(currentDebugger, &AbstractDebugger::runStateChanged, this, &DebugManager::handleRunStateChanged);
                }
            }
        }
    }
    
    AsynInvokeWithParam(currentDebugger->startDebugRemote, info);
}

void DebugManager::attachDebug()
{
    AttachInfoDialog dialog;
    connect(&dialog, &AttachInfoDialog::attachToProcessId, this, [=](const QString &processId){
        AsynInvokeWithParam(currentDebugger->attachDebug, processId);
    });

    dialog.exec();
}

void DebugManager::detachDebug()
{
    AsynInvoke(currentDebugger->detachDebug());
}

void DebugManager::interruptDebug()
{
    AsynInvoke(currentDebugger->interruptDebug());
}

void DebugManager::continueDebug()
{
    AsynInvoke(currentDebugger->continueDebug());
}

void DebugManager::reverseContinue()
{
    AsynInvoke(currentDebugger->reverseContinue());
}

void DebugManager::abortDebug()
{
    AsynInvoke(currentDebugger->abortDebug());
}

void DebugManager::restartDebug()
{
    // save all modified files before debugging.
    dpfGetService(EditorService)->saveAll();

    AsynInvoke(currentDebugger->restartDebug());
}

void DebugManager::stepOver()
{
    AsynInvoke(currentDebugger->stepOver());
}

void DebugManager::stepIn()
{
    AsynInvoke(currentDebugger->stepIn());
}

void DebugManager::stepOut()
{
    AsynInvoke(currentDebugger->stepOut());
}

void DebugManager::stepBack()
{
    AsynInvoke(currentDebugger->stepBack());
}

bool DebugManager::supportStepBack()
{
    return currentDebugger->supportStepBack();
}

void DebugManager::handleRunStateChanged(AbstractDebugger::RunState state)
{
    menuManager->handleRunStateChanged(state);

    if(state == AbstractDebugger::kStart || state == AbstractDebugger::kRunning || state == AbstractDebugger::kCustomRunning) {
        emit debugStarted();
    } else if (state == AbstractDebugger::kNoRun) {
        emit debugStopped();
    }
}

void DebugManager::handleEvents(const dpf::Event &event)
{
    QString topic = event.topic();
    QString data = event.data().toString();
    if (event.data() == debugger.prepareDebugProgress.name) {
        // TODO(logan)
    } else if (event.data() == project.activatedProject.name) {
        auto projectInfo = qvariant_cast<ProjectInfo>(event.property(project.activatedProject.pKeys[0]));
        activeProjectKitName = projectInfo.kitName();
    } else if (event.data() == project.createdProject.name) {
        auto projectInfo = qvariant_cast<ProjectInfo>(event.property(project.createdProject.pKeys[0]));
        activeProjectKitName = projectInfo.kitName();
    } else if (event.data() == project.deletedProject.name) {
        activeProjectKitName.clear();
    } else if (event.data() == editor.switchedFile.name) {
        // TODO(logan)
    } else if (event.data() == editor.fileOpened.name) {
        // TODO(logan)
    } else if (event.data() == editor.fileClosed.name) {
        // TODO(logan)
    }
}

bool DebugManager::runCoredump(const QString &target, const QString &core, const QString &kit)
{
    // return QtConcurrent::run(currentDebugger, &AbstractDebugger::runCoredump, target, core, kit);
    auto future = QtConcurrent::run([&, this](){
        currentDebugger->runCoredump(target, core, kit);
    });
    return future.isRunning();
}

void DebugManager::rrReplay(const QString &target)
{
    auto dapdbgger = qobject_cast<DAPDebugger *>(debuggers["dap"]);
    dapdbgger->startRerverseDebug(target);
}

