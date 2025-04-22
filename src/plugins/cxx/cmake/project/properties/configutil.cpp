// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configutil.h"
#include "cmakeasynparse.h"
#include "cmakeprojectgenerator.h"
#include "targetsmanager.h"
#include "cmake/option/kitmanager.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QVector>
#include <QFileInfo>
#include <QDir>

namespace config {

class ConfigUtilPrivate
{
    friend class ConfigUtil;
    ProjectConfigure configureParam;
    QMap<ConfigType, QString> configTypeStringMap;
};

ConfigUtil::ConfigUtil(QObject *parent)
    : QObject(parent), d(new ConfigUtilPrivate())
{
    d->configTypeStringMap = { { Debug, "Debug" },
                               { Release, "Release" } };
}

ConfigUtil::~ConfigUtil()
{
    if (d) {
        delete d;
    }
}

ConfigUtil *ConfigUtil::instance()
{
    static ConfigUtil ins;
    return &ins;
}

ProjectConfigure *ConfigUtil::getConfigureParamPointer()
{
    return &d->configureParam;
}

QString ConfigUtil::getNameFromType(ConfigType type)
{
    if (d->configTypeStringMap.contains(type)) {
        return d->configTypeStringMap.value(type);
    }

    return "Unkown";
}

ConfigType ConfigUtil::getTypeFromName(QString name)
{
    ConfigType type = d->configTypeStringMap.key(name);
    return type;
}

bool ConfigUtil::isNeedConfig(const QString &workspace, ProjectConfigure &param)
{
    QString propertyFile = getConfigPath(workspace);
    if (QFileInfo(propertyFile).exists() || QFileInfo(propertyFile).isFile()) {
        if (!readConfig(propertyFile, param))
            return true;

        auto kit = KitManager::instance()->findKit(param.kitId);
        if (!kit.isValid())
            return true;

        if (!param.buildTypeConfigures.isEmpty())
            return false;
    }
    return true;
}

dpfservice::ProjectInfo ConfigUtil::createProjectInfo(const ProjectConfigure *param)
{
    dpfservice::ProjectInfo info;
    for (auto iter = param->buildTypeConfigures.begin(); iter != param->buildTypeConfigures.end(); ++iter) {
        if (d->configureParam.tempSelType == iter->type) {
            Kit kit = KitManager::instance()->findKit(param->kitId);

            info.setLanguage(param->language);
            info.setKitName(CmakeProjectGenerator::toolKitName());
            info.setWorkspaceFolder(param->workspace);
            info.setBuildType(ConfigUtil::instance()->getNameFromType(iter->type));
            info.setBuildFolder(iter->directory);
            info.setBuildProgram(kit.cmakeTool().path);
            info.setDebugProgram(kit.debugger().path);
            info.setKitId(kit.id());

            QStringList configArguments;
            configArguments << "-S";
            configArguments << info.workspaceFolder();
            configArguments << "-B";
            configArguments << info.buildFolder();
            configArguments << "-G";
            configArguments << CDT_PROJECT_KIT::get()->CBP_GENERATOR;
            configArguments << "-DCMAKE_BUILD_TYPE=" + info.buildType();
            configArguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
            configArguments << "-DCMAKE_C_COMPILER=" + kit.ccompiler().path;
            configArguments << "-DCMAKE_CXX_COMPILER=" + kit.cxxcompiler().path;
            info.setConfigCustomArgs(configArguments);

            QStringList buildArguments;
            buildArguments << "--build";
            buildArguments << ".";
            buildArguments << "--target";
            buildArguments << "all";
            info.setBuildCustomArgs(buildArguments);

            QStringList cleanArguments;
            cleanArguments << "--build";
            cleanArguments << ".";
            cleanArguments << "--target";
            cleanArguments << "clean";
            info.setCleanCustomArgs(cleanArguments);
        }
    }

    return info;
}

void ConfigUtil::configProject(const ProjectConfigure *param)
{
    dpfservice::ProjectInfo info = createProjectInfo(param);
    if (info.isVaild()) {
        emit configureDone(info);
    }
}

void ConfigUtil::checkConfigInfo(const QString &buildType, const QString &directory)
{
    ConfigType type = getTypeFromName(buildType);
    auto iter = d->configureParam.buildTypeConfigures.begin();
    for (; iter != d->configureParam.buildTypeConfigures.end(); ++iter) {
        if (type == iter->type) {
            if (!directory.isEmpty())
                iter->directory = directory;
            QString cfgFile = iter->directory + QDir::separator() + TargetsManager::instance()->getCMakeConfigFile();
            if (!QFileInfo(cfgFile).isFile()) {
                // not config
                //                configProject(&d->configureParam);
            }
        }
    }
}

QString ConfigUtil::getConfigPath(const QString &projectPath)
{
    return CustomPaths::projectCachePath(projectPath) + QDir::separator() + "project.properties";
}

bool ConfigUtil::readConfig(const QString &filePath, ProjectConfigure &param)
{
    param.clear();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to read the project configuration: " << filePath;
        return false;
    }
    const auto &text = file.readAll();
    file.close();

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(text, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse the project configuration: " << filePath;
        return false;
    }

