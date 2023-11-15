// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buildmanager.h"
#include "common/widget/outputpane.h"
#include "common/util/commandparser.h"
#include "problemoutputpane.h"
#include "commonparser.h"
#include "transceiver/buildersender.h"
#include "compileoutputpane.h"

#include "services/builder/builderservice.h"
#include "services/builder/ioutputparser.h"
#include "services/window/windowservice.h"
#include "services/project/projectinfo.h"
#include "services/builder/buildergenerator.h"
#include "services/option/optionmanager.h"
#include "services/project/projectservice.h"

#include "base/abstractaction.h"

#include <QCoreApplication>

using namespace dpfservice;

class BuildManagerPrivate
{
    friend class BuildManager;

    QSharedPointer<QAction> buildAction;
    QSharedPointer<QAction> rebuildAction;
    QSharedPointer<QAction> cleanAction;
    QSharedPointer<QAction> cancelAction;

    CompileOutputPane *compileOutputPane = nullptr;
    ProblemOutputPane *problemOutputPane = nullptr;

    QString activedKitName;
    QString activedWorkingDir;

    std::unique_ptr<IOutputParser> outputParser = nullptr;

    QProcess cmdProcess;
    QFuture<void> buildThread;

    BuildState currentState = BuildState::kNoBuild;
};

BuildManager *BuildManager::instance()
{
    static BuildManager ins;
    return &ins;
}

BuildManager::BuildManager(QObject *parent)
    : QObject(parent)
    , d(new BuildManagerPrivate())
{
    addMenu();

    d->compileOutputPane = new CompileOutputPane();
    d->problemOutputPane = new ProblemOutputPane();
    d->outputParser.reset(new CommonParser());
    connect(d->outputParser.get(), &IOutputParser::addOutput, this, &BuildManager::addOutput, Qt::DirectConnection);
    connect(d->outputParser.get(), &IOutputParser::addTask, d->problemOutputPane, &ProblemOutputPane::addTask, Qt::DirectConnection);

    QObject::connect(this, &BuildManager::sigOutputCompileInfo, this, &BuildManager::slotOutputCompileInfo);
    QObject::connect(this, &BuildManager::sigOutputProblemInfo, this, &BuildManager::slotOutputProblemInfo);

    qRegisterMetaType<BuildState>("BuildState");
    qRegisterMetaType<BuildCommandInfo>("BuildCommandInfo");
    QObject::connect(this, &BuildManager::sigBuildState, this, &BuildManager::slotBuildState);
    QObject::connect(this, &BuildManager::sigOutputNotify, this, &BuildManager::slotOutputNotify);
    QObject::connect(this, &BuildManager::sigResetBuildUI, this, &BuildManager::slotResetBuildUI);
}

BuildManager::~BuildManager()
{
    if (d) {
        delete d;
    }
}

void BuildManager::addMenu()
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    auto actionInit = [&](QAction *action, QString actionID, QKeySequence key, QString iconFileName){
        ActionManager::getInstance()->registerAction(action, actionID, action->text(), key, iconFileName);
        AbstractAction *actionImpl = new AbstractAction(action);
        windowService->addAction(dpfservice::MWM_BUILD, actionImpl);
    };

    d->buildAction.reset(new QAction(MWMBA_BUILD));
    actionInit(d->buildAction.get(), "Build.Build", QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B),
               ":/buildercore/images/build.svg");
    windowService->addToolBarActionItem("toolbar.Build", d->buildAction.get(), "Build.End");

    d->rebuildAction.reset(new QAction(MWMBA_REBUILD));
    actionInit(d->rebuildAction.get(), "Build.Rebuild", QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_B),
               ":/buildercore/images/rebuild.png");

    d->cleanAction.reset(new QAction(MWMBA_CLEAN));
    actionInit(d->cleanAction.get(), "Build.Clean", QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_C),
               ":/buildercore/images/clean.svg");

    d->cancelAction.reset(new QAction(MWMBA_CANCEL));
    actionInit(d->cancelAction.get(), "Build.Cancel", QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_Backspace),
               ":/buildercore/images/cancel.svg");

    QObject::connect(d->buildAction.get(), &QAction::triggered,
                     this, &BuildManager::buildProject, Qt::DirectConnection);
    QObject::connect(d->rebuildAction.get(), &QAction::triggered,
                     this, &BuildManager::rebuildProject, Qt::DirectConnection);
    QObject::connect(d->cleanAction.get(), &QAction::triggered,
                     this, &BuildManager::cleanProject, Qt::DirectConnection);
    QObject::connect(d->cancelAction.get(), &QAction::triggered,
                     this, &BuildManager::cancelBuild, Qt::DirectConnection);
}

