// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configutil.h"
#include "project/jsprojectgenerator.h"

#include "services/option/optionmanager.h"

namespace config {

class ConfigUtilPrivate
{
    friend class ConfigUtil;
    ProjectConfigure configureParam;
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

ProjectConfigure *ConfigUtil::getConfigureParamPointer()
{
    return &d->configureParam;
}

bool ConfigUtil::getProjectInfo(const ProjectConfigure *param, dpfservice::ProjectInfo &info)
{
    QString sourceFolder = QFileInfo(param->projectPath).path();
    info.setLanguage(param->language);
    info.setKitName(JSProjectGenerator::toolKitName());
    info.setWorkspaceFolder(sourceFolder);

    return true;
}

QString ConfigUtil::getConfigPath(const QString &projectPath)
{
    return CustomPaths::projectCachePath(projectPath) + QDir::separator() + "project.properties";
}

void ConfigUtil::readConfig(const QString &filePath, ProjectConfigure &param)
{
    param.clear();

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        stream >> param;
        file.close();
    }
}

void ConfigUtil::saveConfig(const QString &filePath, const ProjectConfigure &param)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        QDataStream stream(&file);
        stream << param;
        file.close();
    }
}

void ConfigUtil::updateProjectInfo(dpfservice::ProjectInfo &info, const ProjectConfigure *param)
{
    if (!param)
        return;

    info.setLanguage(param->language);
    info.setKitName(JSProjectGenerator::toolKitName());
    info.setWorkspaceFolder(param->projectPath);
}

} //namespace config
