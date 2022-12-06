/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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

}

TargetsManager::~TargetsManager()
{

}

QString TargetsManager::getCMakeConfigFile()
{
    return kProjectFile;
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

void TargetsManager::initialize(const QString &buildDirectory)
{
    if (buildDirectory.isEmpty()) {
        qCritical() << "build directory not set!";
        return;
    }

    exeTargets.clear();
    targets.clear();
    buildTargetNameList.clear();
    exeTargetNameList.clear();

    QString cprojectPath = buildDirectory + QDir::separator() + kProjectFile;
    if (!QFileInfo(cprojectPath).isFile()) {
        qCritical() << cprojectPath + " is not existed!";
        return;
    }

    parser.parse(cprojectPath);
    targets = parser.getTargets();

    QStringList targetNameList;
    QStringList tempExeTargetNameList;
    for (auto target : targets) {
        target.outputPath = buildDirectory;
        auto targetName = target.name;
        if (targetName.contains("[exe]") && !targetName.contains("/fast")) {
            exeTargets.push_back(target);
            tempExeTargetNameList.push_back(target.buildTarget);
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
            targetNameList.append(target.buildTarget);
        }
    }

    buildTargetNameList = targetNameList.toSet().toList();
    exeTargetNameList = tempExeTargetNameList.toSet().toList();

    if (exeTargets.size() > 0) {
        exeTargetSelected = exeTargets.front();
    }

    emit initialized();
}

const QStringList TargetsManager::getTargetNamesList() const
{
    return buildTargetNameList;
}

const QStringList TargetsManager::getExeTargetNamesList() const
{
    return exeTargetNameList;
}

Target TargetsManager::getTargetByName(const QString &targetName)
{
    Target result;
    for (auto iter = targets.begin(); iter != targets.end(); ++iter) {
        if (targetName == iter->buildTarget) {
            result = *iter;
        }
    }

    return result;
}

Target TargetsManager::getActivedTargetByTargetType(const TargetType type)
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
        result = exeTargetSelected;
        break;
    default:
        break;
    }
    return result;
}

const Targets TargetsManager::getAllTargets() const
{
    return targets;
}

void TargetsManager::updateActivedBuildTarget(const QString &targetName)
{
    foreach (auto name, buildTargetNameList) {
        if (name == targetName) {           
            for (auto iter = targets.begin(); iter != targets.end(); ++iter) {
                if (name == iter->name) {
                    buildTargetSelected = *iter;
                    return;
                }
            }
        }
    }
}

void TargetsManager::updateActivedCleanTarget(const QString &targetName)
{
    foreach (auto name, buildTargetNameList) {
        if (name == targetName) {
            for (auto iter = targets.begin(); iter != targets.end(); ++iter) {
                if (name == iter->name) {
                    cleanTargetSelected = *iter;
                    return;
                }
            }
        }
    }
}