void BuildManager::buildProject()
{
    execBuildStep({Build});
}

void BuildManager::rebuildProject()
{
    execBuildStep({Clean, Build});
}

void BuildManager::cleanProject()
{
    execBuildStep({Clean});
}

void BuildManager::cancelBuild()
{
    if (d->currentState == kBuilding) {
        d->buildThread.cancel();
        disconnectSignals();
        d->cmdProcess.kill();
    }
}


void BuildManager::execBuildStep(QList<BuildMenuType> menuTypelist)
{
    if(!canStartBuild()) {
        QMetaObject::invokeMethod(this, "message",
                                  Q_ARG(QString, "The builder is running, please try again later!"));
        return;
    }

    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService || !projectService->getProjectInfo)
        return;

    ProjectInfo projectInfo = projectService->getProjectInfo(d->activedKitName, d->activedWorkingDir);
    if (!projectInfo.isVaild())
        return;

    auto builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        auto generator = builderService->create<BuilderGenerator>(d->activedKitName);
        if (generator) {
            emit sigResetBuildUI();
            generator->appendOutputParser(d->outputParser);
            QList<BuildCommandInfo> list;
            foreach (auto menuType, menuTypelist) {
                BuildCommandInfo info = generator->getMenuCommand(menuType, projectInfo);
                QString retMsg;
                bool ret = generator->checkCommandValidity(info, retMsg);
                if (!ret) {
                    outputLog(retMsg, OutputPane::OutputFormat::StdErr);
                    continue;
                }
                list.append(info);
            }
            execCommands(list, false);
        }
    }
}

CompileOutputPane *BuildManager::getCompileOutputPane() const
{
    return d->compileOutputPane;
}

ProblemOutputPane *BuildManager::getProblemOutputPane() const
{
    return d->problemOutputPane;
}

void BuildManager::slotResetBuildUI()
{
    d->compileOutputPane->clearContents();
    d->problemOutputPane->clearContents();

    editor.switchContext(tr("Co&mpile Output"));
}

void BuildManager::setActivedProjectInfo(const QString &kitName, const QString &workingDir)
{
    d->activedKitName = kitName;
    d->activedWorkingDir = workingDir;
}

void BuildManager::clearActivedProjectInfo()
{
    d->activedKitName.clear();
    d->activedWorkingDir.clear();
}

bool BuildManager::handleCommand(const QList<BuildCommandInfo> &commandInfo, bool isSynchronous)
{
    if(!canStartBuild()) {
        QMetaObject::invokeMethod(this, "message",
                                  Q_ARG(QString, "The builder is running, please try again later!"));
        return false;
    }

    auto &ctx = dpfInstance.serviceContext();
    auto builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        auto generator = builderService->create<BuilderGenerator>(commandInfo.at(0).kitName);
        if (generator) {
            emit sigResetBuildUI();
            generator->appendOutputParser(d->outputParser);
            QString retMsg;
            bool ret = generator->checkCommandValidity(commandInfo.at(0), retMsg);
            if (!ret) {
                outputLog(retMsg, OutputPane::OutputFormat::StdErr);
                return false;
            }
        }
        execCommands(commandInfo, isSynchronous);
    }
    return true;
}

bool BuildManager::execCommands(const QList<BuildCommandInfo> &commandList, bool isSynchronous)
{
    //Synchronous execution is required in commandLine build model
    if (isSynchronous) {
        if (!commandList.isEmpty()) {
            for (auto command : commandList) {
                execCommand(command);
            }
        }
    } else {
        if (!commandList.isEmpty()) {
            d->buildThread = QtConcurrent::run([=](){
                QMutexLocker locker(&releaseMutex);
                for (auto command : commandList) {
                    execCommand(command);
                }
            });
        }
    }

    return true;
}

