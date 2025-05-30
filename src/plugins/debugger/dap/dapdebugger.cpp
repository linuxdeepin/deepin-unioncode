// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debuggerglobals.h"
#include "dapdebugger.h"
#include "runtimecfgprovider.h"
#include "debugsession.h"
#include "debugservice.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "debugmodel.h"
#include "stackframe.h"
#include "interface/stackframemodel.h"
#include "interface/stackframeview.h"
#include "interface/breakpointview.h"
#include "common/widget/messagebox.h"
#include "interface/breakpointmodel.h"
#include "event/eventreceiver.h"
#include "common/widget/appoutputpane.h"
#include "common/common.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"
#include "services/language/languageservice.h"
#include "services/window/windowservice.h"
#include "services/session/sessionservice.h"
#include "unistd.h"
#include "base/baseitemdelegate.h"

#include <DLabel>
#include <DFrame>
#include <DComboBox>
#include <DPushButton>
#include <DSimpleListView>
#include <DSpinner>
#include <DDialog>
#include <DLineEdit>
#include <DSplitter>

#include <QDateTime>
#include <QTextBlock>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>
#include <QAtomicInt>
#include <QApplication>

#define PER_WAIT_MSEC (1000)
#define MAX_WAIT_TIMES (10)

constexpr char kBreakpointList[] { "BreakpointList" };
constexpr char kFileName[] { "FileName" };
constexpr char kLineNumber[] { "LineNumber" };
constexpr char kEnable[] { "Enabel" };
constexpr char kBreakpointCondition[] { "Condition" };
/**
 * @brief Debugger::Debugger
 * For serial debugging service
 */
using namespace dap;
using namespace DEBUG_NAMESPACE;
using namespace dpfservice;
using DTK_WIDGET_NAMESPACE::DSpinner;

void notify(uint type, const QString &message)   // type 0-infomation, 1-warning, 2-error
{
    auto wdService = dpfGetService(WindowService);
    wdService->notify(type, QObject::tr("Debug"), message, {});
}

class DebuggerPrivate
{
    friend class DAPDebugger;
    ~DebuggerPrivate();

    QString activeProjectKitName;
    dpfservice::ProjectInfo projectInfo;
    QString currentOpenedFileName;
    QString currentBuildUuid;
    QString requestDAPPortPpid;
    QString userKitName;
    QString currentDebugKit;

    QSharedPointer<RunTimeCfgProvider> rtCfgProvider;
    DEBUG::DebugSession *localSession { nullptr };
    DEBUG::DebugSession *remoteSession { nullptr };
    DEBUG::DebugSession *currentSession { nullptr };

    dap::integer threadId = 0;
    StackFrameData currentValidFrame;

    /**
     * @brief interface objects.
     */
    OutputPane *outputPane = nullptr;

    DWidget *stackPane = nullptr;
    StackFrameView *stackView = nullptr;
    StackFrameModel stackModel;
    DComboBox *threadSelector = nullptr;

    DSplitter *variablesPane = nullptr;
    DTreeView *localsView = nullptr;
    DTreeView *watchsView = nullptr;
    LocalTreeModel localsModel;
    LocalTreeModel watchsModel;
    QMap<QString, IVariable> watchingVariables;
    DSpinner *variablesSpinner { nullptr };
    QTimer processingVariablesTimer;
    QAtomicInt processingVariablesCount = 0;
    QFuture<void> getLocalsFuture;

    BreakpointView *breakpointView = nullptr;
    BreakpointModel breakpointModel;

    bool pausing = false;

    DFrame *debugMainPane = nullptr;

    QPointer<QWidget> alertBox;
    AbstractDebugger::RunState runState = AbstractDebugger::kNoRun;

    std::atomic_bool isCustomDap = false;

    QProcess backend;

    bool isRemote = false;
    RemoteInfo remoteInfo;

    DAPDebugger::debugState debugState = DAPDebugger::Normal;
    SessionService *sessionSrv = nullptr;
};

DebuggerPrivate::~DebuggerPrivate()
{
    if (alertBox)
        delete alertBox;
}

DAPDebugger::DAPDebugger(QObject *parent)
    : AbstractDebugger(parent), d(new DebuggerPrivate())
{
    qRegisterMetaType<OutputPane::OutputFormat>("OutputPane::OutputFormat");
    qRegisterMetaType<StackFrameData>("StackFrameData");
    qRegisterMetaType<StackFrames>("StackFrames");

    qRegisterMetaType<IVariable>("IVariable");
    qRegisterMetaType<IVariables>("IVariables");
    qRegisterMetaType<dpf::Event>("dpf::Event");
    qRegisterMetaType<RunState>("RunState");

    d->sessionSrv = dpfGetService(SessionService);
    d->localSession = new DebugSession(debugService->getModel(), this);
    d->currentSession = d->localSession;
    connect(d->currentSession, &DebugSession::sigRegisterHandlers, this, &DAPDebugger::registerDapHandlers);
    d->rtCfgProvider.reset(new RunTimeCfgProvider(this));

    connect(debuggerSignals, &DebuggerSignals::receivedEvent, this, &DAPDebugger::handleEvents);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "dapport",
                       this, SLOT(slotReceivedDAPPort(QString, int, QString, QMap<QString, QVariant>)));

    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "output",
                       this, SLOT(slotOutputMsg(const QString &, const QString &)));

    initializeView();

    launchBackend();
}

DAPDebugger::~DAPDebugger()
{
    delete d;
}

DWidget *DAPDebugger::getOutputPane() const
{
    return d->outputPane;
}

DWidget *DAPDebugger::getStackPane() const
{
    return d->stackPane;
}

DWidget *DAPDebugger::getLocalsPane() const
{
    return d->variablesPane;
}

DWidget *DAPDebugger::getBreakpointPane() const
{
    return d->breakpointView;
}

DWidget *DAPDebugger::getDebugMainPane() const
{
    return d->debugMainPane;
}

