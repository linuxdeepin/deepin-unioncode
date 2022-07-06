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
#include "project.h"
#include "buildtarget.h"
#include "cmakemanager.h"
#include "services/window/windowservice.h"
#include "commandstep.h"

#include <QtConcurrent>
#include <QTextBlock>

using namespace dpfservice;
BuildManager *BuildManager::instance()
{
    static BuildManager ins;
    return &ins;
}

void BuildManager::initialize()
{
    project.reset(new Project(this));
}

bool BuildManager::buildList(const QList<BuildStep *> &_bsl, QString originCmd)
{
    if (buildState == kBuilding) {
        return false;
    }

    // Notify listeners.

    bsl = _bsl;
    initBuildList(bsl);

    buildState = kBuilding;
    QtConcurrent::run([=](){
        QMutexLocker locker(&releaseMutex);
        bool success = true;
        for (auto step : bsl) {
            if (step) {
                success &= step->run();
            }
        }
        buildState = success ? kNoBuild : kBuildFailed;
        emit CMakeManager::instance()->buildStateChanged(buildState, originCmd);
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

void BuildManager::destroy()
{
    // Wait for finished.
    QMutexLocker locker(&releaseMutex);

    // Do something.
}

void BuildManager::buildProject()
{
    auto step = makeBuildStep();
    if (step == nullptr)
        return;

    buildList({step});
}

void BuildManager::rebuildProject()
{
    auto cleanStep = makeCleanStep();
    auto buildStep = makeBuildStep();

    if (cleanStep == nullptr || buildStep == nullptr)
        return;

    buildList({cleanStep, buildStep});
}

void BuildManager::cleanProject()
{
    auto step = makeCleanStep();
    buildList({step});
}

BuildManager::BuildManager(QObject *parent) : QObject(parent)
{
    initialize();
}

BuildManager::~BuildManager()
{
    destroy();
}

bool BuildManager::initBuildList(const QList<BuildStep *> &bsl)
{
    // TODO(mozart) : more initialization will be done here.
    for (auto step : bsl) {
        connect(step, &BuildStep::addOutput, CMakeManager::instance(), &CMakeManager::addCompileOutput, Qt::QueuedConnection);
        connect(step, &BuildStep::addTask, CMakeManager::instance(), &CMakeManager::addProblemOutput, Qt::QueuedConnection);
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
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && projectService->getActiveTarget) {
        auto target = projectService->getActiveTarget(type);
        if (target.buildCommand.isEmpty()) {
            emit CMakeManager::instance()->addCompileOutput("Target build command is null.", OutputFormat::NormalMessage);
        } else {
            QStringList args = target.buildArguments << target.buildTarget;
            return makeCommandStep(target.buildCommand, args, target.outputPath);
        }
    }
    return nullptr;
}
