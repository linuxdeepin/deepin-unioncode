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

#include <QtConcurrent>

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
    bsl = _bsl;
    initBuildList(bsl);

    buildState = kBuilding;
    menuManager->handleRunStateChanged(buildState);
    QtConcurrent::run([&](){
        QMutexLocker locker(&releaseMutex);
        for (auto step : bsl) {
            step->run();
        }
        buildState = kNoBuild;
        QMetaObject::invokeMethod(menuManager.get(), "handleRunStateChanged",
                                  Q_ARG(BuildManager::BuildState, buildState));
    });
    return true;
}

BuildOutputPane *BuildManager::getOutputPane() const
{
    return outputPane;
}

void BuildManager::destroy()
{
    if (outputPane) {
        delete outputPane;
    }

    // Wait for finished.
    QMutexLocker locker(&releaseMutex);
}

void BuildManager::slotOutput(const QString &content, BuildStep::OutputFormat format)
{
    outputPane->appendText(content, format);
}

void BuildManager::buildProject()
{
    // TODO(mozart):steps should get from other place.
    auto buildList = project->activeTarget()->getbuildSteps();
    if (buildList.size() > 0)
        BuildManager::instance()->buildList(buildList);
    else {
        BuildManager::instance()->getOutputPane()->appendText("Nothing to do.");
    }
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
    }
    return true;
}
