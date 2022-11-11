/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

#include "configutil.h"
#include "mainframe/cmakeasynparse.h"
#include "mainframe/cmakegenerator.h"
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
    : QObject(parent)
    , d(new ConfigUtilPrivate())
{
    d->configTypeStringMap = {{Debug, "Debug"},
                        {Release, "Release"}};
}

ConfigUtil::~ConfigUtil()
{

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

void ConfigUtil::clearConfigureParam()
{
    d->configureParam.kit.clear();
    d->configureParam.language.clear();
    d->configureParam.defaultType = Unknown;
    d->configureParam.projectPath.clear();
    d->configureParam.buildConfigures.clear();
}

QString ConfigUtil::getConfigFilePath(const QString &projectPath, int configType)
{
    QString newFolder = QFileInfo(projectPath).dir().path() + QDir::separator() + ".unioncode";
    QDir dir(newFolder);
    if (!dir.exists()) {
        dir.mkdir(newFolder);
    }

    QString configFilePath;
    if (dir.exists()) {
        QString fileName;

    }

    return configFilePath;
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

void ConfigUtil::configProject(const ConfigureParam *param)
{
    for (auto iter = param->buildConfigures.begin(); iter != param->buildConfigures.end(); ++iter) {
        if (d->configureParam.defaultType == iter->type) {
            dpfservice::ProjectInfo info;
            QString sourceFolder = QFileInfo(param->projectPath).path();
            info.setLanguage(param->language);
            info.setSourceFolder(sourceFolder);
            info.setKitName(CmakeGenerator::toolKitName());
            info.setWorkspaceFolder(sourceFolder);
            info.setProjectFilePath(param->projectPath);
            info.setBuildType(ConfigUtil::instance()->getNameFromType(iter->type));
            info.setBuildFolder(iter->directory);
            info.setBuildProgram(OptionManager::getInstance()->getCMakeToolPath());

            QStringList arguments;
            arguments << "-S";
            arguments << info.sourceFolder();
            arguments << "-B";
            arguments << info.buildFolder();
            arguments << "-G";
            arguments << CDT_PROJECT_KIT::get()->CDT4_GENERATOR;
            arguments << "-DCMAKE_BUILD_TYPE=" + info.buildType();
            arguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
            arguments << info.buildCustomArgs();
            info.setBuildCustomArgs(arguments);

            emit configureDone(info);
        }
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
            QString cfgFile = iter->directory + QDir::separator() +
                    TargetsManager::instance()->getCMakeConfigFile();
            if (!QFileInfo(cfgFile).isFile()) {
                configProject(&d->configureParam);
            }
        }
    }
}

} //namespace config
