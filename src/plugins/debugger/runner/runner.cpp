// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "runner.h"

#include "base/baseitemdelegate.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"
#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/builder/builderservice.h"
#include "services/window/windowservice.h"
#include "services/editor/editorservice.h"
#include "services/terminal/terminalservice.h"

#include <DComboBox>
#include <DGuiApplicationHelper>
#include <DDialog>

#include <QMenu>
#include <QTextBlock>
#include <QLineEdit>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

class RunnerPrivate
{
    friend class Runner;
    QString currentBuildUuid;
    QString currentOpenedFilePath;
    QSharedPointer<QAction> runAction;
    DComboBox* runProgram;
    bool isRunning = false;
};

Runner::Runner(QObject *parent)
    : QObject(parent)
    , d(new RunnerPrivate())
{
    connect(debuggerSignals, &DebuggerSignals::receivedEvent, this, &Runner::handleEvents);

    d->runAction.reset(new QAction(MWMDA_RUNNING));
    d->runAction.get()->setIcon(QIcon::fromTheme("run"));
    connect(d->runAction.get(), &QAction::triggered, this, &Runner::run);

    auto cmd = ActionManager::instance()->registerAction(d->runAction.get(), "Debug.Running");
    cmd->setDefaultKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_F5);
    WindowService *service = dpfGetService(WindowService);
    service->addTopToolItem(cmd, false, Priority::high);
    
    d->runProgram = new DComboBox;
    d->runProgram->setFixedSize(200, 36);
    d->runProgram->setIconSize(QSize(16, 16));

    auto pal = d->runProgram->palette();
    pal.setColor(QPalette::Light, pal.color(QPalette::Base));
    pal.setColor(QPalette::Dark, pal.color(QPalette::Base));
    d->runProgram->setPalette(pal);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=](){
        auto pal = d->runProgram->palette();
        pal.setColor(QPalette::Light, pal.color(QPalette::Base));
        pal.setColor(QPalette::Dark, pal.color(QPalette::Base));
        d->runProgram->setPalette(pal);
    });

    DWidget *spacer = new DWidget(d->runProgram);
    spacer->setFixedWidth(10);

    service->addWidgetToTopTool(new AbstractWidget(spacer), false, true, Priority::highest + 1);
    service->addWidgetToTopTool(new AbstractWidget(d->runProgram), false, true, Priority::highest);
    connect(d->runProgram, &DComboBox::currentTextChanged, this, [=](const QString &text){
        auto prjInfo = getActiveProjectInfo();
        prjInfo.setCurrentProgram(text);
        dpfGetService(ProjectService)->updateProjectInfo(prjInfo);
        project.projectUpdated(prjInfo);
    });
}

Runner::~Runner()
{
    delete d;
}

void Runner::run()
{
    // save all modified files before running.
    dpfGetService(EditorService)->saveAll();

    LanguageService *service = dpfGetService(LanguageService);
    if (service) {
        auto generator = service->create<LanguageGenerator>(getActiveProjectInfo().kitName());
        if (generator) {
            if (generator->isNeedBuild()) {
                d->currentBuildUuid = generator->build(getActiveProjectInfo().workspaceFolder());
            } else {
                running();
            }
        } else {
            auto windowService = dpfGetService(WindowService);
            windowService->notify(1, tr("Warning"), tr("The project does not have an associated build kit. Please reopen the project and select the corresponding build tool."), {});
        }
    }
}

void Runner::handleEvents(const dpf::Event &event)
{
    QString topic = event.topic();
    QString data = event.data().toString();
    if (topic == T_BUILDER) {
        if (data == D_BUILD_STATE) {
            int state = event.property(P_STATE).toInt();
            BuildCommandInfo commandInfo = qvariant_cast<BuildCommandInfo>(event.property(P_ORIGINCMD));
            if (commandInfo.uuid == d->currentBuildUuid) {
                int buildSuccess = 0;
                if (state == buildSuccess) {
                    running();
                }
            }
        }
    } else if (event.data() == editor.switchedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        if (d->currentOpenedFilePath != filePath) {
            d->currentOpenedFilePath = filePath;
        }
    } else if (event.data() == editor.fileOpened.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        d->currentOpenedFilePath = filePath;
    } else if (event.data() == editor.fileClosed.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        if (d->currentOpenedFilePath == filePath) {
            d->currentOpenedFilePath.clear();
        }
    } else if (event.data() == project.activatedProject.name) {
        QVariant proInfoVar = event.property("projectInfo");
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        auto programs = projectInfo.exePrograms();
        d->runProgram->clear();
        for (auto program : programs)
            d->runProgram->addItem(QIcon::fromTheme("executable"), program);
        d->runProgram->setCurrentText(projectInfo.currentProgram());
    }
}

