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
#include "event/eventsender.h"
#include "event/eventreceiver.h"
#include "common/common.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"
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

/**
 * @brief Debugger::Debugger
 * For serial debugging service
 */
using namespace dap;
using namespace DEBUG_NAMESPACE;
using namespace dpfservice;

Debugger::Debugger(QObject *parent)
    : QObject(parent)
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
                          "send_java_dapport",
                          this, SLOT(slotReceiveJavaDAPPort(int, QString, QString, QStringList)));
    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "send_java_dapport",
                       this, SLOT(slotReceiveJavaDAPPort(int, QString, QString, QStringList)));

    connect(this, &Debugger::sigJavaLSPPluginReady, this, &Debugger::slotJavaLSPPluginReady);
    connect(this, &Debugger::sigJavaDAPPluginReady, this, &Debugger::slotJavaDAPPluginReady);
    connect(this, &Debugger::sigJavaDAPPort, this, &Debugger::slotHandleJavaDAPPort);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, [this]() {
        emit sigJavaDAPPort(0, "", "", QStringList{});
    });

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
    // build project everytime before debug.
    if (!isCMakeProject(activeProjectKitName)
            && !isJavaProject(activeProjectKitName)) //temporary, TODO for python
        return;

    updateRunState(kPreparing);
    requestBuild();
    // start debug when received build success event in handleFrameEvent().
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
        EventSender::clearEditorPointer();
    }
}

void Debugger::abortDebug()
{
    if (runState == kRunning || runState == kStopped) {
        if (isJavaProject(activeProjectKitName))
            stopJavaDAP();
        else if(isCMakeProject(activeProjectKitName))
            session->terminate();
    }
}

