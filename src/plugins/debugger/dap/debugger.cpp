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
#include "debugger.h"
#include "runtimecfgprovider.h"
#include "debugsession.h"
#include "debugservice.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "debugmodel.h"
#include "stackframe.h"
#include "interface/appoutputpane.h"
#include "interface/stackframemodel.h"
#include "interface/stackframeview.h"
#include "interface/messagebox.h"
#include "event/eventreceiver.h"
#include "common/common.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"
#include "services/language/languageservice.h"
#include "common/util/downloadutil.h"
#include "common/util/fileoperation.h"

#include <QDateTime>
#include <QTextBlock>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>

#define PER_WAIT_MSEC (1000)
#define MAX_WAIT_TIMES (10)

/**
 * @brief Debugger::Debugger
 * For serial debugging service
 */
using namespace dap;
using namespace DEBUG_NAMESPACE;
using namespace dpfservice;

class DebuggerPrivate
{
    friend class Debugger;

    QString activeProjectKitName;
    dpfservice::ProjectInfo projectInfo;
    QString currentOpenedFileName;
    QString currentBuildUuid;
    QString requestDAPPortUuid;
    QSharedPointer<QTimer> timer = nullptr;
    QSharedPointer<QThread> timerThread = nullptr;
    std::atomic_bool isWaitingPort = false;
    std::atomic_bool isCustomDap = false;
    QString userKitName;
};

Debugger::Debugger(QObject *parent)
    : QObject(parent)
    , d(new DebuggerPrivate())
{
    qRegisterMetaType<OutputFormat>("OutputFormat");
    qRegisterMetaType<StackFrameData>("StackFrameData");
    qRegisterMetaType<StackFrames>("StackFrames");

    qRegisterMetaType<IVariable>("IVariable");
    qRegisterMetaType<IVariables>("IVariables");
    qRegisterMetaType<dpf::Event>("dpf::Event");
    qRegisterMetaType<RunState>("RunState");

    session.reset(new DebugSession(debugService->getModel(), this));
    connect(session.get(), &DebugSession::sigRegisterHandlers, this, &Debugger::registerDapHandlers);
    rtCfgProvider.reset(new RunTimeCfgProvider(this));

    connect(debuggerSignals, &DebuggerSignals::receivedEvent, this, &Debugger::handleFrameEvent);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.disconnect(QString(""),
                          "/path",
                          "com.deepin.unioncode.interface",
                          "dapport",
                          this, SLOT(slotReceivedDAPPort(QString, int, QString, QMap<QString, QVariant>)));
    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "dapport",
                       this, SLOT(slotReceivedDAPPort(QString, int, QString, QMap<QString, QVariant>)));

    initializeView();
}

Debugger::~Debugger()
{
    delete alertBox;
    // all widgets in tabWidget will be deleted automatically.
}

AppOutputPane *Debugger::getOutputPane() const
{
    return outputPane;
}

QWidget *Debugger::getStackPane() const
{
    return stackPane;
}

QTreeView *Debugger::getLocalsPane() const
{
    return localsView;
}

QTreeView *Debugger::getBreakpointPane() const
{
    return breakpointView;
}

void Debugger::startDebug()
{
    updateRunState(kPreparing);
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
        if (generator) {
            if (generator->isNeedBuild()) {
                d->currentBuildUuid = requestBuild();
            } else {
                prepareDebug();
            }
        }
    }
}

void Debugger::detachDebug()
{
}

void Debugger::interruptDebug()
{
    if (runState == kRunning) {
        // Just use temporary parameters now, same for the back
        session->pause(threadId);
    }
}

void Debugger::continueDebug()
{
    if (runState == kStopped) {
        session->continueDbg(threadId);
        editor.cleanRunning();
    }
}

void Debugger::abortDebug()
{
    if (runState == kRunning || runState == kStopped || runState == kCustomRunning) {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            QString kitName = (runState == kCustomRunning) ? d->userKitName : d->activeProjectKitName;
            auto generator = service->create<LanguageGenerator>(kitName);
            if (generator) {
                if (generator->isStopDAPManually()) {
                    stopDAP();
                } else {
                    session->terminate();
                }
            }
        }
    }
}

