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
#include "buildmanager.h"
#include "buildstep.h"
#include "buildoutputpane.h"
#include "project.h"
#include "buildtarget.h"
#include "services/window/windowservice.h"
#include "menumanager.h"
#include "tasks/taskmanager.h"
#include "commandstep.h"
#include "eventsender.h"

#include <QtConcurrent>
#include <QTextBlock>

using namespace dpfservice;
BuildManager *BuildManager::instance()
{
    static BuildManager ins;
    return &ins;
}

void BuildManager::initialize(WindowService *windowService)
{
    qRegisterMetaType<BuildManager::BuildState>("BuildManager::BuildState");

    project.reset(new Project(this));
    menuManager.reset(new MenuManager(this));

    menuManager->initialize(windowService);
}

bool BuildManager::buildList(const QList<BuildStep *> &_bsl)
{
    TaskManager::instance()->clearTasks();
    if (outputPane)
        outputPane->clearContents();

    // Notify listeners.
    emit buildStarted();

    bsl = _bsl;
    initBuildList(bsl);

    buildState = kBuilding;
    menuManager->handleRunStateChanged(buildState);
    QtConcurrent::run([&](){
        QMutexLocker locker(&releaseMutex);
        bool success = true;
        for (auto step : bsl) {
            if (step) {
                success &= step->run();
            }
        }
        buildState = success ? kNoBuild : kBuildFailed;
        QMetaObject::invokeMethod(menuManager.get(), "handleRunStateChanged",
                                  Q_ARG(BuildManager::BuildState, buildState));

        EventSender::notifyBuildState(buildState);
    });
    return true;
}

BuildStep *BuildManager::makeCommandStep(const QString &cmd, const QStringList &args, QString outputDirectory)
{
    auto cmdStep = new CommandStep();
    cmdStep->setBuildOutputDir(outputDirectory);
    cmdStep->setCommand(cmd, args);
    return cmdStep;
}

BuildOutputPane *BuildManager::getOutputPane() const
{
    return outputPane;
}

void BuildManager::destroy()
{
    // Wait for finished.
    QMutexLocker locker(&releaseMutex);

    // Do something.
}

void BuildManager::slotOutput(const QString &content, OutputFormat format)
{
    QString outputContent = content;
    if (format == NormalMessage) {
        QDateTime curDatetime = QDateTime::currentDateTime();
        QString time = curDatetime.toString("hh:mm:ss");
        outputContent = time + ":" + content + "\r";
    }

    outputPane->appendText(outputContent, format);
}

void BuildManager::buildProject()
{
    auto step = makeBuildStep();
    buildList({step});
}

void BuildManager::rebuildProject()
{
    auto cleanStep = makeCleanStep();
    auto buildStep = makeBuildStep();
    buildList({cleanStep, buildStep});
}

void BuildManager::cleanProject()
{
    auto step = makeCleanStep();
    buildList({step});
}

BuildManager::BuildManager(QObject *parent) : QObject(parent)
{
    outputPane = new BuildOutputPane();
}

BuildManager::~BuildManager()
{
    destroy();
}

bool BuildManager::initBuildList(const QList<BuildStep *> &bsl)
{
    if (!outputPane)
        return false;

    // TODO(mozart) : more initialization will be done here.
    for (auto step : bsl) {
        connect(step, &BuildStep::addOutput, this, &BuildManager::slotOutput, Qt::QueuedConnection);
        connect(step, &BuildStep::addTask, TaskManager::instance(), &TaskManager::slotAddTask, Qt::QueuedConnection);
    }
    return true;
}

BuildStep *BuildManager::makeBuildStep()
{
    return makeStep(kBuildTarget);
}

BuildStep *BuildManager::makeCleanStep()
{
    return makeStep(kCleanTarget);
}

BuildStep *BuildManager::makeStep(TargetType type)
{
    auto &ctx = dpfInstance.serviceContext();
    RuntimeService *runTimeService = ctx.service<RuntimeService>(RuntimeService::name());
    if (runTimeService && runTimeService->getActiveTarget) {
        auto target = runTimeService->getActiveTarget(type);
        if (target.buildCommand.isEmpty()) {
            BuildManager::instance()->getOutputPane()->appendText("Nothing to do.");
        } else {
            QStringList args = target.buildArguments << target.buildTarget;
            return makeCommandStep(target.buildCommand, args, target.outputPath);
        }
    }
    return nullptr;
}
