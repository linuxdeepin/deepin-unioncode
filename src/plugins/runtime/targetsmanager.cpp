/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "targetsmanager.h"
#include "services/project/projectservice.h"
#include "kitmanager.h"

static const char *kCleanName = "clean";
static const char *kProjectFile = ".cproject";

using namespace dpfservice;
TargetsManager::TargetsManager(QObject *parent) : QObject(parent)
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());

    if (projectService) {
        connect(projectService, &ProjectService::projectConfigureDone, [this](){
            intialize();
        });
    }
}

TargetsManager *TargetsManager::instance()
{
    static TargetsManager instance;
    return &instance;
}

void TargetsManager::intialize()
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());

    // TODO(Mozart):cproject path should get from workspace.
    QString buildDirectory;
    if (projectService && projectService->getDefaultOutputPath) {
        buildDirectory = projectService->getDefaultOutputPath();
    }

    if (buildDirectory.isEmpty()) {
        qCritical() << "build directory not set!";
        return;
    }

    QString cprojectPath = buildDirectory + "/" + kProjectFile;
    parser.parse(cprojectPath);

    auto targets = parser.getTargets();
    for (auto target : targets) {
        if (target.buildTarget.contains(kCleanName, Qt::CaseInsensitive)) {
            if (target.path.isEmpty()) {
                cleanTargetSelected = target;
            }
        } else {
            buildTargets.push_back(target);
            if (target.path.isEmpty()) {
                buildTargetSelected = target;
            }
            auto targetName = target.name;
            if (targetName.contains("[exe]") && !targetName.contains("/fast")) {
                exeTargets.push_back(target);
            }
        }

        if (!target.buildTarget.isEmpty()) {
            targetNamesList.append(target.buildTarget);
        }
    }

    // remove the repeat items.
    targetNamesList = targetNamesList.toSet().toList();
}

Target TargetsManager::getSelectedTargetInList()
{
    return buildTargetSelected;
}

Target TargetsManager::getActiveBuildTarget()
{
    Target retTarget;
    // TODO(Mozart):re-write here when ui support target choose.
    if (exeTargets.size() > 0)
        retTarget = exeTargets.front();

    retTarget.outputPath = KitManager::instance()->getDefaultOutputPath();

    return retTarget;
}

const Target &TargetsManager::getActiveCleanTarget() const
{
    return cleanTargetSelected;
}

const QStringList &TargetsManager::getTargetNamesList() const
{
    return targetNamesList;
}

const Targets &TargetsManager::getTargets() const
{
    return parser.getTargets();
}

void TargetsManager::updateActiveBuildTarget(const QString &target)
{
    for (auto t : buildTargets) {
        if (t.buildTarget == target) {
            buildTargetSelected = t;
            break;
        }
    }
}

void TargetsManager::updateActiveCleanTarget(const QString &target)
{
    for (auto t : buildTargets) {
        // TODO(Mozart)
        if (t.buildTarget == target) {
            cleanTargetSelected = t;
            break;
        }
    }
}

