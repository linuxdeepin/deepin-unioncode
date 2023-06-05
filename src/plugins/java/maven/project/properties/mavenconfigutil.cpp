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

#include "mavenconfigutil.h"
#include "maven/project/mavenprojectgenerator.h"

#include "services/option/optionmanager.h"

namespace mavenConfig {

const QString kJrePath = "jrePath";
const QString kJreExecute = "jreExecute";
const QString kLaunchConfigPath = "launchConfigPath";
const QString kLaunchPackageFile = "launchPackageFile";
const QString kDapPackageFile = "dapPackageFile";

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
    info.setKitName(MavenProjectGenerator::toolKitName());
    info.setWorkspaceFolder(sourceFolder);
    info.setBuildProgram(OptionManager::getInstance()->getMavenToolPath());
    info.setDetailInformation(param->detailInfo);
    info.setProperty(kJrePath, param->jreExecute);
    info.setProperty(kJreExecute, param->launchConfigPath);
    info.setProperty(kLaunchConfigPath, param->launchConfigPath);
    info.setProperty(kLaunchPackageFile, param->launchPackageFile);
    info.setProperty(kDapPackageFile, param->dapPackageFile);

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

void ConfigUtil::updateProjectInfo(dpfservice::ProjectInfo &info, const ConfigureParam *cfgParams)
{
    if (!cfgParams)
        return;

    info.setLanguage(cfgParams->language);
    info.setKitName(MavenProjectGenerator::toolKitName());
    info.setWorkspaceFolder(cfgParams->projectPath);
    info.setBuildFolder(cfgParams->projectPath);
    info.setBuildProgram(cfgParams->mavenVersion.path);
    info.setDetailInformation(cfgParams->detailInfo);
    info.setProperty(kJrePath, cfgParams->jrePath);
    info.setProperty(kJreExecute, cfgParams->jreExecute);
    info.setProperty(kLaunchConfigPath, cfgParams->launchConfigPath);
    info.setProperty(kLaunchPackageFile, cfgParams->launchPackageFile);
    info.setProperty(kDapPackageFile, cfgParams->dapPackageFile);
}

} //namespace mavenConfig