void DAPDebugger::startDebug()
{
    d->isRemote = false;
    if (d->currentSession == d->remoteSession)
        d->currentSession = d->localSession;

    QMetaObject::invokeMethod(this, [=](){
        auto appOutPutPane = AppOutputPane::instance();
        appOutPutPane->createApplicationPane("debugPane", "debugTarget");
        appOutPutPane->setStopHandler("debugPane", [=]() {
            abortDebug();
            d->outputPane = appOutPutPane->defaultPane();
        });
        d->outputPane = appOutPutPane->getOutputPaneById("debugPane");

        appOutPutPane->bindToolBarToPane(debugToolBarName, d->outputPane);
        AppOutputPane::instance()->setProcessFinished("debugPane"); // only show log untill debuggee launching
    });

    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
        if (generator) {
            updateRunState(kPreparing);
            if (generator->isNeedBuild()) {
                d->currentBuildUuid = requestBuild();
            } else {
                prepareDebug();
            }
        }
    }
}

void DAPDebugger::startRerverseDebug(const QString &target)
{
    d->isRemote = false;
    d->debugState = Reverse;
    if (d->currentSession == d->remoteSession)
        d->currentSession = d->localSession;

    updateRunState(kPreparing);

    QMap<QString, QVariant> param;
    param.insert("program", "rr");

    d->requestDAPPortPpid = QString::number(getpid());
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "getDebugPort");

    msg << d->requestDAPPortPpid
        << "cmake"   //rr only support c/c++
        << ""
        << QStringList { target };

    bool ret = QDBusConnection::sessionBus().send(msg);
    if (!ret) {
        qWarning() << "requeset debug port failed";
        updateRunState(kNoRun);
    }
}

void DAPDebugger::startDebugRemote(const RemoteInfo &info)
{
    d->remoteInfo = info;
    d->isRemote = true;

    if (d->remoteSession)
        delete d->remoteSession;

    d->remoteSession = new DebugSession(debugService->getModel(), this);
    d->remoteSession->setRemote(true);
    d->remoteSession->setLocalProjectPath(getActiveProjectInfo().workspaceFolder());
    d->remoteSession->setRemoteProjectPath(info.projectPath);

    d->currentSession = d->remoteSession;
    connect(d->currentSession, &DebugSession::sigRegisterHandlers, this, &DAPDebugger::registerDapHandlers, Qt::DirectConnection);

    QMap<QString, QVariant> param;
    param.insert("ip", info.ip);
    param.insert("workspace", info.projectPath);
    param.insert("targetPath", info.executablePath);
    prepareDebug();

    launchSession(info.port, param, d->activeProjectKitName);

    updateRunState(kPreparing);
}

void DAPDebugger::attachDebug(const QString &processId)
{
    if (d->runState != kNoRun) {
        qWarning() << "can`t attaching to debugee when debuging other application";
        DDialog dialog;
        dialog.setMessage(tr("can`t attaching to debugee when debuging other application"));
        dialog.setIcon(QIcon::fromTheme("dialog-warning"));
        dialog.addButton(tr("Cancel"));
        dialog.exec();
        return;
    }

    d->isRemote = false;
    d->debugState = Attaching;
    d->currentSession = d->localSession;

    // only support gdb for now
    updateRunState(kStart);
    auto prjInfo = dpfGetService(ProjectService)->getActiveProjectInfo();
    QString debuggerTool = prjInfo.debugProgram();
    if (!debuggerTool.contains("gdb")) {
        auto msg = tr("The gdb is required, please install it in console with \"sudo apt install gdb\", "
                      "and then restart the tool, reselect the CMake Debugger in Options Dialog...");
        printOutput(msg, OutputPane::OutputFormat::ErrorMessage);
        return;
    }

    //todo : change signal to other debuger
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "getDebugPort");
    d->requestDAPPortPpid = QString::number(getpid());
    msg << d->requestDAPPortPpid
        << debuggerTool
        << processId
        << QStringList();
    bool ret = QDBusConnection::sessionBus().send(msg);
    if (!ret)
        printOutput(tr("Request cxx dap port failed, please retry."), OutputPane::OutputFormat::ErrorMessage);
}

void DAPDebugger::detachDebug()
{
}

void DAPDebugger::interruptDebug()
{
    if (d->runState == kRunning) {
        // Just use temporary parameters now, same for the back
        d->pausing = true;
        d->currentSession->pause(d->threadId);
    }
}

void DAPDebugger::continueDebug()
{
    if (d->runState == kStopped) {
        d->currentSession->continueDbg(d->threadId);
        editor.removeDebugLine();
    }
}

void DAPDebugger::reverseContinue()
{
    if (d->runState == kStopped) {
        d->currentSession->reverseContinue(d->threadId);
        editor.removeDebugLine();
    }
}

void DAPDebugger::abortDebug()
{
    if (d->runState == kRunning || d->runState == kStopped || d->runState == kCustomRunning) {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            auto generator = service->create<LanguageGenerator>(d->currentDebugKit);
            if (generator) {
                if (generator->isStopDAPManually()) {
                    stopDAP();
                }
            }
        }
    }

    d->currentSession->terminate();
    printOutput(tr("\nThe debugee has Terminated.\n"), OutputPane::OutputFormat::NormalMessage);
}

void DAPDebugger::restartDebug()
{
    if (d->runState == kStopped || d->runState == kRunning) {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            if (d->activeProjectKitName != d->currentDebugKit) {
                abortDebug();
                return;
            }
            auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
            if (generator) {
                if (generator->isRestartDAPManually()) {
                    stopDAP();
                    prepareDebug();
                } else {
                    d->currentSession->restart();
                }
            }
        }
    }
}

void DAPDebugger::stepOver()
{
    if (d->runState == kStopped && d->processingVariablesCount == 0) {
        d->currentSession->next(d->threadId, undefined);
    }
}

void DAPDebugger::stepIn()
{
    if (d->runState == kStopped && d->processingVariablesCount == 0) {
        d->currentSession->stepIn(d->threadId, undefined, undefined);
    }
}

void DAPDebugger::stepOut()
{
    if (d->runState == kStopped && d->processingVariablesCount == 0) {
        d->currentSession->stepOut(d->threadId, undefined);
    }
}

void DAPDebugger::stepBack()
{
    if (d->runState == kStopped && d->processingVariablesCount == 0) {
        d->currentSession->stepBack(d->threadId, undefined);
    }
}

bool DAPDebugger::supportStepBack()
{
    return d->debugState == Reverse;
}

DAPDebugger::RunState DAPDebugger::getRunState() const
{
    return d->runState;
}