void Debugger::restartDebug()
{
    if (runState == kStopped || runState == kRunning) {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
            if (generator) {
                if (generator->isRestartDAPManually()) {
                    stopDAP();
                    prepareDebug();
                } else {
                    session->restart();
                }
            }
        }
    }
}

void Debugger::stepOver()
{
    if (runState == kStopped) {
        session->next(threadId, undefined);
    }
}

void Debugger::stepIn()
{
    if (runState == kStopped) {
        session->stepIn(threadId, undefined, undefined);
    }
}

void Debugger::stepOut()
{
    if (runState == kStopped) {
        session->stepOut(threadId, undefined);
    }
}

Debugger::RunState Debugger::getRunState() const
{
    return runState;
}

void Debugger::addBreakpoint(const QString &filePath, int lineNumber)
{
    // update model here.
    Internal::Breakpoint bp;
    bp.filePath = filePath;
    bp.fileName = QFileInfo(filePath).fileName();
    bp.lineNumber = lineNumber;
    breakpointModel.insertBreakpoint(bp);

    // send to backend.
    dap::array<IBreakpointData> rawBreakpoints;
    IBreakpointData bpData;
    bpData.id = QUuid::createUuid().toString().toStdString();
    bpData.lineNumber = lineNumber;
    bpData.enabled = true;   // TODO(mozart):get from editor.
    rawBreakpoints.push_back(bpData);

    if (runState == kStopped || runState == kRunning) {
        debugService->addBreakpoints(filePath, rawBreakpoints, session.get());
    } else {
        debugService->addBreakpoints(filePath, rawBreakpoints, undefined);
    }
}

void Debugger::removeBreakpoint(const QString &filePath, int lineNumber)
{
    // update model here.
    Internal::Breakpoint bp;
    bp.filePath = filePath;
    bp.fileName = QFileInfo(filePath).fileName();
    bp.lineNumber = lineNumber;
    breakpointModel.removeBreakpoint(bp);

    // send to backend.
    if (runState == kStopped || runState == kRunning) {
        debugService->removeBreakpoints(filePath, lineNumber, session.get());
    } else {
        debugService->removeBreakpoints(filePath, lineNumber, undefined);
    }
}

bool Debugger::getLocals(dap::integer frameId, IVariables *out)
{
    return session->getLocals(frameId, out);
}