void Debugger::restartDebug()
{
    if (runState == kStopped || runState == kRunning) {
        if (isJavaProject(activeProjectKitName))
            restartJavaDAP();
        else if(isCMakeProject(activeProjectKitName))
            session->restart();
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
        session.get()->getRawSession()->setReadyForBreakpoints(true);
        debugService->sendAllBreakpoints(session.get());
        session.get()->getRawSession()->configurationDone().wait();
        session->fetchThreads(nullptr);

        updateRunState(Debugger::RunState::kRunning);
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
                || event.reason == "end-stepping-range") {
            if (event.threadId) {
                threadId = event.threadId.value(0);
                switchCurrentThread(static_cast<int>(threadId));
            }
            updateRunState(Debugger::RunState::kStopped);
        } else if (event.reason == "exception" || event.reason == "signal-received") {
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
    if (topic == T_CODEEDITOR) {
        // breakpoint process.
        QString filePath = event.property(P_FILEPATH).toString();
        int lineNumber = event.property(P_FILELINE).toInt();
        if (data == D_MARGIN_DEBUG_POINT_ADD) {
            addBreakpoint(filePath, lineNumber);
        } else if (data == D_MARGIN_DEBUG_POINT_REMOVE) {
            removeBreakpoint(filePath, lineNumber);
        }
    } else if (topic == T_BUILDER) {
        if (data == D_BUILD_STATE) {
            int state = event.property(P_STATE).toInt();
            BuildCommandInfo commandInfo = qvariant_cast<BuildCommandInfo>(event.property(P_ORIGINCMD));
            if (commandInfo.uuid == currentBuildUuid) {
                int buildSuccess = 0;
                if (state == buildSuccess && runState == kPreparing)
                    start();
            }
        }
    } else if (event.topic() == T_PROJECT) {
        if (event.data() == D_ACTIVED || event.data() == D_CRETED) {
            projectInfo = qvariant_cast<dpfservice::ProjectInfo>(event.property(P_PROJECT_INFO));
            activeProjectKitName = projectInfo.kitName();
            updateRunState(kNoRun);
        } else if (event.data() == D_DELETED){
            activeProjectKitName.clear();
            updateRunState(kNoRun);
        }
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

    EventSender::jumpTo(curFrame.file.toStdString(),
                        curFrame.line);

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
    Q_UNUSED(index);
    auto curFrame = stackModel.currentFrame();
    EventSender::jumpTo(curFrame.file.toStdString(),
                        curFrame.line,
                        projectInfo.workspaceFolder().toStdString(),
                        projectInfo.language().toStdString());

    // update local variables.
    IVariables locals;
    getLocals(curFrame.frameId, &locals);
    localsModel.setDatas(locals);
}

void Debugger::slotBreakpointSelected(const QModelIndex &index)
{
    Q_UNUSED(index);
    auto curBP = breakpointModel.currentBreakpoint();
    EventSender::jumpTo(curBP.filePath.toStdString(),
                        curBP.lineNumber,
                        projectInfo.workspaceFolder().toStdString(),
                        projectInfo.language().toStdString());
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
    EventSender::clearEditorPointer();
    QMetaObject::invokeMethod(localsView, "hide");

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

bool Debugger::isCMakeProject(const QString &kitName)
{
    return ("cmake" == kitName);
}

bool Debugger::isMavenProject(const QString &kitName)
{
    return ("maven" == kitName);
}

bool Debugger::isGradleProject(const QString &kitName)
{
    return ("gradle" == kitName);
}

bool Debugger::isJavaProject(const QString &kitName)
{
    return isMavenProject(kitName) || isGradleProject(kitName);
}

bool Debugger::checkTargetIsReady()
{
    targetPath.clear();

    if (isCMakeProject(activeProjectKitName)) {
        auto &ctx = dpfInstance.serviceContext();
        ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
        if (projectService && projectService->getActiveTarget) {
            auto target = projectService->getActiveTarget(kActiveExecTarget);
            targetPath = target.outputPath + QDir::separator() + target.path + QDir::separator() + target.buildTarget;
        }
    } else {
        targetPath = projectInfo.workspaceFolder();
    }

    return QFile::exists(targetPath);
}

void Debugger::requestBuild()
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (isCMakeProject(activeProjectKitName) && builderService && projectService
            && projectService->getActiveTarget) {
        auto target = projectService->getActiveTarget(TargetType::kBuildTarget);
        if (!target.buildCommand.isEmpty()) {
            QStringList args = target.buildArguments << target.buildTarget;
            BuildCommandInfo commandInfo;
            commandInfo.kitName = activeProjectKitName;
            commandInfo.program = target.buildCommand;
            commandInfo.arguments = args;
            commandInfo.workingDir = target.outputPath;

            currentBuildUuid = commandInfo.uuid;
            builderService->interface.builderCommand(commandInfo);
        }
    }

    if (isMavenProject(activeProjectKitName) && builderService) {
        BuildCommandInfo commandInfo;
        commandInfo.kitName = activeProjectKitName;
        commandInfo.program = OptionManager::getInstance()->getMavenToolPath();
        commandInfo.arguments = QStringList("compile");
        commandInfo.workingDir = projectInfo.workspaceFolder();

        currentBuildUuid = commandInfo.uuid;
        builderService->interface.builderCommand(commandInfo);
    }

    if (isGradleProject(activeProjectKitName) && builderService) {
        BuildCommandInfo commandInfo;
        commandInfo.kitName = activeProjectKitName;
        commandInfo.program = OptionManager::getInstance()->getGradleToolPath();
        commandInfo.arguments = QStringList("build");
        commandInfo.workingDir = projectInfo.workspaceFolder();

        currentBuildUuid = commandInfo.uuid;
        builderService->interface.builderCommand(commandInfo);
    }
}

void Debugger::start()
{
    if (!checkTargetIsReady()) {
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, "Please build first.\n Build : Ctrl + B"));
        return;
    }

    updateRunState(kStart);

    // Setup debug environment.
    printOutput(tr("Debugging starts"));

    if (isCMakeProject(activeProjectKitName)) {
        int port = rtCfgProvider->port();
        launchSession(port);
    }
    else if (isJavaProject(activeProjectKitName)) {
        if (!javaDapPluginFileReady) {
            QString dapSupportFilePath = support_file::DapSupportConfig::globalPath();
            bool ret = support_file::DapSupportConfig::readFromSupportFile(dapSupportFilePath, javaDapPluginConfig);
            if (!ret) {
                printOutput("\nRead dapconfig.support failed, please check the file and retry.\n", ErrorMessageFormat);
                QMetaObject::invokeMethod(this, "message",
                                          Q_ARG(QString, "Read dapconfig support file failed, please check dapconfig.support file and retry!"));
                updateRunState(kPreparing);
                return;
            }

            javaDapPluginFileReady = true;
        }
        checkJavaLSPPlugin();
    }
}

void Debugger::slotJavaLSPPluginReady(bool succeed)
{
    if (!succeed) {
        printOutput("\nJava lsp plugin is not ready, please retry.\n", ErrorMessageFormat);
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, "Java lsp dependent plugins is not ready, please check and retry."));
        updateRunState(kPreparing);
        return;
    }

    checkJavaDAPPlugin();
}

