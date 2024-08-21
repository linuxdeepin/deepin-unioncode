// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "reversedebugger.h"
#include "rrdialog.h"
#include "common/actionmanager/actionmanager.h"
#include "common/actionmanager/actioncontainer.h"
#include "services/window/windowservice.h"
#include "services/window/windowelement.h"
#include "services/terminal/terminalservice.h"
#include "services/language/languageservice.h"

#include <QMenu>
#include <QTextBlock>
#include <QProcess>

constexpr char A_REVERSE_DEBUG_RECORD[] = "RverseDebug.Action.Record";
constexpr char A_REVERSE_DEBUG_REPLAY[] = "RverseDebug.Action.Replay";

ReverseDebugger::ReverseDebugger(QObject *parent)
    : QObject(parent)
{
    init();
}

void ReverseDebugger::init()
{
    auto mTools = ActionManager::instance()->actionContainer(M_TOOLS);
    auto mReverseDbg = ActionManager::instance()->createContainer(M_TOOLS_REVERSEDEBUG);
    mReverseDbg->menu()->setTitle(tr("Reverse debug"));
    mTools->addMenu(mReverseDbg);

    auto actionInit = [&](QAction *action, QString actionID) {
        auto cmd = ActionManager::instance()->registerAction(action, actionID);
        mReverseDbg->addAction(cmd);
    };

    auto recoredAction = new QAction(tr("Record"), mReverseDbg);
    actionInit(recoredAction, A_REVERSE_DEBUG_RECORD);
    auto replayAction = new QAction(tr("Replay"), mReverseDbg);
    actionInit(replayAction, A_REVERSE_DEBUG_REPLAY);

    connect(recoredAction, &QAction::triggered, this, &ReverseDebugger::record);
    connect(replayAction, &QAction::triggered, this, &ReverseDebugger::replay);
    connect(this, &ReverseDebugger::recordDone, this, [=]() {
        QMessageBox::information(nullptr, tr("Reverse Debug"), tr("Recored done."));
    });
    connect(this, &ReverseDebugger::recordFailed, this, [=](const QString &err) {
        QMessageBox::warning(nullptr, tr("Reverse Debug"), tr("Recored Failed.\n%1").arg(err));
    });
}

void ReverseDebugger::record()
{
    if (!checkRRInstalled())
        return;
    RecordDialog dialog;
    connect(&dialog, &RecordDialog::startRecord, this, [=](const dpfservice::RunCommandInfo &args) {
        QtConcurrent::run([=]() {
            execCommand(args);
        });
    });
    dialog.exec();
}

void ReverseDebugger::replay()
{
    if (!checkRRInstalled())
        return;
    ReplayDialog dialog;
    connect(&dialog, &ReplayDialog::startReplay, this, &ReverseDebugger::startReplay);
    dialog.exec();
}

bool ReverseDebugger::execCommand(const dpfservice::RunCommandInfo &info)
{
    bool ret = false;

    auto target = info.program;
    if (target.isEmpty() || !QFile::exists(target)) {
        emit recordFailed(tr("target not found!"));
        return ret;
    }

    QProcess process;
    process.setWorkingDirectory(info.workingDir);
    process.setEnvironment(info.envs);

    auto program = "rr";
    QStringList arguments;
    arguments.append("record");
    arguments.append(info.program);
    arguments.append(info.arguments);

    connect(&process, &QProcess::readyReadStandardOutput, [&]() {
        process.setReadChannel(QProcess::StandardOutput);
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine());
            outputMsg(process.pid(), line, OutputPane::OutputFormat::StdOut);
        }
    });

    connect(&process, &QProcess::readyReadStandardError, [&]() {
        process.setReadChannel(QProcess::StandardError);
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine());
            outputMsg(process.pid(), line, OutputPane::OutputFormat::StdErr);
        }
    });

    process.start(program, arguments);
    uiController.switchContext(tr("&Application Output"));
    quint64 pid = process.pid();
    QString startMsg = tr("Start execute command: \"%1\" \"%2\" in workspace \"%3\".\n")
                               .arg(program, arguments.join(" "), info.workingDir);
    QMetaObject::invokeMethod(AppOutputPane::instance(), "createApplicationPane",
                              Q_ARG(const QString &, QString::number(pid)), Q_ARG(QString, program));
    outputMsg(pid, startMsg, OutputPane::OutputFormat::NormalMessage);
    process.waitForFinished();

    AppOutputPane::instance()->setProcessFinished(QString::number(pid));

    emit recordDone();

    return ret;
}

void ReverseDebugger::outputMsg(const quint64 &pid, const QString &content, OutputPane::OutputFormat format)
{
    QMetaObject::invokeMethod(this, "synOutputMsg", Q_ARG(quint64, pid), Q_ARG(QString, content), Q_ARG(OutputPane::OutputFormat, format));
}

void ReverseDebugger::synOutputMsg(const quint64 &pid, const QString &content, OutputPane::OutputFormat format)
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

void installRR(const QString &id)
{
    if (id == "apt") {
        auto windowService = dpfGetService(dpfservice::WindowService);
        windowService->installPackages(ReverseDebugger::tr("Reverse debugger"), "apt", { "rr" }, nullptr);
    } else if (id == "wget") {
        auto terminalService = dpfGetService(dpfservice::TerminalService);
        auto command = "cd /tmp && wget https://github.com/rr-debugger/rr/releases/download/5.5.0/rr-5.5.0-Linux-x86_64.deb && sudo dpkg -i rr-5.5.0-Linux-x86_64.deb";
        uiController.switchContext(dpfservice::TERMINAL_TAB_TEXT);
        terminalService->sendCommand(command);
    }
}

bool ReverseDebugger::checkRRInstalled()
{
    QProcess process;
    process.start("rr");

    if (process.waitForStarted()) {
        process.close();
        return true;
    } else {
        auto windowService = dpfGetService(dpfservice::WindowService);
        windowService->notifyWithCallback(0, tr("Reverse Debug"), tr("Can not find rr debugger, please install it first"), QStringList{"apt", tr("Install by apt"), "wget", tr("Install by Wget")}, installRR);
        process.close();
        return false;
    }
}