void Debugger::registerDapHandlers()
{
    dap::Session *dapSession = session.get()->getDapSession();
    /*
     *  Process the only one reverse request.
     */
    dapSession->registerHandler([&](const RunInTerminalRequest &request) {
        Q_UNUSED(request)
        qInfo() << "\n--> recv : "
                << "RunInTerminalRequest";
        return RunInTerminalResponse();
    });

    /*
     *  Register events.
     */
    // This event indicates that the debug adapter is ready to accept configuration requests.
    dapSession->registerHandler([&](const InitializedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "InitializedEvent";

        if (d->isCustomDap) {
            auto threads = session->fetchThreads(nullptr);
            updateThreadList(-1, threads);
            updateRunState(Debugger::RunState::kCustomRunning);
        } else {
            session.get()->getRawSession()->setReadyForBreakpoints(true);
            debugService->sendAllBreakpoints(session.get());

            session.get()->getRawSession()->configurationDone().wait();

            session->fetchThreads(nullptr);
            updateRunState(Debugger::RunState::kRunning);
        }
    });

    // The event indicates that the execution of the debuggee has stopped due to some condition.
    dapSession->registerHandler([&](const StoppedEvent &event) {
        qInfo() << "\n--> recv : "
                << "StoppedEvent";
        qInfo() << "\n THREAD STOPPED. Reason : " << event.reason.c_str();

        IRawStoppedDetails *details = new IRawStoppedDetails();
        details->reason = event.reason;
        details->description = event.description;
        details->threadId = event.threadId;
        details->text = event.text;
        //        details.totalFrames = event.;
        details->allThreadsStopped = event.allThreadsStopped.value();
        //        details.framesErrorMessage = even;
        details->hitBreakpointIds = event.hitBreakpointIds;
        session->getStoppedDetails().push_back(details);

        auto threads = session->fetchThreads(details);

        int curThreadID = static_cast<int>(event.threadId.value(0));
        updateThreadList(curThreadID, threads);

        // ui focus on the active frame.
        if (event.reason == "function breakpoint"
                || event.reason == "breakpoint"
                || event.reason == "step"
                || event.reason == "breakpoint-hit"
                || event.reason == "function-finished"
                || event.reason == "end-stepping-range"
                || event.reason == "signal-received") {
            if (event.threadId) {
                threadId = event.threadId.value(0);
                switchCurrentThread(static_cast<int>(threadId));
            }
            updateRunState(Debugger::RunState::kStopped);
        } else if (event.reason == "exception") {
            QString name;
            if (event.description) {
                name = event.description.value().c_str();
            } else {
                name = event.reason.c_str();
            }
            QString meaning;
            if (event.text) {
                meaning = event.text.value().c_str();
            }

            QMetaObject::invokeMethod(this, "showStoppedBySignalMessageBox",
                                      Q_ARG(QString, meaning), Q_ARG(QString, name));

            printOutput(tr("\nThe debugee has Terminated.\n"), NormalMessageFormat);

            updateRunState(kNoRun);
        }
    });

    // The event indicates that the execution of the debuggee has continued.
    //    session->registerHandler([&](const ContinuedEvent &event){
    //        allThreadsContinued = event.allThreadsContinued;
    //        Q_UNUSED(event)
    //        qInfo() << "\n--> recv : " << "ContinuedEvent";
    //    });

    // The event indicates that the debuggee has exited and returns its exit code.
    dapSession->registerHandler([&](const ExitedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ExitedEvent";
        printOutput(tr("The debugee has Exited.\n"), NormalMessageFormat);
        updateRunState(kNoRun);
    });

    // The event indicates that debugging of the debuggee has terminated.
    // This does not mean that the debuggee itself has exited.
    dapSession->registerHandler([&](const TerminatedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "TerminatedEvent";
        printOutput(tr("\nThe debugee has Terminated.\n"), NormalMessageFormat);
        updateRunState(kNoRun);
    });

    // The event indicates that a thread has started or exited.
    dapSession->registerHandler([&](const ThreadEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ThreadEvent";
    });

    // The event indicates that the target has produced some output.
    dapSession->registerHandler([&](const OutputEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "OutputEvent\n"
                << "content : " << event.output.c_str();

        if (event.category) {
            if (event.category.value() == "assembler") {
                QString output = event.output.c_str();
                handleAssemble(output);
                return;
            }
        }

        OutputFormat format = NormalMessageFormat;
        if (event.category) {
            dap::string category = event.category.value();
            if (category == "stdout") {
                format = OutputFormat::StdOutFormat;
            } else if (category == "stderr") {
                format = OutputFormat::StdErrFormat;
            } else {
                format = OutputFormat::LogMessageFormat;
            }
        }

        QString output = event.output.c_str();
        if (output.contains("received signal")
                || output.contains("Program")) {
            format = OutputFormat::StdErrFormat;
        }
        printOutput(output, format);
    });

    // The event indicates that some information about a breakpoint has changed.
    dapSession->registerHandler([&](const BreakpointEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "BreakpointEvent";
    });

    // The event indicates that some information about a module has changed.
    dapSession->registerHandler([&](const ModuleEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ModuleEvent";
    });

    // The event indicates that some source has been added, changed,
    // or removed from the set of all loaded sources.
    dapSession->registerHandler([&](const LoadedSourceEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "LoadedSourceEvent";
    });

    // The event indicates that the debugger has begun debugging a new process.
    dapSession->registerHandler([&](const ProcessEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ProcessEvent";
    });

    //    // The event indicates that one or more capabilities have changed.
    //    session->registerHandler([&](const CapabilitiesEvent &event){
    //        Q_UNUSED(event)
    //        qInfo() << "\n--> recv : " << "CapabilitiesEvent";
    //    });

    dapSession->registerHandler([&](const ProgressStartEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ProgressStartEvent";
    });

    dapSession->registerHandler([&](const ProgressUpdateEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ProgressUpdateEvent";
    });

    dapSession->registerHandler([&](const ProgressEndEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "ProgressEndEvent";
    });

    // This event signals that some state in the debug adapter has changed
    // and requires that the client needs to re-render the data snapshot previously requested.
    dapSession->registerHandler([&](const InvalidatedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "InvalidatedEvent";
    });
}

