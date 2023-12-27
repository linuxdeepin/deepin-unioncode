// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configutil.h"
#include "cmake/project/mainframe/cmakeasynparse.h"
#include "cmake/project/mainframe/cmakeprojectgenerator.h"
#include "targetsmanager.h"

#include "services/option/optionmanager.h"

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
    ConfigureParam configureParam;
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

ConfigureParam *ConfigUtil::getConfigureParamPointer()
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

bool ConfigUtil::isNeedConfig(const QString &workspace, ConfigureParam &param)
{
    QString propertyFile = getConfigPath(workspace);
    if (QFileInfo(propertyFile).exists() || QFileInfo(propertyFile).isFile()) {
        readConfig(propertyFile, param);
        if (!param.buildConfigures.isEmpty()) {
            return false;
        }
    }
    return true;
}

dpfservice::ProjectInfo ConfigUtil::createProjectInfo(const ConfigureParam *param)
{
    dpfservice::ProjectInfo info;
    for (auto iter = param->buildConfigures.begin(); iter != param->buildConfigures.end(); ++iter) {
        if (d->configureParam.tempSelType == iter->type) {
            info.setLanguage(param->language);
            info.setKitName(CmakeProjectGenerator::toolKitName());
            info.setWorkspaceFolder(param->workspace);
            info.setBuildType(ConfigUtil::instance()->getNameFromType(iter->type));
            info.setBuildFolder(iter->directory);
            info.setBuildProgram(OptionManager::getInstance()->getCMakeToolPath());

            QStringList configArguments;
            configArguments << "-S";
            configArguments << info.workspaceFolder();
            configArguments << "-B";
            configArguments << info.buildFolder();
            configArguments << "-G";
            configArguments << CDT_PROJECT_KIT::get()->CBP_GENERATOR;
            configArguments << "-DCMAKE_BUILD_TYPE=" + info.buildType();
            configArguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
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
            info.setCleanCustomArgs(buildArguments);
        }
    }

    return info;
}

void ConfigUtil::configProject(const ConfigureParam *param)
{
    dpfservice::ProjectInfo info = createProjectInfo(param);
    if (info.isVaild()) {
        emit configureDone(info);
    }
}

void ConfigUtil::checkConfigInfo(const QString &buildType, const QString &directory)
{
    ConfigType type = getTypeFromName(buildType);
    auto iter = d->configureParam.buildConfigures.begin();
    for (; iter != d->configureParam.buildConfigures.end(); ++iter) {
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

void ConfigUtil::readConfig(const QString &filePath, ConfigureParam &param)
{
    param.clear();
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        stream >> param;
        file.close();
    }
}

void ConfigUtil::saveConfig(const QString &filePath, const ConfigureParam &param)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        QDataStream stream(&file);
        stream << param;
        file.close();
    }
}

bool ConfigUtil::updateProjectInfo(dpfservice::ProjectInfo &info, const ConfigureParam *param)
{
    if (!param)
        return false;

    for (auto iter = param->buildConfigures.begin(); iter != param->buildConfigures.end(); ++iter) {
        if (d->configureParam.defaultType == iter->type) {
            info.setLanguage(param->language);
            info.setKitName(CmakeProjectGenerator::toolKitName());
            info.setWorkspaceFolder(param->workspace);
            info.setBuildType(ConfigUtil::instance()->getNameFromType(iter->type));
            info.setBuildFolder(iter->directory);
            info.setBuildProgram(OptionManager::getInstance()->getCMakeToolPath());

            QStringList arguments;
            arguments << "-S";
            arguments << info.workspaceFolder();
            arguments << "-B";
            arguments << info.buildFolder();
            arguments << "-G";
            arguments << CDT_PROJECT_KIT::get()->CBP_GENERATOR;
            arguments << "-DCMAKE_BUILD_TYPE=" + info.buildType();
            arguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
            info.setConfigCustomArgs(arguments);

            for (auto iterStep = iter->steps.begin(); iterStep != iter->steps.end(); ++iterStep) {
                QStringList arguments;
                arguments << "--build";
                arguments << ".";
                arguments << "--target";

                if (iterStep->type == StepType::Build) {
                    QString buildTarget = iterStep->targetName;
                    if (buildTarget.isEmpty()) {
                        buildTarget = "all";
                    }
                    TargetsManager::instance()->updateActivedBuildTarget(buildTarget);
                    arguments << buildTarget;
                } else if (iterStep->type == StepType::Clean) {
                    QString cleanTarget = iterStep->targetName;
                    if (cleanTarget.isEmpty()) {
                        cleanTarget = "clean";
                    }
                    TargetsManager::instance()->updateActivedCleanTarget(cleanTarget);
                    arguments << cleanTarget;
                }

                if (!iterStep->arguments.isEmpty()) {
                    arguments << "--";
                    arguments << iterStep->arguments;
                }

                if (iterStep->type == StepType::Build) {
                    info.setBuildCustomArgs(arguments);
                } else if (iterStep->type == StepType::Clean) {
                    info.setCleanCustomArgs(arguments);
                }
            }
            // update run config according to ui parameters.
            auto iterRun = iter->runConfigure.params.begin();
            for (; iterRun != iter->runConfigure.params.end(); ++iterRun) {
                if (iterRun->targetName == iter->runConfigure.defaultTargetName) {
                    info.setRunProgram(iterRun->targetPath);
                    QStringList arguments;
                    if (!iterRun->arguments.isEmpty())
                        arguments << iterRun->arguments;
                    info.setRunCustomArgs(arguments);
                    info.setRunWorkspaceDir(iterRun->workDirectory);

                    TargetsManager::instance()->updateActiveExceTarget(iterRun->targetName);
                    break;
                }
            }

            QStringList envs;
            for (auto it = iter->env.environments.begin() ; it != iter->env.environments.end() ; it++) {
                envs.append(it.key() + "=" + it.value());
            }
            info.setRunEnvironment(envs);

            return true;
        }
    }
    return false;
}

}   //namespace config
