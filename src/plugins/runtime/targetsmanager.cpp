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

static const char *kProjectFile = ".cproject";

// target name build all.
static const char *kGlobalBuild = ": all";

// target name rebuild all.
static const char *kGlobalRebuild = ": rebuild_cache";

// target name rebuild all.
static const char *kGlobalClean = ": clean";

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

bool TargetsManager::isGloablTarget(Target &target)
{
    return target.path.isEmpty();
}

TargetsManager *TargetsManager::instance()
{
    static TargetsManager instance;
    return &instance;
}

void TargetsManager::intialize()
{
    // TODO(Mozart):cproject path should get from workspace.
    QString buildDirectory = KitManager::instance()->getDefaultOutputPath();
    if (buildDirectory.isEmpty()) {
        qCritical() << "build directory not set!";
        return;
    }

    QString cprojectPath = buildDirectory + "/" + kProjectFile;
    parser.parse(cprojectPath);

    auto targets = parser.getTargets();
    for (auto target : targets) {
        target.outputPath = buildDirectory;
        auto targetName = target.name;
        if (targetName.contains("[exe]") && !targetName.contains("/fast")) {
            exeTargets.push_back(target);
        } else if (isGloablTarget(target)) {
            if (target.name == kGlobalBuild) {
                buildTargetSelected = target;
            } else if (target.name == kGlobalClean) {
                cleanTargetSelected = target;
            } else if (target.name == kGlobalRebuild) {
                rebuildTargetSelected = target;
            }
        }

        if (!target.buildTarget.isEmpty()) {
            buildTargetList.append(target.buildTarget);
        }
    }

    // remove the repeat items.
    buildTargetList = buildTargetList.toSet().toList();
}

Target TargetsManager::getTarget(TargetType type)
{
    Target result;
    switch (type) {
    case kBuildTarget:
        result = buildTargetSelected;
        break;
    case kRebuildTarget:
        result = rebuildTargetSelected;
        break;
    case kCleanTarget:
        result = cleanTargetSelected;
        break;
    case kActiveExecTarget:
        result = getActiveBuildTarget();
        break;
    default:
        // do nothing.
        break;
    }
    return result;
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

    return retTarget;
}

const Target &TargetsManager::getActiveCleanTarget() const
{
    return cleanTargetSelected;
}

const QStringList &TargetsManager::getTargetNamesList() const
{
    return buildTargetList;
}

const Targets &TargetsManager::getTargets() const
{
    return parser.getTargets();
}

void TargetsManager::updateActiveBuildTarget(const QString &target)
{
    for (auto t : getTargets()) {
        if (t.buildTarget == target) {
            buildTargetSelected = t;
            break;
        }
    }
}

void TargetsManager::updateActiveCleanTarget(const QString &target)
{
    for (auto t : getTargets()) {
        // TODO(Mozart)
        if (t.buildTarget == target) {
            cleanTargetSelected = t;
            break;
        }
    }
}

void TargetsManager::save()
{

}