void Debugger::handleFrameEvent(const dpf::Event &event)
{
    if (!DebugEventReceiver::topics().contains(event.topic()))
        return;

    QString topic = event.topic();
    QString data = event.data().toString();
    if (topic == T_BUILDER) {
        if (data == D_BUILD_STATE) {
            int state = event.property(P_STATE).toInt();
            BuildCommandInfo commandInfo = qvariant_cast<BuildCommandInfo>(event.property(P_ORIGINCMD));
            if (commandInfo.uuid == d->currentBuildUuid) {
                int buildSuccess = 0;
                if (state == buildSuccess && runState == kPreparing)
                    start();
            }
        }
    }

    if (event.data() == debugger.prepareDebugDone.name) {
        bool succeed = event.property(debugger.prepareDebugDone.pKeys[0]).toBool();
        QString errorMsg = event.property(debugger.prepareDebugDone.pKeys[1]).toString();
        if (!succeed) {
            printOutput(errorMsg);
            QMetaObject::invokeMethod(this, "message", Q_ARG(QString, errorMsg));
            updateRunState(kPreparing);
        } else {
            auto &ctx = dpfInstance.serviceContext();
            LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
            if (service) {
                auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
                if (generator) {
                    QMap<QString, QVariant> param = generator->getDebugArguments(d->projectInfo, d->currentOpenedFileName);
                    prepareDAPPort(param, d->activeProjectKitName, false);
                }
            }
        }
    } else if (event.data() == debugger.prepareDebugProgress.name) {
        printOutput(event.property(debugger.prepareDebugProgress.pKeys[0]).toString());
    } else if (event.data() == project.activedProject.name) {
        d->projectInfo = qvariant_cast<ProjectInfo>(event.property(project.activedProject.pKeys[0]));
        d->activeProjectKitName = d->projectInfo.kitName();
        updateRunState(kNoRun);
    } else if (event.data() == project.createdProject.name) {
        d->projectInfo = qvariant_cast<ProjectInfo>(event.property(project.createdProject.pKeys[0]));
        d->activeProjectKitName = d->projectInfo.kitName();
        updateRunState(kNoRun);
    } else if (event.data() == project.deletedProject.name) {
        d->activeProjectKitName.clear();
        updateRunState(kNoRun);
    } else if (event.data() == editor.switchedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        if (d->currentOpenedFileName != filePath) {
            d->currentOpenedFileName = filePath;
        }
    } else if (event.data() == editor.openedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        d->currentOpenedFileName = filePath;
    } else if (event.data() == editor.closedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        if (d->currentOpenedFileName == filePath) {
            d->currentOpenedFileName.clear();
        }
    } else if (event.data() == editor.addadDebugPoint.name) {
        QString filePath = event.property(editor.addadDebugPoint.pKeys[0]).toString();
        int line = event.property(editor.addadDebugPoint.pKeys[1]).toInt();
        addBreakpoint(filePath, line);
    } else if (event.data() == editor.removedDebugPoint.name) {
        QString filePath = event.property(editor.addadDebugPoint.pKeys[0]).toString();
        int line = event.property(editor.addadDebugPoint.pKeys[1]).toInt();
        removeBreakpoint(filePath, line);
    }
}

void Debugger::printOutput(const QString &content, OutputFormat format)
{
    QString outputContent = content;
    if (format == NormalMessageFormat) {
        QTextDocument *doc = outputPane->document();
        QTextBlock tb = doc->lastBlock();
        QString lastLineText = tb.text();
        QString prefix = "\n";
        if (lastLineText.isEmpty()) {
            prefix = "";
        }
        QDateTime curDatetime = QDateTime::currentDateTime();
        QString time = curDatetime.toString("hh:mm:ss");
        outputContent = prefix + time + ":" + content + "\n";
    }
    QMetaObject::invokeMethod(outputPane, "appendText",
                              Q_ARG(QString, outputContent), Q_ARG(OutputFormat, format));
}