bool BuildManager::execCommand(const BuildCommandInfo &info)
{
    outBuildState(BuildState::kBuilding);
    bool ret = false;
    QString executeResult = tr("Execute command failed!\n");

    d->cmdProcess.setWorkingDirectory(info.workingDir);

    QString startMsg = tr("Start execute command: \"%1\" \"%2\" in workspace \"%3\".\n")
            .arg(info.program, info.arguments.join(" "), info.workingDir);
    outputLog(startMsg, OutputPane::OutputFormat::NormalMessage);

    connect(&d->cmdProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
        if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
            ret = true;
            executeResult = tr("The process \"%1\" exited normally.\n").arg(d->cmdProcess.program());
        } else if (exitStatus == QProcess::NormalExit) {
            ret = false;
            executeResult = tr("The process \"%1\" exited with code %2.\n")
                    .arg(d->cmdProcess.program(), QString::number(exitcode));
        } else {
            ret = false;
            executeResult = tr("The process \"%1\" crashed.\n")
                    .arg(d->cmdProcess.program());
        }
    });

    connect(&d->cmdProcess, &QProcess::readyReadStandardOutput, [&]() {
        d->cmdProcess.setReadChannel(QProcess::StandardOutput);
        while (d->cmdProcess.canReadLine()) {
            QString line = QString::fromUtf8(d->cmdProcess.readLine());
            outputLog(line, OutputPane::OutputFormat::StdOut);
        }
    });

    connect(&d->cmdProcess, &QProcess::readyReadStandardError, [&]() {
        d->cmdProcess.setReadChannel(QProcess::StandardError);
        while (d->cmdProcess.canReadLine()) {
            QString line = QString::fromUtf8(d->cmdProcess.readLine());
            outputLog(line, OutputPane::OutputFormat::StdErr);
            outputError(line);
        }
    });

    d->cmdProcess.start(info.program, info.arguments);
    d->cmdProcess.waitForFinished(-1);

    disconnectSignals();
    outputLog(executeResult, ret ? OutputPane::OutputFormat::NormalMessage : OutputPane::OutputFormat::StdErr);

    QString endMsg = tr("Execute command finished.\n");
    outputLog(endMsg, OutputPane::OutputFormat::NormalMessage);

    BuildState buildState = ret ? BuildState::kNoBuild : BuildState::kBuildFailed;
    outBuildState(buildState);

    outputNotify(buildState, info);
    return ret;
}

void BuildManager::outputLog(const QString &content, const OutputPane::OutputFormat format)
{
    emit sigOutputCompileInfo(content, format);
}

void BuildManager::outputError(const QString &content)
{
    emit sigOutputProblemInfo(content);
}

void BuildManager::outputNotify(const BuildState &state, const BuildCommandInfo &commandInfo)
{
    emit sigOutputNotify(state, commandInfo);
}

void BuildManager::slotOutputCompileInfo(const QString &content, const OutputPane::OutputFormat format)
{
    if (format == OutputPane::OutputFormat::StdOut || OutputPane::OutputFormat::NormalMessage) {
        std::cout << content.toStdString() << std::endl;
    } else if (format == OutputPane::OutputFormat::StdErr) {
        std::cerr << content.toStdString() << std::endl;
    }
    d->outputParser->stdOutput(content, format);
}

void BuildManager::slotOutputProblemInfo(const QString &content)
{
    d->outputParser->stdError(content);
}

void BuildManager::slotOutputNotify(const BuildState &state, const BuildCommandInfo &commandInfo)
{
    BuilderSender::notifyBuildState(state, commandInfo);
}

void BuildManager::addOutput(const QString &content, const OutputPane::OutputFormat format)
{
    QString newContent = content;
    if (OutputPane::OutputFormat::NormalMessage == format
            || OutputPane::OutputFormat::ErrorMessage == format
            || OutputPane::OutputFormat::StdOut == format) {

        QDateTime curDatetime = QDateTime::currentDateTime();
        QString time = curDatetime.toString("hh:mm:ss");
        newContent = time + ": " + newContent;
    }
    d->compileOutputPane->appendText(newContent, format);
}

void BuildManager::outBuildState(const BuildState &buildState)
{
    emit sigBuildState(buildState);
}

void BuildManager::slotBuildState(const BuildState &buildState)
{
    d->currentState = buildState;

    switch (buildState) {
    case BuildState::kNoBuild:
    case BuildState::kBuildFailed:
        d->buildAction->setEnabled(true);
        d->rebuildAction->setEnabled(true);
        d->cleanAction->setEnabled(true);
        d->cancelAction->setEnabled(false);
        break;
    case BuildState::kBuilding:
        d->buildAction->setEnabled(false);
        d->rebuildAction->setEnabled(false);
        d->cleanAction->setEnabled(false);
        d->cancelAction->setEnabled(true);
        break;
    }
}

bool BuildManager::canStartBuild()
{
    return BuildState::kBuilding != d->currentState;
}

void BuildManager::disconnectSignals()
{
    disconnect(&d->cmdProcess, static_cast<void (QProcess::*)\
                        (int, QProcess::ExitStatus)>(&QProcess::finished), nullptr, nullptr);
    disconnect(&d->cmdProcess, &QProcess::readyReadStandardOutput, nullptr, nullptr);
    disconnect(&d->cmdProcess, &QProcess::readyReadStandardError, nullptr, nullptr);
}