void Debugger::slotJavaDAPPluginReady(bool succeed)
{
    if (!succeed) {
        printOutput("\nJava dap plugin is not ready, please retry.\n", ErrorMessageFormat);
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, "Java dap dependent plugins is not ready, please check and retry."));
        updateRunState(kPreparing);
        return;
    }

    printOutput("\nSearching java dap port, please waiting...\n");
    waitHandleJavaDAPPort = true;
    if (timer)
        timer->start(10000); //10s timeout

    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "launch_java_dap");
    msg << projectInfo.workspaceFolder()
        << javaDapPluginConfig.configHomePath
        << javaDapPluginConfig.jrePath
        << javaDapPluginConfig.jreExecute
        << javaDapPluginConfig.launchPackageFile
        << javaDapPluginConfig.launchConfigPath
        << javaDapPluginConfig.dapPackageFile;
    QDBusConnection::sessionBus().send(msg);
}

void Debugger::slotReceiveJavaDAPPort(int port, const QString &mainClass, const QString &projectName, const QStringList &classPaths)
{
    if (waitHandleJavaDAPPort)
        emit sigJavaDAPPort(port, mainClass, projectName, classPaths);
}

void Debugger::slotHandleJavaDAPPort(int port, const QString &mainClass, const QString &projectName, const QStringList &classPaths)
{
    waitHandleJavaDAPPort = false;
    timer->stop();
    launchSession(port, mainClass, projectName, classPaths);
}

void Debugger::restartJavaDAP()
{
    stopJavaDAP();
    start();
}

void Debugger::stopJavaDAP()
{
    updateRunState(kNoRun);
    session->closeSession();
}

void Debugger::checkJavaLSPPlugin()
{
    QString configFolder = QDir::homePath() + javaDapPluginConfig.configHomePath;
    bool bLaunchJarPath = QFileInfo(configFolder + javaDapPluginConfig.launchPackageFile).isFile();
    bool bJrePath = QFileInfo(configFolder + javaDapPluginConfig.jreExecute).isFile();
    if (!bLaunchJarPath || !bJrePath) {
        FileOperation::deleteDir(configFolder + javaDapPluginConfig.launchPackageName);

        auto decompress = [&](const QString workDir, const QString srcTarget, const QString folder) {
            if (!QFileInfo(workDir).isDir()
                    || !QFileInfo(srcTarget).isFile()
                    || folder.isEmpty()) {
                emit sigJavaLSPPluginReady(false);
                return;
            }

            QProcess process;
            connect(&process, &QProcess::readyReadStandardError, [&]() {
                printOutput("\nDecompress " + srcTarget + " error: " + process.readAllStandardError(), ErrorMessageFormat);
            });
            connect(&process, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
                    [&](int exitcode, QProcess::ExitStatus exitStatus) {
                if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
                    emit sigJavaLSPPluginReady(true);
                } else {
                    QFile::remove(srcTarget);
                    emit sigJavaLSPPluginReady(false);
                }
            });
            process.setWorkingDirectory(workDir);
            process.start("unzip", QStringList{srcTarget, "-d", folder});
            process.waitForFinished();
        };

        QString redhatLspFilePath = configFolder + javaDapPluginConfig.launchPackageName + ".zip";
        if (QFileInfo(redhatLspFilePath).isFile()) {
            decompress(configFolder, redhatLspFilePath, javaDapPluginConfig.launchPackageName);
        } else {
            DownloadUtil *downloadUtil = new DownloadUtil(javaDapPluginConfig.launchPackageUrl,
                                                          configFolder,
                                                          javaDapPluginConfig.launchPackageName + ".zip");
            connect(downloadUtil, &DownloadUtil::sigProgress, [=](qint64 bytesRead, qint64 totalBytes){
                Q_UNUSED(totalBytes)
                QString progress = QString("received: %1 MB")
                        .arg(bytesRead/1024.00/1024.00);
                printOutput(progress);
            });
            connect(downloadUtil, &DownloadUtil::sigFinished, [=](){
                printOutput("Download java lsp plugin finished.");
                decompress(configFolder, redhatLspFilePath, javaDapPluginConfig.launchPackageName);
            });

            connect(downloadUtil, &DownloadUtil::sigFailed, [=](){
                printOutput("Download java lsp plugin failed, please retry!", ErrorMessageFormat);
                emit sigJavaLSPPluginReady(false);
            });

            printOutput("Downloading java lsp plugin, please waiting...");
            bool ret = downloadUtil->start();
            if (!ret) {
                printOutput("Download java lsp plugin failed, please retry!", ErrorMessageFormat);
                emit sigJavaLSPPluginReady(false);
            }
        }
    } else {
        emit sigJavaLSPPluginReady(true);
    }
}