void Debugger::handleFrames(const StackFrames &stackFrames)
{
    stackModel.setFrames(stackFrames);

    auto curFrame = stackModel.currentFrame();
    if (curFrame.line == -1) {
        // none of frame in model.
        return;
    }

    if (QFileInfo(curFrame.file).exists()) {
        editor.runningToLine(curFrame.file, curFrame.line);
    } else {
        if (curFrame.address.isEmpty()) {
            disassemble(curFrame.address);
        }
    }

    // update local variables.
    IVariables locals;
    getLocals(curFrame.frameId, &locals);
    localsModel.setDatas(locals);
}

void Debugger::updateThreadList(int curr, const dap::array<dap::Thread> &threads)
{
    threadSelector->clear();
    int currIdx = -1;
    for (const auto& e: threads) {
        QString itemText = "#" + QString::number(e.id) + " " + e.name.c_str();
        threadSelector->addItem(itemText);
        if (curr == e.id)
            currIdx = threadSelector->count() - 1;
    }
    if (currIdx != -1)
        threadSelector->setCurrentIndex(currIdx);
    else if (!threads.empty()) {
        threadSelector->setCurrentIndex(0);
        int threadId = static_cast<int>(threads.at(0).id);
        switchCurrentThread(threadId);
    }
}

void Debugger::switchCurrentThread(int threadId)
{
    auto thread = session->getThread(threadId);
    if (thread) {
        thread.value()->fetchCallStack();
        auto stacks = thread.value()->getCallStack();
        StackFrames frames;
        int level = 0;
        for (auto it : stacks) {
            // TODO(mozart):send to ui.
            StackFrameData sf;
            sf.level = std::to_string(level++).c_str();
            sf.function = it.name.c_str();
            if (it.source) {
                sf.file = it.source.value().path ? it.source.value().path->c_str() : "";
            } else {
                sf.file = "No file found.";
            }

            if (it.moduleId) {
                auto v = it.moduleId.value();
                if (v.is<dap::integer>()) {
                    // TODO(mozart)
                }
            }

            sf.line = static_cast<qint32>(it.line);
            sf.address = it.instructionPointerReference ? it.instructionPointerReference.value().c_str() : "";
            sf.frameId = it.id;
            frames.push_back(sf);
        }
        //                    emit debuggerSignals->processStackFrames(frames);
        handleFrames(frames);
    }
}

bool Debugger::showStoppedBySignalMessageBox(QString meaning, QString name)
{
    if (alertBox)
        return false;

    if (name.isEmpty())
        name = ' ' + tr("<Unknown>", "name") + ' ';
    if (meaning.isEmpty())
        meaning = ' ' + tr("<Unknown>", "meaning") + ' ';
    const QString msg = tr("<p>The inferior stopped because it received a "
                           "signal from the operating system.<p>"
                           "<table><tr><td>Signal name : </td><td>%1</td></tr>"
                           "<tr><td>Signal meaning : </td><td>%2</td></tr></table>")
            .arg(name, meaning);

    alertBox = Internal::information(tr("Signal Received"), msg);
    return true;
}

void Debugger::slotFrameSelected(const QModelIndex &index)
{
    Q_UNUSED(index)
    auto curFrame = stackModel.currentFrame();

    if (QFileInfo(curFrame.file).exists()) {
        editor.jumpToLine(curFrame.file, curFrame.line);
    } else {
        if (!curFrame.address.isEmpty()) {
            disassemble(curFrame.address);
        }
    }

    // update local variables.
    IVariables locals;
    getLocals(curFrame.frameId, &locals);
    localsModel.setDatas(locals);
}

void Debugger::slotBreakpointSelected(const QModelIndex &index)
{
    Q_UNUSED(index);
    auto curBP = breakpointModel.currentBreakpoint();
    editor.jumpToLine(curBP.filePath, curBP.lineNumber);
}