void DAPDebugger::addBreakpoint(const QString &filePath, int lineNumber)
{
    const auto &bps = d->breakpointModel.breakpointList();
    bool ret = std::any_of(bps.cbegin(), bps.cend(),
                           [&](const BreakpointItem &bp) {
                               return filePath == bp.filePath() && lineNumber == bp.lineNumber();
                           });
    if (ret)
        return;

    // update model here.
    Internal::Breakpoint bp;
    bp.filePath = filePath;
    bp.fileName = QFileInfo(filePath).fileName();
    bp.lineNumber = lineNumber;
    d->breakpointModel.insertBreakpoint(bp);

    // send to backend.
    dap::array<IBreakpointData> rawBreakpoints;
    IBreakpointData bpData;
    bpData.id = QUuid::createUuid().toString().toStdString();
    bpData.lineNumber = lineNumber;
    bpData.enabled = true;   // TODO(mozart):get from editor.
    rawBreakpoints.push_back(bpData);

    if (d->runState == kStopped || d->runState == kRunning) {
        debugService->addBreakpoints(filePath, rawBreakpoints, d->currentSession);
    } else {
        debugService->addBreakpoints(filePath, rawBreakpoints, undefined);
    }
}

void DAPDebugger::removeBreakpoint(const QString &filePath, int lineNumber)
{
    // update model here.
    Internal::Breakpoint bp;
    bp.filePath = filePath;
    bp.fileName = QFileInfo(filePath).fileName();
    bp.lineNumber = lineNumber;
    d->breakpointModel.removeBreakpoint(bp);

    // send to backend.
    if (d->runState == kStopped || d->runState == kRunning) {
        debugService->removeBreakpoints(filePath, lineNumber, d->currentSession);
    } else {
        debugService->removeBreakpoints(filePath, lineNumber, undefined);
    }
}

void DAPDebugger::removeAllBreakpoints()
{
    const auto &bps = d->breakpointModel.breakpointList();
    for (const auto &bp : bps)
        removeBreakpoint(bp.filePath(), bp.lineNumber());
}

void DAPDebugger::switchBreakpointsStatus(const QString &filePath, int lineNumber, bool enabled)
{
    Internal::Breakpoint bp;
    bp.filePath = filePath;
    bp.fileName = QFileInfo(filePath).fileName();
    bp.lineNumber = lineNumber;
    bp.enabled = enabled;
    d->breakpointModel.switchBreakpointStatus(bp);

    // send to backend.
    if (d->runState == kStopped || d->runState == kRunning) {
        debugService->switchBreakpointStatus(filePath, lineNumber, enabled, d->currentSession);
    } else {
        debugService->switchBreakpointStatus(filePath, lineNumber, enabled, undefined);
    }
}

void DAPDebugger::setBreakpointCondition(const QString &filePath, int lineNumber, const QString &expression)
{
    Internal::Breakpoint bp;
    bp.filePath = filePath;
    bp.fileName = QFileInfo(filePath).fileName();
    bp.lineNumber = lineNumber;
    bp.condition = expression;
    d->breakpointModel.setBreakpointCondition(bp);

    // send to backend.
    if (d->runState == kStopped || d->runState == kRunning) {
        debugService->setBreakpointCondition(filePath, lineNumber, expression, d->currentSession);
    } else {
        debugService->setBreakpointCondition(filePath, lineNumber, expression, undefined);
    }
}

void DAPDebugger::jumpToLine(const QString &filePath, int lineNumber)
{
    if (!d->currentSession)
        return;

    dap::Source source;
    source.path = filePath.toStdString();

    if (d->runState == kStopped || d->runState == kRunning) {
        auto response = d->currentSession->gotoTargets(source, lineNumber, 0);
        for (auto target : response.value().targets) {
            if (target.label == QString("%1:%2").arg(filePath, QString::number(lineNumber)).toStdString()) {
                d->currentSession->goto_(d->threadId, target.id);
            }
        }
    }
}

void DAPDebugger::runToLine(const QString &filePath, int lineNumber)
{
    if (!d->currentSession)
        return;

    if (d->runState == kStopped) {
        dap::array<IBreakpointData> rawBreakpoints;
        IBreakpointData bpData;
        bpData.id = QUuid::createUuid().toString().toStdString();
        bpData.lineNumber = lineNumber;
        bpData.enabled = true;
        bpData.hitCondition = "1";
        rawBreakpoints.push_back(bpData);

        debugService->addBreakpoints(filePath, rawBreakpoints, d->currentSession);
        continueDebug();
    }
}

void DAPDebugger::evaluateWatchVariable(const QString &expression)
{
    IVariable var;
    var.name = expression.toStdString();
    var.var.name = expression.toStdString();

    if (d->runState == kStopped) {
        auto response = d->currentSession->evaluate(var.name, d->currentValidFrame.frameId, "watch");
        var.var.value = response.value().result;
        var.var.type = response.value().type.has_value() ? response.value().type : "";
        var.var.variablesReference = response.value().variablesReference;
    } else {
        var.var.value = "";
        var.var.type = "";
    }

    d->watchingVariables[expression] = var;
    auto vars = d->watchingVariables.values().toVector();
    d->watchsModel.setDatas(vars);
}

bool DAPDebugger::getLocals(dap::integer frameId, IVariables *out)
{
    return d->currentSession->getLocals(frameId, out);
}

