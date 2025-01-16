// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "targetsmanager.h"
#include "services/project/projectservice.h"
#include "cbp/cbpparser.h"

//static const char *kProjectFile = ".cproject";
static const char *kProjectFile = ".cbp";

// target name build all.
static const char *kGlobalBuild = "all";

// target name rebuild all.
static const char *kGlobalRebuild = "rebuild_cache";

// target name rebuild all.
static const char *kGlobalClean = "clean";

using namespace dpfservice;

TargetsManager::TargetsManager(QObject *parent) : QObject(parent)
  , parser(new CMakeCbpParser())
{

}

TargetsManager::~TargetsManager()
{
    if (parser) {
        delete parser;
        parser = nullptr;
    }
}

QString TargetsManager::getCMakeConfigFile()
{
    return kProjectFile;
}

bool TargetsManager::isGloablTarget(Target &target)
{
    return target.srcPath.isEmpty();
}

QString TargetsManager::cbpFilePath(const QString &buildFolder)
{
    QString cbpFilePath;
    QDir dir(buildFolder);

    QStringList filters;
    filters << "*.cbp";

    QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files);
    if (fileInfoList.size() == 0) {
        qInfo() << ".cbp file not found!";
        return {};
    }
    return fileInfoList.first().filePath();
}

TargetsManager *TargetsManager::instance()
{
    static TargetsManager instance;
    return &instance;
}

CMakeCbpParser *TargetsManager::cbpParser() const
{
    return parser;
}

void TargetsManager::readTargets(const QString &buildDirectory, const QString &workspaceDirectory)
{
    if (buildDirectory.isEmpty()) {
        qCritical() << "build directory not set!";
        return;
    }

    exeTargets.clear();
    targets.clear();
    buildTargetNameList.clear();
    exeTargetNameList.clear();

    QString cbp = cbpFilePath(buildDirectory);
    if (parser) {
        delete parser;
        parser = new CMakeCbpParser();
    }
    parser->parseCbpFile(cbp, workspaceDirectory);

    QStringList targetNameList;
    QStringList tempExeTargetNameList;

    QList<CMakeBuildTarget> cbpTargets = parser->getBuildTargets();
    for (auto cbpTarget : cbpTargets) {

        QStringList commandItems = cbpTarget.makeCommand.split(" ");
        QStringList argsFiltered;
        for (auto &arg : commandItems) {
            if (!arg.isEmpty()) {
                argsFiltered << arg.replace("\"", "");
            }
        }

        Target target;
        target.name = cbpTarget.title;
        target.buildCommand = argsFiltered.first();
        argsFiltered.pop_front();
        target.buildArguments = argsFiltered;
        target.output = cbpTarget.output;
        target.buildTarget = cbpTarget.title;
        target.workingDir = cbpTarget.workingDirectory;
        targets.push_back(target);

        if (cbpTarget.type == CBPTargetType::kExecutable) {
            exeTargets.push_back(target);
            tempExeTargetNameList.push_back(target.buildTarget);

            if (cbpTarget.title == cbpParser()->getProjectName()) {
                exeTargetSelected = target;
            }
        } else if (cbpTarget.type == CBPTargetType::kUtility) {
            if (target.name == kGlobalBuild) {
                buildTargetSelected = target;
            } else if (target.name == kGlobalClean) {
                cleanTargetSelected = target;
            } else if (target.name == kGlobalRebuild) {
                rebuildTargetSelected = target;
            }
        }
        targetNameList.append(target.buildTarget);
    }

    targetNameList.removeDuplicates();
    buildTargetNameList = targetNameList;
    tempExeTargetNameList.removeDuplicates();
    exeTargetNameList = tempExeTargetNameList;

    if (exeTargetSelected.name.isEmpty() && exeTargets.size() > 0) {
        exeTargetSelected = exeTargets.front();
    }

    emit initialized(workspaceDirectory);
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
        if (targetName == iter->name) {
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
    buildTargetSelected = getTargetByName(targetName);
}

void TargetsManager::updateActivedCleanTarget(const QString &targetName)
{
    cleanTargetSelected = getTargetByName(targetName);
}

void TargetsManager::updateActiveExceTarget(const QString &targetName)
{
    exeTargetSelected = getTargetByName(targetName);
}