    return param.fromJson(doc.object());
}

void ConfigUtil::saveConfig(const QString &filePath, const ProjectConfigure &param)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to write the project configuration: " << filePath;
        return;
    }

    QJsonDocument doc(param.toJson());
    file.write(doc.toJson());
    file.close();
}

bool ConfigUtil::updateProjectInfo(dpfservice::ProjectInfo &info, const ProjectConfigure *param)
{
    if (!param)
        return false;

    for (auto iter = param->buildTypeConfigures.begin(); iter != param->buildTypeConfigures.end(); ++iter) {
        if (d->configureParam.defaultType == iter->type) {
            Kit kit = KitManager::instance()->findKit(param->kitId);

            info.setLanguage(param->language);
            info.setKitName(CmakeProjectGenerator::toolKitName());
            info.setWorkspaceFolder(param->workspace);
            info.setBuildType(ConfigUtil::instance()->getNameFromType(iter->type));
            info.setBuildFolder(iter->directory);
            info.setBuildProgram(kit.cmakeTool().path);
            info.setDebugProgram(kit.debugger().path);
            info.setKitId(kit.id());

            QStringList arguments;
            arguments << "-S";
            arguments << info.workspaceFolder();
            arguments << "-B";
            arguments << info.buildFolder();
            arguments << "-G";
            arguments << CDT_PROJECT_KIT::get()->CBP_GENERATOR;
            arguments << "-DCMAKE_BUILD_TYPE=" + info.buildType();
            arguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
            arguments << "-DCMAKE_C_COMPILER=" + kit.ccompiler().path;
            arguments << "-DCMAKE_CXX_COMPILER=" + kit.cxxcompiler().path;
            info.setConfigCustomArgs(arguments);

            for (auto iterStep = iter->buildConfigure.steps.begin(); iterStep != iter->buildConfigure.steps.end(); ++iterStep) {
                QStringList arguments;
                arguments << "--build";
                arguments << ".";
                arguments << "--target";

                if (iterStep->type == StepType::Build) {
                    QString buildTarget = iterStep->activeTargetName;
                    if (buildTarget.isEmpty()) {
                        buildTarget = "all";
                    }
                    TargetsManager::instance()->updateActivedBuildTarget(buildTarget);
                    arguments << buildTarget;
                } else if (iterStep->type == StepType::Clean) {
                    QString cleanTarget = iterStep->activeTargetName;
                    if (cleanTarget.isEmpty()) {
                        cleanTarget = "clean";
                    }
                    TargetsManager::instance()->updateActivedCleanTarget(cleanTarget);
                    arguments << cleanTarget;
                }

                if (!iterStep->buildArguments.isEmpty()) {
                    arguments << "--";
                    arguments << iterStep->buildArguments;
                }

                if (iterStep->type == StepType::Build) {
                    info.setBuildCustomArgs(arguments);
                } else if (iterStep->type == StepType::Clean) {
                    info.setCleanCustomArgs(arguments);
                }
            }
            // update run config according to ui parameters.
            auto iterRun = iter->runConfigure.targetsRunConfigure.begin();
            for (; iterRun != iter->runConfigure.targetsRunConfigure.end(); ++iterRun) {
                if (iterRun->targetName == iter->runConfigure.defaultTargetName) {
                    info.setRunProgram(iterRun->targetPath);
                    QStringList arguments;
                    if (!iterRun->arguments.isEmpty())
                    #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                        arguments = iterRun->arguments.split(" ", Qt::SkipEmptyParts);
                    #else
                        arguments = iterRun->arguments.split(" ", QString::SkipEmptyParts);
                    #endif
                    info.setRunCustomArgs(arguments);
                    info.setRunWorkspaceDir(iterRun->workDirectory);
                    info.setCurrentProgram(iterRun->targetName);
                    info.setRunInTerminal(iterRun->runInTermal);

                    TargetsManager::instance()->updateActiveExceTarget(iterRun->targetName);
                    break;
                }
            }

            return true;
        }
    }
    return false;
}

void ConfigUtil::switchConfigType(const ConfigType &type)
{
    emit configTypeSwitched(type);
}

}   //namespace config
