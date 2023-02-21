/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "runner.h"

#include "debuggersignals.h"
#include "debuggerglobals.h"
#include "base/abstractmenu.h"
#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/builder/builderservice.h"
#include "services/window/windowservice.h"

#include <QMenu>

using namespace dpfservice;

class RunnerPrivate
{
    friend class Runner;
    QString currentBuildUuid;
    ProjectInfo activeProjectInfo;
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
    ActionManager::getInstance()->registerAction(d->runAction.get(), "Debug.Running",
                                                 MWMDA_RUNNING, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_F5),
                                                 ":/resource/images/run.png");
    connect(d->runAction.get(), &QAction::triggered, this, &Runner::run);
    WindowService *service = dpfGetService(WindowService);
    service->addToolBarActionItem(tr("Running"), d->runAction.get(), "Debug");
}

void Runner::run()
{
    LanguageService *service = dpfGetService(LanguageService);
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectInfo.kitName());
        if (generator) {
            if (generator->isNeedBuild()) {
                d->currentBuildUuid = generator->build(d->activeProjectInfo.workspaceFolder());
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
    } else if (event.data() == project.activedProject.name) {
        d->activeProjectInfo = qvariant_cast<ProjectInfo>(event.property(project.activedProject.pKeys[0]));
    } else if (event.data() == editor.switchedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        if (d->currentOpenedFilePath != filePath) {
            d->currentOpenedFilePath = filePath;
        }
    } else if (event.data() == editor.openedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        d->currentOpenedFilePath = filePath;
    } else if (event.data() == editor.closedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        if (d->currentOpenedFilePath == filePath) {
            d->currentOpenedFilePath.clear();
        }
    }
}

void Runner::running()
{
    if(d->isRunning) {
        outputMsg("The program is running, please try again later!", OutputFormat::ErrorMessageFormat);
        return;
    }

    editor.switchContext(tr("&Application Output"));

    LanguageService *service = dpfGetService(LanguageService);
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activeProjectInfo.kitName());
        if (generator) {
            RunCommandInfo args = generator->getRunArguments(d->activeProjectInfo, d->currentOpenedFilePath);
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

    QString startMsg = tr("Start execute command: \"%1\" \"%2\" in workspace \"%3\".\n")
            .arg(info.program, info.arguments.join(" "), info.workingDir);
    outputMsg(startMsg, OutputFormat::NormalMessageFormat);

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
            outputMsg(line, OutputFormat::StdOutFormat);
        }
    });

    connect(&process, &QProcess::readyReadStandardError, [&]() {
        process.setReadChannel(QProcess::StandardError);
        while (process.canReadLine()) {
            QString line = QString::fromUtf8(process.readLine());
            outputMsg(line, OutputFormat::StdErrFormat);
        }
    });

    process.start(info.program, info.arguments);
    process.waitForFinished(-1);

    outputMsg(retMsg, ret ? OutputFormat::NormalMessageFormat : OutputFormat::StdErrFormat);

    QString endMsg = tr("Execute command finished.\n");
    outputMsg(endMsg, OutputFormat::NormalMessageFormat);

    return ret;
}

void Runner::outputMsg(const QString &content, OutputFormat format)
{
    emit sigOutputMsg(content, format);
}