void DAPDebugger::registerDapHandlers()
{
    dap::Session *dapSession = d->currentSession->getDapSession();
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
            updateRunState(DAPDebugger::RunState::kCustomRunning);
        } else {
            d->currentSession->getRawSession()->setReadyForBreakpoints(true);
            debugService->sendAllBreakpoints(d->currentSession);

            d->currentSession->getRawSession()->configurationDone().wait();
            updateRunState(DAPDebugger::RunState::kRunning);
        }
    });

    dapSession->registerHandler([&](const ContinuedEvent &event) {
        qInfo() << "\n--> recv : "
                << "ContinuedEvent";

        updateRunState(DAPDebugger::RunState::kRunning);
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
        details->allThreadsStopped = event.allThreadsStopped.has_value() ? event.allThreadsStopped.value() : dap::boolean(true);
        //        details.framesErrorMessage = even;
        details->hitBreakpointIds = event.hitBreakpointIds;
        d->currentSession->getStoppedDetails().push_back(details);

        bool signalStopped = false;
        if (event.reason == "signal-received" && event.description.has_value()) {
            signalStopped = true;
            auto signalName = QString::fromStdString(event.description.value().c_str());
            if (signalName == "SIGSEGV") {   // Segmentation fault
                auto signalMeaning = event.text.has_value() ? event.text.value().c_str() : "";
                QMetaObject::invokeMethod(this, "showStoppedBySignalMessageBox",
                                          Q_ARG(QString, QString::fromStdString(signalMeaning)), Q_ARG(QString, signalName));
            }
            if (signalName == "SIGINT" && !d->pausing)   // stopped by user
                signalStopped = false;
        }

        bool attaching = d->debugState == Attaching;
        // ui focus on the active frame.
        if (event.reason == "function breakpoint"
            || event.reason == "breakpoint"
            || event.reason == "step"
            || event.reason == "breakpoint-hit"
            || event.reason == "function-finished"
            || event.reason == "end-stepping-range"
            || event.reason == "goto"
            || event.reason == "pause"   // python send it when pauseing
            || signalStopped
            || (event.reason == "unknown" && attaching)) {
            //when attaching to running program . it won`t receive initialized event and event`s reason is "unknwon"
            //so initial breakpoints in here
            if (attaching) {
                d->currentSession->getRawSession()->setReadyForBreakpoints(true);
                debugService->sendAllBreakpoints(d->currentSession);
            }
            if (event.threadId) {
                d->threadId = event.threadId.value(0);
                int curThreadID = static_cast<int>(d->threadId);
                auto threads = d->currentSession->fetchThreads(details);
                updateThreadList(curThreadID, threads);
                switchCurrentThread(static_cast<int>(d->threadId));
            }
            updateRunState(DAPDebugger::RunState::kStopped);
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

            printOutput(tr("\nThe debugee has Terminated.\n"), OutputPane::OutputFormat::NormalMessage);

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
        printOutput(tr("The debugee has Exited.\n"), OutputPane::OutputFormat::NormalMessage);
        updateRunState(kNoRun);
    });

    // The event indicates that debugging of the debuggee has terminated.
    // This does not mean that the debuggee itself has exited.
    dapSession->registerHandler([&](const TerminatedEvent &event) {
        Q_UNUSED(event)
        qInfo() << "\n--> recv : "
                << "TerminatedEvent";

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

        OutputPane::OutputFormat format = OutputPane::OutputFormat::NormalMessage;
        if (event.category) {
            dap::string category = event.category.value();
            if (category == "stdout") {
                format = OutputPane::OutputFormat::StdOut;
            } else if (category == "stderr") {
                format = OutputPane::OutputFormat::StdErr;
            } else {
                format = OutputPane::OutputFormat::LogMessage;
            }
        }

        QString output = event.output.c_str();
        if (output.contains("received signal")
            || output.contains("Program")) {
            format = OutputPane::OutputFormat::StdErr;
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

void DAPDebugger::handleEvents(const dpf::Event &event)
{
    QString topic = event.topic();
    QString data = event.data().toString();
    if (topic == T_BUILDER) {
        if (data == D_BUILD_STATE) {
            int state = event.property(P_STATE).toInt();
            BuildCommandInfo commandInfo = qvariant_cast<BuildCommandInfo>(event.property(P_ORIGINCMD));
            if (commandInfo.uuid == d->currentBuildUuid) {
                int buildSuccess = 0;
                if (state == buildSuccess && d->runState == kPreparing)
                    start();
            }
        }
    }

    if (event.data() == debugger.prepareDebugDone.name) {
        bool succeed = event.property(debugger.prepareDebugDone.pKeys[0]).toBool();
        QString errorMsg = event.property(debugger.prepareDebugDone.pKeys[1]).toString();
        if (!succeed) {
            printOutput(errorMsg, OutputPane::ErrorMessage);
            updateRunState(kPreparing);
        } else {
            auto &ctx = dpfInstance.serviceContext();
            LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
            if (service) {
                auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
                if (generator) {
                    QMap<QString, QVariant> param = generator->getDebugArguments(getActiveProjectInfo(), d->currentOpenedFileName);
                    requestDebugPort(param, d->activeProjectKitName, false);
                }
            }
        }
    } else if (event.data() == debugger.prepareDebugProgress.name) {
        printOutput(event.property(debugger.prepareDebugProgress.pKeys[0]).toString());
    } else if (event.data() == project.activatedProject.name) {
        d->projectInfo = qvariant_cast<ProjectInfo>(event.property("projectInfo"));
        d->activeProjectKitName = d->projectInfo.kitName();
    } else if (event.data() == project.deletedProject.name) {
        auto prjInfo = event.property("projectInfo").value<dpfservice::ProjectInfo>();
        if (d->projectInfo.isSame(prjInfo)) {
            abortDebug();
            d->activeProjectKitName.clear();
        }
    } else if (event.data() == editor.switchedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        if (d->currentOpenedFileName != filePath) {
            d->currentOpenedFileName = filePath;
        }
    } else if (event.data() == editor.fileOpened.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        d->currentOpenedFileName = filePath;
        if (d->breakpointModel.breakpointSize() > 0) {
            for (auto index = 0; index < d->breakpointModel.breakpointSize(); index++) {
                auto bp = d->breakpointModel.BreakpointAt(index);
                if (bp.filePath == filePath)
                    editor.addBreakpoint(filePath, bp.lineNumber - 1, bp.enabled);
            }
        }
    } else if (event.data() == editor.fileClosed.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        if (d->currentOpenedFileName == filePath) {
            d->currentOpenedFileName.clear();
        }
    } else if (event.data() == editor.breakpointAdded.name) {
        QString filePath = event.property(editor.breakpointAdded.pKeys[0]).toString();
        int line = event.property(editor.breakpointAdded.pKeys[1]).toInt() + 1;
        addBreakpoint(filePath, line);
    } else if (event.data() == editor.breakpointRemoved.name) {
        QString filePath = event.property(editor.breakpointRemoved.pKeys[0]).toString();
        int line = event.property(editor.breakpointRemoved.pKeys[1]).toInt() + 1;
        removeBreakpoint(filePath, line);
    } else if (event.data() == editor.breakpointStatusChanged.name) {
        QString filePath = event.property("fileName").toString();
        int line = event.property("line").toInt() + 1;
        bool enabled = event.property("enabled").toBool();
        switchBreakpointsStatus(filePath, line, enabled);
    } else if (event.data() == editor.setBreakpointCondition.name) {
        QString filePath = event.property("fileName").toString();
        int line = event.property("line").toInt() + 1;

        DDialog condition;
        DLineEdit *edit = new DLineEdit(d->breakpointView);
        edit->setPlaceholderText(tr("Input Condition Expression"));
        condition.addContent(edit);
        condition.setWindowTitle(tr("Condition"));
        condition.setMessage(tr("When the breakpoint is reached, it will be hit only when the expression is true"));
        condition.insertButton(0, tr("Cancel"));
        condition.insertButton(1, tr("Ok"));

        if (condition.exec() == 1) {
            QString expression = edit->text();
            setBreakpointCondition(filePath, line, expression);
        }
    } else if (event.data() == editor.jumpToLine.name) {
        QString filePath = event.property("fileName").toString();
        int line = event.property("line").toInt() + 1;

        jumpToLine(filePath, line);
    } else if (event.data() == editor.runToLine.name) {
        QString filePath = event.property("fileName").toString();
        int line = event.property("line").toInt() + 1;

        runToLine(filePath, line);
    } else if (event.data() == session.readyToSaveSession.name) {
        QVariantList bpList;
        const auto &bps = d->breakpointModel.breakpointList();
        for (const auto &bp : bps) {
            QVariantMap map;
            map.insert(kFileName, bp.filePath());
            map.insert(kLineNumber, bp.lineNumber());
            map.insert(kEnable, bp.isEnabled());
            map.insert(kBreakpointCondition, bp.condition());
            bpList << map;
        }
        d->sessionSrv->setValue(kBreakpointList, bpList);
    } else if (event.data() == session.sessionLoaded.name) {
        removeAllBreakpoints();
        const auto bps = d->sessionSrv->value(kBreakpointList).toList();
        for (const auto &bp : bps) {
            const auto &map = bp.toMap();
            const auto fileName = map.value(kFileName).toString();
            const int lineNumber = map.value(kLineNumber).toInt();
            const bool enable = map.value(kEnable).toBool();
            const auto condition = map.value(kBreakpointCondition).toString();
            addBreakpoint(fileName, lineNumber);
            if (!enable)
                switchBreakpointsStatus(fileName, lineNumber, enable);
            if (!condition.isEmpty())
                setBreakpointCondition(fileName, lineNumber, condition);
        }
    }
}

void DAPDebugger::printOutput(const QString &content, OutputPane::OutputFormat format)
{
    QMetaObject::invokeMethod(this, "synPrintOutput", Q_ARG(QString, content), Q_ARG(OutputPane::OutputFormat, format));
}

void DAPDebugger::synPrintOutput(const QString &content, OutputPane::OutputFormat format)
{
    QString outputContent = content;
    if (format == OutputPane::OutputFormat::NormalMessage) {
        QTextDocument *doc = d->outputPane->document();
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
    OutputPane::AppendMode mode = OutputPane::AppendMode::Normal;
    d->outputPane->appendText(outputContent, format, mode);
}

void DAPDebugger::handleFrames(const StackFrames &stackFrames)
{
    d->stackModel.setFrames(stackFrames);

    auto curFrame = d->stackModel.currentFrame();
    if (curFrame.line == -1) {
        // none of frame in model.
        return;
    }

    //frame changed  reset localsmodel
    if (d->currentValidFrame.function != curFrame.function || d->currentValidFrame.module != curFrame.module)
        d->localsModel.clear();
    d->currentValidFrame = curFrame;

    if (d->getLocalsFuture.isRunning())
        d->getLocalsFuture.cancel();

    // update local variables.
    d->processingVariablesTimer.start(50);   // if processing time < 50ms, do not show spinner
    d->processingVariablesCount.ref();
    d->getLocalsFuture = QtConcurrent::run([=]() {
        IVariables locals;
        getLocals(curFrame.frameId, &locals);
        d->localsModel.clearHighlightItems();
        d->localsModel.setDatas(locals);
        d->processingVariablesCount.deref();
        updateWatchingVariables();

        emit processingVariablesDone();
    });
}

void DAPDebugger::updateThreadList(int curr, const dap::array<dap::Thread> &threads)
{
    d->threadSelector->clear();
    int currIdx = -1;
    for (const auto &e : threads) {
        QString itemText = "#" + QString::number(e.id) + " " + e.name.c_str();
        d->threadSelector->addItem(itemText);
        if (curr == e.id)
            currIdx = d->threadSelector->count() - 1;
    }
    if (currIdx != -1)
        d->threadSelector->setCurrentIndex(currIdx);
    else if (!threads.empty()) {
        d->threadSelector->setCurrentIndex(0);
        int threadId = static_cast<int>(threads.at(0).id);
        switchCurrentThread(threadId);
    }
}

void DAPDebugger::switchCurrentThread(int threadId)
{
    auto thread = d->currentSession->getThread(threadId);
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
                if (d->isRemote)
                    sf.file = transformRemotePath(sf.file);
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

QHBoxLayout *DAPDebugger::initFrameTitle(const QString &frameName)
{
    if (frameName.isNull())
        return nullptr;

    DPushButton *arrow = new DPushButton();
    arrow->setFocusPolicy(Qt::NoFocus);
    arrow->setIcon(QIcon::fromTheme("go-down"));
    arrow->setFixedSize(QSize(18, 18));
    arrow->setIconSize(QSize(8, 8));
    arrow->setFlat(true);

    DLabel *stackTitleText = new DLabel();
    stackTitleText->setText(frameName);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(10, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(arrow);
    layout->addWidget(stackTitleText);

    return layout;
}

bool DAPDebugger::showStoppedBySignalMessageBox(QString meaning, QString name)
{
    if (d->alertBox)
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

    d->alertBox = Internal::information(tr("Signal Received"), msg);
    return true;
}

void DAPDebugger::slotFrameSelected()
{
    // update local variables.
    d->processingVariablesTimer.start(50);
    d->processingVariablesCount.ref();
    auto curFrame = d->stackModel.currentFrame();
    QtConcurrent::run([=]() {
        IVariables locals;
        getLocals(curFrame.frameId, &locals);
        d->localsModel.clearHighlightItems();
        d->localsModel.setDatas(locals);
        d->processingVariablesCount.deref();
        emit processingVariablesDone();
    });
}

void DAPDebugger::slotBreakpointSelected(const QModelIndex &index)
{
    Q_UNUSED(index);
    auto curBP = d->breakpointModel.currentBreakpoint();
    editor.gotoLine(curBP.filePath, curBP.lineNumber - 1);
}

void DAPDebugger::slotGetChildVariable(const QModelIndex &index)
{
    auto treeItem = static_cast<LocalTreeItem *>(index.internalPointer());
    if (!treeItem->canFetchChildren() || (!d->localsView->isExpanded(index) && !d->watchsView->isExpanded(index)))
        return;

    treeItem->setChildrenFetched(true);

    if (treeItem->childReference() == 0) {   //clear child variables
        emit childVariablesUpdated(treeItem, {});
        return;
    }

    d->processingVariablesTimer.start(50);
    d->processingVariablesCount.ref();
    QtConcurrent::run([=]() {
        IVariables variables;
        d->currentSession->getVariables(treeItem->childReference(), &variables, 0);

        d->processingVariablesCount.deref();
        emit childVariablesUpdated(treeItem, variables);
        emit processingVariablesDone();
    });
}

void DAPDebugger::slotEvaluateWatchVariable()
{
    DDialog dialog;
    DLineEdit *edit = new DLineEdit(d->watchsView);
    dialog.setWindowTitle(tr("New Evaluator Expression"));
    dialog.setMessage(tr("Enter an expression to evaluate"));
    dialog.addContent(edit);
    dialog.insertButton(0, tr("Cancel"));
    dialog.insertButton(1, tr("Ok"));

    if (dialog.exec() == 1) {
        QString expression = edit->text();
        evaluateWatchVariable(expression);
    }
}

void DAPDebugger::slotRemoveEvaluator()
{
    auto rows = d->watchsView->selectionModel()->selectedRows();
    auto expression = d->watchsModel.index(rows.at(0).row(), 0).data().toString();
    d->watchingVariables.remove(expression);
    auto vars = d->watchingVariables.values().toVector();
    d->watchsModel.setDatas(vars);
}

void DAPDebugger::initializeView()
{
    // initialize output pane.
    d->outputPane = OutputPane::instance();

    // initialize stack monitor pane.
    d->stackPane = new DWidget;
    d->stackPane->setMinimumWidth(300);
    QVBoxLayout *vLayout = new QVBoxLayout(d->stackPane);
    vLayout->setContentsMargins(0, 6, 0, 0);
    d->stackPane->setLayout(vLayout);

    d->stackView = new StackFrameView(d->stackPane);
    d->stackView->setModel(d->stackModel.model());

    d->threadSelector = new DComboBox(d->stackPane);
    d->threadSelector->setMinimumWidth(240);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    connect(d->threadSelector, QOverload<const QString &>::of(&DComboBox::activated), this, &DAPDebugger::currentThreadChanged);
#else
    connect(d->threadSelector, &DComboBox::textActivated, this, &DAPDebugger::currentThreadChanged);
#endif

    QHBoxLayout *hLayout = new QHBoxLayout(d->stackPane);
    hLayout->setAlignment(Qt::AlignLeft);
    hLayout->setContentsMargins(10, 0, 0, 0);
    DLabel *label = new DLabel(tr("Threads:"), d->stackPane);
    hLayout->addWidget(label);
    hLayout->addWidget(d->threadSelector);

    vLayout->addLayout(initFrameTitle(tr("Stack List")));
    vLayout->addLayout(hLayout);
    vLayout->addWidget(d->stackView);

    // intialize breakpint pane.
    d->breakpointView = new BreakpointView(d->stackPane);
    d->breakpointView->setMinimumWidth(300);
    d->breakpointView->setModel(d->breakpointModel.model());

    initializeVairablesPane();

    connect(&d->processingVariablesTimer, &QTimer::timeout, this, [=]() {
        d->variablesSpinner->show();
        d->variablesSpinner->raise();
        d->variablesSpinner->move(d->localsView->width() / 2 - d->variablesSpinner->width(), d->localsView->height() / 3);
    });
    connect(this, &DAPDebugger::processingVariablesDone, this, [=]() {
        if (d->processingVariablesCount != 0)
            return;
        d->processingVariablesTimer.stop();
        d->variablesSpinner->hide();
        handleUpdateDebugLine();
    });

    d->debugMainPane = new DFrame;
    d->debugMainPane->setLineWidth(0);
    DStyle::setFrameRadius(d->debugMainPane, 0);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(d->stackPane);
    mainLayout->addWidget(new DHorizontalLine());
    mainLayout->addLayout(initFrameTitle(tr("Breakpoint List")));
    mainLayout->addWidget(new DHorizontalLine());
    mainLayout->addWidget(d->breakpointView);
    d->debugMainPane->setLayout(mainLayout);

    connect(&d->stackModel, &StackFrameModel::currentIndexChanged, this, &DAPDebugger::slotFrameSelected);
    connect(d->breakpointView, &QTreeView::doubleClicked, this, &DAPDebugger::slotBreakpointSelected);

    connect(d->localsView, &QTreeView::expanded, this, &DAPDebugger::slotGetChildVariable);
    connect(this, &DAPDebugger::childVariablesUpdated, d->localsView, [=](LocalTreeItem *treeItem, IVariables vars) {
        d->localsModel.appendItem(treeItem, vars);
    });
    connect(&d->localsModel, &LocalTreeModel::updateChildVariables, this, &DAPDebugger::slotGetChildVariable);

    connect(d->watchsView, &QTreeView::expanded, this, &DAPDebugger::slotGetChildVariable);
    connect(this, &DAPDebugger::childVariablesUpdated, d->watchsView, [=](LocalTreeItem *treeItem, IVariables vars) {
        d->watchsModel.appendItem(treeItem, vars);
    });
    connect(&d->watchsModel, &LocalTreeModel::updateChildVariables, this, &DAPDebugger::slotGetChildVariable);
}

void DAPDebugger::initializeVairablesPane()
{
    d->variablesPane = new DSplitter;
    d->localsView = new DTreeView(d->variablesPane);
    d->localsView->setModel(&d->localsModel);
    d->localsView->setUniformRowHeights(true);
    d->localsView->setItemDelegate(new BaseItemDelegate(this));

    d->watchsView = new DTreeView(d->variablesPane);
    d->watchsView->setModel(&d->watchsModel);
    d->watchsView->setUniformRowHeights(true);
    d->watchsView->setItemDelegate(new BaseItemDelegate(this));
    d->variablesPane->addWidget(d->localsView);
    d->variablesPane->addWidget(d->watchsView);
    d->variablesPane->setOrientation(Qt::Vertical);
    d->variablesPane->setChildrenCollapsible(false);

    d->watchsView->setContextMenuPolicy(Qt::CustomContextMenu);
    d->watchsView->setSelectionBehavior(QAbstractItemView::SelectRows);

    DMenu *menu = new DMenu(d->watchsView);
    QAction *evaluateWatchVariable = new QAction(tr("Add New Expression Evaluator"), d->watchsView);
    QAction *remove = new QAction(tr("Remove This Evaluator"), d->watchsView);

    menu->addAction(evaluateWatchVariable);
    menu->addAction(remove);
    connect(d->watchsView, &DTreeView::customContextMenuRequested, this, [=](const QPoint &pos) {
        auto index = d->watchsView->indexAt(pos);
        if (index.isValid() && d->watchsView->selectionModel()->selectedRows().size() == 1)
            remove->setEnabled(true);
        else
            remove->setEnabled(false);
        menu->exec(d->watchsView->mapToGlobal(pos));
    });

    connect(evaluateWatchVariable, &QAction::triggered, this, &DAPDebugger::slotEvaluateWatchVariable);
    connect(remove, &QAction::triggered, this, &DAPDebugger::slotRemoveEvaluator);

    QStringList headers { tr("Name"), tr("Value"), tr("Type") /*, "Reference" */ };
    d->localsModel.setHeaders(headers);
    d->watchsModel.setHeaders(headers);

    d->variablesSpinner = new DSpinner(d->localsView);
    d->variablesSpinner->setFixedSize(30, 30);
    d->variablesSpinner->start();
    d->variablesSpinner->hide();
}

void DAPDebugger::updateWatchingVariables()
{
    if (d->watchingVariables.isEmpty())
        return;

    for (auto exp : d->watchingVariables.keys())
        evaluateWatchVariable(exp);
}

void DAPDebugger::exitDebug()
{
    // Change UI.
    editor.removeDebugLine();
    d->watchsModel.clear();
    d->localsModel.clear();
    d->currentDebugKit.clear();

    d->localsModel.clear();
    d->stackModel.removeAll();

    d->threadId = 0;

    d->threadSelector->clear();
}

void DAPDebugger::updateRunState(DAPDebugger::RunState state)
{
    if (d->runState != state) {
        d->runState = state;
        switch (state) {
        case kNoRun:
            exitDebug();
            AppOutputPane::instance()->setProcessFinished("debugPane");
            d->debugState = Normal;
            break;
        case kRunning:
        case kCustomRunning:
            d->pausing = false;
            break;
        case kStopped:
            break;
        default:
            // do nothing.
            break;
        }
        emit runStateChanged(d->runState);
    }
}

QString DAPDebugger::requestBuild()
{
    QString buildUuid;
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
        if (generator) {
            buildUuid = generator->build(getActiveProjectInfo().workspaceFolder());
        }
    }

    return buildUuid;
}

void DAPDebugger::start()
{
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
        if (generator) {
            if (!generator->isTargetReady()) {
                printOutput(tr("Please build first.\n Build : Ctrl + B"), OutputPane::ErrorMessage);
                return;
            }
            prepareDebug();
        }
    }
}

void DAPDebugger::prepareDebug()
{
    auto runState = getRunState();
    if (runState == kRunning) {
        printOutput(tr("Is preparing dependence, please waiting for a moment"));
        return;
    }

    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectKitName);
        if (generator) {
            updateRunState(kStart);
            QString retMsg;
            QMap<QString, QVariant> param;
            if (!d->isRemote)
                param = generator->getDebugArguments(getActiveProjectInfo(), d->currentOpenedFileName);

            bool ret = generator->prepareDebug(param, retMsg);
            if (!ret) {
                printOutput(retMsg, OutputPane::ErrorMessage);
                updateRunState(kPreparing);
            } else if (!generator->isAnsyPrepareDebug() && !d->isRemote) {
                requestDebugPort(param, d->activeProjectKitName, false);
            }
        }
    }
}

bool DAPDebugger::requestDebugPort(const QMap<QString, QVariant> &param, const QString &kitName, bool customDap)
{
    if (d->runState == kRunning) {
        printOutput(tr("Is getting the dap port, please waiting for a moment"));
        return false;
    }

    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(kitName);
        if (generator) {
            d->isCustomDap = customDap;
            QString retMsg;
            d->requestDAPPortPpid = QString::number(getpid());
            printOutput(tr("Requesting debug port..."));
            if (!generator->requestDAPPort(d->requestDAPPortPpid, param, retMsg)) {
                printOutput(retMsg, OutputPane::ErrorMessage);
                stopWaitingDebugPort();
                return false;
            }
        }
    }

    return true;
}

void DAPDebugger::stopDAP()
{
    updateRunState(kNoRun);
    d->currentSession->closeSession();
}

void DAPDebugger::slotReceivedDAPPort(const QString &ppid, int port, const QString &kitName, const QMap<QString, QVariant> &param)
{
    if (d->requestDAPPortPpid == ppid)
        launchSession(port, param, kitName);
}

void DAPDebugger::slotOutputMsg(const QString &title, const QString &msg)
{
    OutputPane::OutputFormat format = OutputPane::OutputFormat::Debug;
    if (title == "stdErr") {
        format = OutputPane::OutputFormat::StdErr;
    } else if (title == "stdOut") {
        format = OutputPane::OutputFormat::StdOut;
    } else if (title == "normal") {
        format = OutputPane::OutputFormat::NormalMessage;
    }
    bool isDetail = dpfGetService(ProjectService)->getActiveProjectInfo().detailInformation();
    if (isDetail || title == "stdErr") {
        printOutput(msg, format);
    }
}

void DAPDebugger::launchBackend()
{
    // launch backend by client.
    if (d->backend.isOpen())
        return;

    QString toolPath = CustomPaths::global(CustomPaths::Tools);
    QString backendPath = toolPath + QDir::separator() + "debugadapter";

    d->backend.setProgram(backendPath);
    d->backend.start();
    d->backend.waitForStarted();
}

void DAPDebugger::stopWaitingDebugPort()
{
    updateRunState(kPreparing);
}

void DAPDebugger::launchSession(int port, const QMap<QString, QVariant> &param, const QString &kitName)
{
    if (!port) {
        printOutput(tr("\nThe dap port is not ready, please retry.\n"), OutputPane::OutputFormat::ErrorMessage);
        return;
    }

    printOutput(tr("Debugging starts"));
    QString launchTip = tr("Launch dap session with port %1 ...")
                                .arg(port);
    printOutput(launchTip);

    d->currentDebugKit = kitName;
    auto iniRequet = d->rtCfgProvider->initalizeRequest();
    bool bSuccess = false;
    if (!d->isRemote)
        bSuccess = d->currentSession->initialize(d->rtCfgProvider->ip(),
                                                 port,
                                                 iniRequet);
    else
        bSuccess = d->currentSession->initialize(param.value("ip").toString().toLatin1(),
                                                 port,
                                                 iniRequet);

    if (!bSuccess) {
        updateRunState(DAPDebugger::RunState::kNoRun);
        notify(2, tr("Debugging service initialization failed"));
        qCritical() << "startDebug failed!";
        return;
    }

    // Launch debuggee.
    switch (d->debugState) {
    case Normal: {
        auto &ctx = dpfInstance.serviceContext();
        LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
        if (service) {
            auto generator = service->create<LanguageGenerator>(kitName);
            if (generator) {
                if (generator->isLaunchNotAttach()) {
                    dap::LaunchRequest request = generator->launchDAP(param);
                    bSuccess &= d->currentSession->launch(request);
                } else {
                    dap::AttachRequest request = generator->attachDAP(port, param);
                    bSuccess &= d->currentSession->attach(request);
                }
            }
        } else {
            bSuccess &= false;
        }
        break;
    }
    case Attaching: {
        dap::object obj;
        obj["processId"] = param.value("targetPath").toString().toStdString();
        dap::AttachRequest request;
        request.name = kitName.toStdString();   //kitName: gdb
        request.connect = obj;
        bSuccess &= d->currentSession->attach(request);
        break;
    }
    case Reverse: {
        dap::LaunchRequest request;
        request.name = "rr";
        request.type = "cppdbg";
        request.request = "launch";
        request.program = "";   // targetPath.toStdString();
        request.stopAtEntry = false;
        dap::array<dap::string> arrayArg;
        foreach (QString arg, param["arguments"].toStringList()) {
            arrayArg.push_back(arg.toStdString());
        }
        request.args = arrayArg;
        request.externalConsole = false;
        request.MIMode = "gdb";
        request.__sessionId = QUuid::createUuid().toString().toStdString();
        bSuccess &= d->currentSession->launch(request);
    }
    default:
        break;
    }

    if (!bSuccess) {
        updateRunState(DAPDebugger::RunState::kNoRun);
        notify(2, tr("Debugger startup failed"));
        qCritical() << "startDebug failed!";
    } else {
        debugService->getModel()->clear();
        debugService->getModel()->addSession(d->currentSession);
        AppOutputPane::instance()->setProcessStarted("debugPane");
    }
}

void DAPDebugger::currentThreadChanged(const QString &text)
{
    QtConcurrent::run([&]() {   // run in thread to avoid blocked when get variables.
        QStringList l = text.split("#");
        QString threadNumber = l.last().split(" ").first();
        switchCurrentThread(threadNumber.toInt());
    });
}

bool DAPDebugger::runCoredump(const QString &target, const QString &core, const QString &kit)
{
    launchBackend();

    updateRunState(kStart);
    updateRunState(kNoRun);
    printOutput(tr("Start debugging coredump file: ") + core + " with " + target);

    if (target.isEmpty() || !QFileInfo(target).isFile()) {
        QString tipMsg = tr("The coredump target file is error: ") + target;
        printOutput(tipMsg, OutputPane::ErrorMessage);
        return false;
    }

    if (core.isEmpty() || !QFileInfo(core).isFile()) {
        QString tipMsg = tr("The coredump file is error: ") + core;
        printOutput(tipMsg, OutputPane::ErrorMessage);
        return false;
    }

    QMap<QString, QVariant> param;
    param.insert("targetPath", target);
    param.insert("arguments", QStringList { core });
    d->userKitName = kit;

    return requestDebugPort(param, d->userKitName, true);
}

void DAPDebugger::disassemble(const QString &address)
{
    if (d->runState == kCustomRunning || d->runState == kStopped) {
        d->currentSession->disassemble(address.toStdString());
    }
}

void DAPDebugger::handleAssemble(const QString &content)
{
    if (content.isEmpty())
        return;

    QString assemblerPath = CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString("Disassembler");

    QFile file(assemblerPath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << content;
        file.close();
        // avoid load manually.
        editor.setModifiedAutoReload(assemblerPath, true);

        editor.gotoLine(assemblerPath, 0);
    }
}

ProjectInfo DAPDebugger::getActiveProjectInfo() const
{
    return dpfGetService(ProjectService)->getActiveProjectInfo();
}

void DAPDebugger::handleUpdateDebugLine()
{
    auto curFrame = d->stackModel.currentFrame();
    if (curFrame.line != -1) {
        QString localFile;
        localFile = d->isRemote ? transformRemotePath(curFrame.file) : curFrame.file;

        if (QFileInfo(localFile).exists()) {
            editor.setDebugLine(localFile, curFrame.line - 1);
        } else if (!curFrame.address.isEmpty()) {
            disassemble(curFrame.address);
        }
    }
}

QString DAPDebugger::transformRemotePath(const QString &remotePath)
{
    if (!d->isRemote || d->remoteInfo.projectPath.isEmpty())
        return remotePath;

    auto prjInfo = getActiveProjectInfo();
    auto workSpace = prjInfo.workspaceFolder();

    QString ret = remotePath;
    if (remotePath.startsWith(d->remoteInfo.projectPath)) {
        ret.replace(0, d->remoteInfo.projectPath.size(), workSpace);

        QFileInfo info(ret);
        if (!info.exists())
            qWarning() << ret << " is not exists!";
    }
    return ret;
}
