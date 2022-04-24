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
            cleanTargets.push_back(target);
            if (target.path.isEmpty()) {
                activeCleanTargetName = target.buildTarget;
            }
        } else {
            buildTargets.push_back(target);
            if (target.path.isEmpty()) {
                activeBuildTargetName = target.buildTarget;
            }
        }

        if (!target.buildTarget.isEmpty()) {
            targetNamesList.append(target.buildTarget);
        }
    }

    // remove the repeat items.
    targetNamesList = targetNamesList.toSet().toList();
}

const QString &TargetsManager::getActiveBuildTargetName() const
{
    return activeBuildTargetName;
}

const QString &TargetsManager::getActiveCleanTargetName() const
{
    return activeCleanTargetName;
}

const QStringList &TargetsManager::getTargetNamesList() const
{
    return targetNamesList;
}

const Targets &TargetsManager::getTargets() const
{
    return parser.getTargets();
}

const Targets &TargetsManager::getBuildTargets() const
{
    return buildTargets;
}

const Targets &TargetsManager::getCleanTargets() const
{
    return cleanTargets;
}

void TargetsManager::updateActiveBuildTarget(const QString &target)
{
    activeBuildTargetName = target;
}

void TargetsManager::updateActiveCleanTarget(const QString &target)
{
    activeCleanTargetName = target;
}