void Debugger::initializeView()
{
    // initialize output pane.
    outputPane = new AppOutputPane();

    // initialize stack monitor pane.
    stackPane = new QWidget;
    QVBoxLayout *vLayout = new QVBoxLayout(stackPane);
    stackPane->setLayout(vLayout);

    stackView = new StackFrameView();
    stackView->setModel(stackModel.model());

    threadSelector = new QComboBox(stackPane);
    threadSelector->setMinimumWidth(200);
    connect(threadSelector, QOverload<const QString &>::of(&QComboBox::activated), this, &Debugger::currentThreadChanged);

    QHBoxLayout *hLayout = new QHBoxLayout(stackPane);
    hLayout->setAlignment(Qt::AlignLeft);
    QLabel *label = new QLabel(tr("Threads:"), stackPane);
    hLayout->addWidget(label);
    hLayout->addWidget(threadSelector);

    vLayout->addLayout(hLayout);
    vLayout->addWidget(stackView);

    // intialize breakpint pane.
    breakpointView = new StackFrameView();
    breakpointView->setModel(breakpointModel.model());

    localsView = new QTreeView();
    localsView->setModel(&localsModel);
    QStringList headers { "name", "value"/*, "reference" */};
    localsModel.setHeaders(headers);

    connect(stackView, &QTreeView::doubleClicked, this, &Debugger::slotFrameSelected);
    connect(breakpointView, &QTreeView::doubleClicked, this, &Debugger::slotBreakpointSelected);
}

void Debugger::exitDebug()
{
    // Change UI.
    editor.cleanRunning({});
    localsView->hide();

    localsModel.clear();
    stackModel.removeAll();

    threadId = 0;

    threadSelector->clear();
}

void Debugger::updateRunState(Debugger::RunState state)
{
    if (runState != state) {
        runState = state;
        switch (state) {
        case kNoRun:
            exitDebug();
            break;
        case kRunning:
        case kCustomRunning:
            QMetaObject::invokeMethod(localsView, "show");
            break;
        case kStopped:
            break;
        default:
            // do nothing.
            break;
        }
        emit runStateChanged(runState);
    }
}

QString Debugger::requestBuild()
{
    QString buildUuid;
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
        if (generator) {
            buildUuid = generator->build(d->projectInfo.workspaceFolder());
        }
    }

    return buildUuid;
}

void Debugger::start()
{
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
        if (generator) {
            if (!generator->isTargetReady()) {
                QMetaObject::invokeMethod(this, "message", Q_ARG(QString, "Please build first.\n Build : Ctrl + B"));
                return;
            }

            prepareDebug();
        }
    }
}

void Debugger::prepareDebug()
{
    if (getRunState() == kStart) {
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, "Is preparing dependence, please waiting for a moment"));
        return;
    }

    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
        if (generator) {
            updateRunState(kStart);
            QString retMsg;
            QMap<QString, QVariant> param = generator->getDebugArguments(d->projectInfo, d->currentOpenedFileName);
            bool ret = generator->prepareDebug(param, retMsg);
            if (!ret) {
                QMetaObject::invokeMethod(this, "message", Q_ARG(QString, retMsg));
                updateRunState(kPreparing);
            } else if (!generator->isAnsyPrepareDebug()) {
                prepareDAPPort(param, d->activeProjectKitName, false);
            }
        }
    }
}

bool Debugger::prepareDAPPort(const QMap<QString, QVariant> &param, const QString &kitName, bool customDap)
{
    if (d->isWaitingPort) {
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, "Is getting the dap port, please waiting for a moment"));
        return false;
    }

    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(kitName);
        if (generator) {
            d->isWaitingPort = true;
            printOutput(tr("Searching dap port, please waiting...(a maximum of 10 seconds) "));

            d->timer.reset(new QTimer());
            d->timerThread.reset(new QThread());
            d->timer->start(PER_WAIT_MSEC);
            d->timer->moveToThread(d->timerThread.get());
            static int times = MAX_WAIT_TIMES;
            connect(d->timer.get(), &QTimer::timeout, [this]() {
                times--;
                if (times < 0) {
                    times = MAX_WAIT_TIMES;
                    stopWaitingDAPPort();
                    QString tip = tr("Getting dap port timeout, please retry.");
                    printOutput(tip);
                    QMetaObject::invokeMethod(this, "message", Q_ARG(QString, tip));
                } else {
                    QString tip = QString("...").arg(times);
                    printOutput(tip, StdOutFormatSameLine);
                }
            });
            d->timerThread->start();

            d->isCustomDap = customDap;
            QString retMsg;
            d->requestDAPPortUuid = QUuid::createUuid().toString();
            if (!generator->requestDAPPort(d->requestDAPPortUuid, param, retMsg)) {
                QMetaObject::invokeMethod(this, "message", Q_ARG(QString, retMsg));
                stopWaitingDAPPort();
                return false;
            }
        }
    }

    return true;
}

