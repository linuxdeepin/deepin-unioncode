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

#include <QtConcurrent>

BuildManager *BuildManager::instance()
{
    static BuildManager ins;
    return &ins;
}

bool BuildManager::isBuilding() const
{
    return isBuild;
}

bool BuildManager::buildList(const QList<BuildStep *> &bsl)
{
    initBuildList(bsl);

    isBuild = true;
    QtConcurrent::run([&](){
        for (auto step : bsl) {
            step->run();
        }
    });
    isBuild = false;
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
}

BuildManager::BuildManager(QObject *parent) : QObject(parent)
{
    outputPane = new BuildOutputPane();
}

bool BuildManager::initBuildList(const QList<BuildStep *> &bsl)
{
    if (!outputPane)
        return false;

    // TODO(mozart) : more initialization will be done here.
    for (auto step : bsl) {
        connect(step, &BuildStep::addOutput, this, [&](const QString &content){
            outputPane->appendText(content);
        }, Qt::QueuedConnection);
    }
    return true;
}
