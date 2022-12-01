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
#include "mainframe/pythongenerator.h"

#include "services/option/optionmanager.h"

namespace config {

class ConfigUtilPrivate
{
    friend class ConfigUtil;
    ConfigureParam configureParam;
};

ConfigUtil::ConfigUtil(QObject *parent)
    : QObject(parent)
    , d(new ConfigUtilPrivate())
{

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

bool ConfigUtil::getProjectInfo(const ConfigureParam *param, dpfservice::ProjectInfo &info)
{
    QString sourceFolder = QFileInfo(param->projectPath).path();
    info.setLanguage(param->language);
    info.setKitName(PythonGenerator::toolKitName());
    info.setWorkspaceFolder(sourceFolder);
    info.setBuildProgram(OptionManager::getInstance()->getPythonToolPath());

    return true;
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

void ConfigUtil::updateProjectInfo(dpfservice::ProjectInfo &info, const ConfigureParam *param)
{
    if (!param)
        return;

    info.setLanguage(param->language);
    info.setKitName(PythonGenerator::toolKitName());
    info.setWorkspaceFolder(param->projectPath);
    info.setBuildFolder(param->projectPath);
    info.setBuildProgram(param->pythonVersion.path);
}

} //namespace config