void Runner::running()
{
    if(d->isRunning) {
        outputMsg(0, "The program is running, please try again later!", OutputPane::OutputFormat::ErrorMessage);
        return;
    }

    LanguageService *service = dpfGetService(LanguageService);
    if (service) {
        auto generator = service->create<LanguageGenerator>(getActiveProjectInfo().kitName());
        if (generator) {
            dpfservice::ProjectInfo activeProjInfo = dpfGetService(ProjectService)->getActiveProjectInfo();
            RunCommandInfo args = generator->getRunArguments(activeProjInfo, d->currentOpenedFilePath);
            if (!args.runInTerminal) {
                uiController.switchContext(tr("&Application Output"));

                QtConcurrent::run([=]() {
                    execCommand(args);
                });
            } else {
                uiController.switchContext(TERMINAL_TAB_TEXT);

                auto terminalService = dpfGetService(TerminalService);
                terminalService->executeCommand(args.program, args.program, args.arguments, args.workingDir, args.envs);
            }
        }
    }
}

bool Runner::execCommand(const RunCommandInfo &info)
{
    bool ret = false;
    QString retMsg = tr("Error: execute command error! The reason is unknown.\n");
    QProcess process;
    process.setWorkingDirectory(info.workingDir);
    process.setEnvironment(info.envs);

    QString startMsg = tr("Start execute command: \"%1\" \"%2\" in workspace \"%3\".\n")
            .arg(info.program, info.arguments.join(" "), info.workingDir);

    connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
        if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
            ret = true;
            retMsg = tr("The process \"%1\" exited normally.\n").arg(process.program());
        } else if (exitStatus == QProcess::NormalExit) {
            ret = false;
            retMsg = tr("The process \"%1\" exited with code %2.\n")
                           .arg(process.program(), QString::number(exitcode));
        } else {
            ret = false;
            retMsg = tr("The process \"%1\" crashed.\n")
                           .arg(process.program());
        }
    });

    connect(&process, &QProcess::readyReadStandardOutput, [&]() {
        process.setReadChannel(QProcess::StandardOutput);
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine());
            outputMsg(process.processId(), line, OutputPane::OutputFormat::StdOut);
        }
    });

    connect(&process, &QProcess::readyReadStandardError, [&]() {
        process.setReadChannel(QProcess::StandardError);
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine());
            outputMsg(process.processId(), line, OutputPane::OutputFormat::StdErr);
        }
    });

    process.start(info.program, info.arguments);
    quint64 pid = process.processId();
    QMetaObject::invokeMethod(AppOutputPane::instance(), "createApplicationPane",
                              Q_ARG(const QString &, QString::number(pid)), Q_ARG(QString, info.program));
    outputMsg(pid, startMsg, OutputPane::OutputFormat::NormalMessage);
    process.waitForFinished(-1);

    AppOutputPane::instance()->setProcessFinished(QString::number(pid));

    outputMsg(pid, retMsg, ret ? OutputPane::OutputFormat::NormalMessage : OutputPane::OutputFormat::StdErr);

    QString endMsg = tr("Execute command finished.\n");
    outputMsg(pid, endMsg, OutputPane::OutputFormat::NormalMessage);

    return ret;
}

void Runner::outputMsg(const quint64 &pid, const QString &content, OutputPane::OutputFormat format)
{
    QMetaObject::invokeMethod(this, "synOutputMsg", Q_ARG(quint64, pid), Q_ARG(QString, content), Q_ARG(OutputPane::OutputFormat, format));
}

ProjectInfo Runner::getActiveProjectInfo() const
{
    return dpfGetService(ProjectService)->getActiveProjectInfo();
}

void Runner::synOutputMsg(const quint64 &pid, const QString &content, OutputPane::OutputFormat format)
{
    auto outputPane = AppOutputPane::instance()->getOutputPaneById(QString::number(pid));
    QString outputContent = content;
    if (format == OutputPane::OutputFormat::NormalMessage) {
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
    OutputPane::AppendMode mode = OutputPane::AppendMode::Normal;
    outputPane->appendText(outputContent, format, mode);
}