void Debugger::stopDAP()
{
    updateRunState(kNoRun);
    session->closeSession();
}

void Debugger::slotReceivedDAPPort(const QString &uuid, int port, const QString &kitName, const QMap<QString, QVariant> &param)
{
    if (d->isWaitingPort) {
        stopWaitingDAPPort();

        if (d->requestDAPPortUuid == uuid)
            launchSession(port, param, kitName);
    }
}

void Debugger::stopWaitingDAPPort()
{
    d->timer->stop();
    d->timerThread->quit();
    d->isWaitingPort = false;
    updateRunState(kPreparing);
}

void Debugger::launchSession(int port, const QMap<QString, QVariant> &param, const QString &kitName)
{
    if (!port) {
        printOutput(tr("\nThe dap port is not ready, please retry.\n"), ErrorMessageFormat);
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, "Could not find server port!"));
        return;
    }

    printOutput(tr("Debugging starts"));
    QString launchTip = QString("Launch dap session with port %1 ...")
            .arg(port);
    printOutput(launchTip);

    auto iniRequet = rtCfgProvider->initalizeRequest();
    bool bSuccess = session->initialize(rtCfgProvider->ip(),
                                        port,
                                        iniRequet);

    // Launch debuggee.
    if (bSuccess) {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            auto generator = service->create<LanguageGenerator>(kitName);
            if (generator) {
                if (generator->isLaunchNotAttach()) {
                    dap::LaunchRequest request = generator->launchDAP(param);
                    bSuccess &= session->launch(request);
                } else {
                    dap::AttachRequest request = generator->attachDAP(port, param);
                    bSuccess &= session->attach(request);
                }
            }
        } else
            bSuccess &= false;
    }

    if (!bSuccess) {
        qCritical() << "startDebug failed!";
    } else {
        debugService->getModel()->clear();
        debugService->getModel()->addSession(session.get());
    }
}

void Debugger::message(QString msg)
{
    ContextDialog::ok(msg);
}

void Debugger::currentThreadChanged(const QString &text)
{
    QtConcurrent::run([&]() { // run in thread to avoid blocked when get variables.
        QStringList l = text.split("#");
        QString threadNumber = l.last().split(" ").first();
        switchCurrentThread(threadNumber.toInt());
    });
}

bool Debugger::runCoredump(const QString &target, const QString &core, const QString &kit)
{
    updateRunState(kStart);
    updateRunState(kNoRun);
    printOutput(tr("Start debugging coredump file: ") + core + " with " + target);

    if (target.isEmpty() || !QFileInfo(target).isFile()) {
        QString tipMsg = tr("The coredump target file is error: ") + target;
        printOutput(tipMsg);
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, tipMsg));
        return false;
    }

    if (core.isEmpty() || !QFileInfo(core).isFile()) {
        QString tipMsg = tr("The coredump file is error: ") + core;
        printOutput(tipMsg);
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, tipMsg));
        return false;
    }

    QMap<QString, QVariant> param;
    param.insert("targetPath", target);
    param.insert("arguments", QStringList{core});
    d->userKitName = kit;

    return prepareDAPPort(param, d->userKitName, true);
}

void Debugger::disassemble(const QString &address)
{
    if (runState == kCustomRunning) {
        session->disassemble(address.toStdString());
    }
}

void Debugger::handleAssemble(const QString &content)
{
    if (content.isEmpty())
        return;

    QString assemblerPath = CustomPaths::user(CustomPaths::Flags::Configures) +
            QDir::separator() + QString("Disassembler");

    QFile file(assemblerPath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << content;
        file.close();
        editor.jumpToLine(assemblerPath, 0);
    }
}
