// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "runner.h"

#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "base/abstractmenu.h"
#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/builder/builderservice.h"
#include "services/window/windowservice.h"

#include <QMenu>
#include <QTextBlock>

using namespace dpfservice;

class RunnerPrivate
{
    friend class Runner;
    QString currentBuildUuid;
    QString currentOpenedFilePath;
    QSharedPointer<QAction> runAction;
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

    auto actionImpl = new AbstractAction(d->runAction.get(), this);
    actionImpl->setShortCutInfo("Debug.Running",
                                MWMDA_RUNNING, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_F5));
    WindowService *service = dpfGetService(WindowService);
    service->addTopToolItem(actionImpl, MWTG_EDIT, true);
}

Runner::~Runner()
{
    delete d;
}

void Runner::run()
{
    LanguageService *service = dpfGetService(LanguageService);
    if (service) {
        auto generator = service->create<LanguageGenerator>(getActiveProjectInfo().kitName());
        if (generator) {
            if (generator->isNeedBuild()) {
                d->currentBuildUuid = generator->build(getActiveProjectInfo().workspaceFolder());
            } else {
                running();
            }
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
    }
}

void Runner::running()
{
    if(d->isRunning) {
        outputMsg("The program is running, please try again later!", OutputPane::OutputFormat::ErrorMessage);
        return;
    }

    uiController.switchContext(tr("&Application Output"));

    LanguageService *service = dpfGetService(LanguageService);
    if (service) {
        auto generator = service->create<LanguageGenerator>(getActiveProjectInfo().kitName());
        if (generator) {
            dpfservice::ProjectInfo activeProjInfo = dpfGetService(ProjectService)->getActiveProjectInfo();
            RunCommandInfo args = generator->getRunArguments(activeProjInfo, d->currentOpenedFilePath);
            QtConcurrent::run([=](){
               execCommand(args);
            });
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
    outputMsg(startMsg, OutputPane::OutputFormat::NormalMessage);

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
            outputMsg(line, OutputPane::OutputFormat::StdOut);
        }
    });

    connect(&process, &QProcess::readyReadStandardError, [&]() {
        process.setReadChannel(QProcess::StandardError);
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine());
            outputMsg(line, OutputPane::OutputFormat::StdErr);
        }
    });

    process.start(info.program, info.arguments);
    process.waitForFinished(-1);

    outputMsg(retMsg, ret ? OutputPane::OutputFormat::NormalMessage : OutputPane::OutputFormat::StdErr);

    QString endMsg = tr("Execute command finished.\n");
    outputMsg(endMsg, OutputPane::OutputFormat::NormalMessage);

    return ret;
}

void Runner::outputMsg(const QString &content, OutputPane::OutputFormat format)
{
    QMetaObject::invokeMethod(this, "synOutputMsg", Q_ARG(QString, content), Q_ARG(OutputPane::OutputFormat, format));
}

ProjectInfo Runner::getActiveProjectInfo() const
{
    return dpfGetService(ProjectService)->getActiveProjectInfo();
}

void Runner::synOutputMsg(const QString &content, OutputPane::OutputFormat format)
{
    auto outputPane = OutputPane::instance();
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

