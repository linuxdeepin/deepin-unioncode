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

#ifndef CONFIGUTIL_H
#define CONFIGUTIL_H

#include "services/project/projectservice.h"

#include <QObject>
#include <QVector>
#include <QMap>

namespace config {

enum ConfigType {
    Unknown = 0,
    Debug,
    Release
};

enum StepType {
    Build = 0,
    Clean,
    StepCount
};

struct StepItem {
    StepType type;
    QString targetName;
    QStringList targetList;
    QString arguments;
};

struct EnvironmentItem {
    bool enable;
    QMap<QString, QString> environments;
};

struct RunParam {
    QString targetName;
    QString targetPath;
    QString arguments;
    QString workDirectory;
    EnvironmentItem env;
};

struct RunConfigure {
    QString defaultTargetName;
    QVector<RunParam> params;
};

struct BuildConfigure {
    ConfigType type;
    QString directory;
    QVector<StepItem> steps;
    EnvironmentItem env;
    RunConfigure runConfigure;
};

struct ConfigureParam {
    QString kit;
    QString language;
    QString projectPath;
    ConfigType defaultType;
    QVector<BuildConfigure> buildConfigures;
};

class ConfigUtilPrivate;
class ConfigUtil final: public QObject
{
    Q_OBJECT
public:
    static ConfigUtil *instance();

    QString getConfigFilePath(const QString &projectPath, int configType);

    ConfigureParam *getConfigureParamPointer();

    QString getNameFromType(ConfigType type);
    ConfigType getTypeFromName(QString name);

    void checkConfigInfo(const QString &buildType, const QString &directory);

    void configProject(const QString &projectPath, const QString &language,
                       const BuildConfigure &configure);
signals:
    void configureDone(const dpfservice::ProjectInfo &info);

private:
    explicit ConfigUtil(QObject *parent = nullptr);
    ~ConfigUtil();

    ConfigUtilPrivate *const d;
};

} //namespace config

#endif // CONFIGUTIL_H