void Debugger::checkJavaDAPPlugin()
{
    QString configFolder = QDir::homePath() + javaDapPluginConfig.configHomePath;
    bool bDapJarPath = QFileInfo(configFolder + javaDapPluginConfig.dapPackageFile).isFile();
    if (!bDapJarPath) {
        FileOperation::deleteDir(configFolder + javaDapPluginConfig.dapPackageName);

        auto decompress = [this](const QString workDir, const QString srcTarget, const QString folder) {
            if (!QFileInfo(workDir).isDir()
                    || !QFileInfo(srcTarget).isFile()
                    || folder.isEmpty()) {
                emit sigJavaLSPPluginReady(false);
                return;
            }

            QProcess process;
            connect(&process, &QProcess::readyReadStandardError, [&]() {
                printOutput("\nDecompress " + srcTarget + " error: " + process.readAllStandardError(), ErrorMessageFormat);
            });
            connect(&process, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
                    [&](int exitcode, QProcess::ExitStatus exitStatus) {
                if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
                    emit sigJavaDAPPluginReady(true);
                } else {
                    QFile::remove(srcTarget);
                    emit sigJavaDAPPluginReady(false);
                }
            });
            process.setWorkingDirectory(workDir);
            process.start("unzip", QStringList{srcTarget, "-d", folder});
            process.waitForFinished();
        };

        QString vsdapFilePath = configFolder + javaDapPluginConfig.dapPackageName + ".zip";
        if (QFileInfo(vsdapFilePath).isFile()) {
            decompress(configFolder, vsdapFilePath, javaDapPluginConfig.dapPackageName);
        } else {
            DownloadUtil *downloadUtil = new DownloadUtil(javaDapPluginConfig.dapPackageUrl,
                                                          configFolder,
                                                          javaDapPluginConfig.dapPackageName + ".zip");
            connect(downloadUtil, &DownloadUtil::sigProgress, [=](qint64 bytesRead, qint64 totalBytes){
                Q_UNUSED(totalBytes)
                QString progress = QString("received: %1 MB")
                        .arg(bytesRead/1024.00/1024.00);
                printOutput(progress);
            });

            connect(downloadUtil, &DownloadUtil::sigFinished, [=](){
                printOutput("Download java dap plugin finished.");
                decompress(configFolder, vsdapFilePath, "vscjava");
            });

            connect(downloadUtil, &DownloadUtil::sigFailed, [=](){
                printOutput("Download java dap plugin failed, please retry!", ErrorMessageFormat);
                emit sigJavaDAPPluginReady(false);
            });

            printOutput("Downloading java dap plugin, please waiting...");
            bool ret = downloadUtil->start();
            if (!ret) {
                printOutput("Download java dap plugin failed, please retry!", ErrorMessageFormat);
                emit sigJavaDAPPluginReady(false);
            }
        }
    } else {
        emit sigJavaDAPPluginReady(true);
    }
}

void Debugger::launchSession(const int port, const QString &mainClass/* = ""*/,
                             const QString &projectName/* = ""*/, const QStringList &classPaths/* = QStringList{}*/)
{
    if (!port) {
        printOutput("Java dap port is not ready, please retry.", ErrorMessageFormat);
        QMetaObject::invokeMethod(this, "message", Q_ARG(QString, "Could not find server port!"));
        updateRunState(kPreparing);
        return;
    }

    QString launchTip = QString("Launch java dap session with port %1 ...")
            .arg(port);
    printOutput(launchTip);

    auto iniRequet = rtCfgProvider->initalizeRequest();
    bool bSuccess = session->initialize(rtCfgProvider->ip(),
                                        port,
                                        iniRequet);

    // Launch debuggee.
    if (bSuccess) {
        if (isCMakeProject(activeProjectKitName))
            bSuccess &= session->launch(targetPath);
        else if (isJavaProject(activeProjectKitName))
            bSuccess &= session->launchJavaDap(projectInfo.workspaceFolder(),
                                               mainClass,
                                               projectName,
                                               classPaths,
                                               QDir::homePath() + javaDapPluginConfig.configHomePath + javaDapPluginConfig.jreExecute);
        else
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
