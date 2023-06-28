// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    StepType type = Build;
    QString targetName;
    QStringList targetList;
    QString arguments;

    friend QDataStream &operator<<(QDataStream &stream, const StepItem &data)
    {
        stream << data.type;
        stream << data.targetName;
        stream << data.targetList;
        stream << data.arguments;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, StepItem &data)
    {
        int type = 0;
        stream >> type;
        data.type = static_cast<StepType>(type);
        stream >> data.targetName;
        stream >> data.targetList;
        stream >> data.arguments;

        return stream;
    }
};

struct EnvironmentItem {
    bool enable;
    QMap<QString, QString> environments;

    friend QDataStream &operator<<(QDataStream &stream, const EnvironmentItem &data)
    {
        stream << data.enable;
        stream << data.environments;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, EnvironmentItem &data)
    {
        stream >> data.enable;
        stream >> data.environments;

        return stream;
    }
};

struct RunParam {
    QString targetName;
    QString targetPath;
    QString arguments;
    QString workDirectory;
    EnvironmentItem env;

    friend QDataStream &operator<<(QDataStream &stream, const RunParam &data)
    {
        stream << data.targetName;
        stream << data.targetPath;
        stream << data.arguments;
        stream << data.workDirectory;
        stream << data.env;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, RunParam &data)
    {
        stream >> data.targetName;
        stream >> data.targetPath;
        stream >> data.arguments;
        stream >> data.workDirectory;
        stream >> data.env;

        return stream;
    }
};

struct RunConfigure {
    QString defaultTargetName;
    QVector<RunParam> params;

    friend QDataStream &operator<<(QDataStream &stream, const RunConfigure &data)
    {
        stream << data.defaultTargetName;
        stream << data.params;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, RunConfigure &data)
    {
        stream >> data.defaultTargetName;
        stream >> data.params;

        return stream;
    }
};

struct BuildConfigure {
    ConfigType type = Unknown;
    QString directory;
    QVector<StepItem> steps;
    EnvironmentItem env;
    RunConfigure runConfigure;

    friend QDataStream &operator<<(QDataStream &stream, const BuildConfigure &data)
    {
        stream << data.type;
        stream << data.directory;
        stream << data.steps;
        stream << data.env;
        stream << data.runConfigure;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, BuildConfigure &data)
    {
        int type = 0;
        stream >> type;
        data.type = static_cast<ConfigType>(type);
        stream >> data.directory;
        stream >> data.steps;
        stream >> data.env;
        stream >> data.runConfigure;

        return stream;
    }
};

struct ConfigureParam {
    QString kit;
    QString language;
    QString workspace;
    ConfigType defaultType = Unknown;
    ConfigType tempSelType = Unknown;
    QVector<BuildConfigure> buildConfigures;

    friend QDataStream &operator<<(QDataStream &stream, const ConfigureParam &data)
    {
        stream << data.kit;
        stream << data.language;
        stream << data.workspace;
        int type = data.defaultType;
        stream << type;
        type = data.tempSelType;
        stream << type;
        stream << data.buildConfigures;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, ConfigureParam &data)
    {
        stream >> data.kit;
        stream >> data.language;
        stream >> data.workspace;
        int type = 0;
        stream >> type;
        data.defaultType = static_cast<ConfigType>(type);
        stream >> type;
        data.tempSelType = static_cast<ConfigType>(type);
        stream >> data.buildConfigures;

        return stream;
    }

    void clear()
    {
        kit.clear();
        language.clear();
        workspace.clear();
        defaultType = Unknown;
        tempSelType = Unknown;
        buildConfigures.clear();
    }
};

class ConfigUtilPrivate;
class ConfigUtil final: public QObject
{
    Q_OBJECT
public:
    static ConfigUtil *instance();

    ConfigUtil(const ConfigUtil &) = delete;
    ConfigUtil &operator=(const ConfigUtil &) = delete;

    QString getConfigPath(const QString &workspace);

    ConfigureParam *getConfigureParamPointer();

    QString getNameFromType(ConfigType type);
    ConfigType getTypeFromName(QString name);

    dpfservice::ProjectInfo createProjectInfo(const ConfigureParam *param);
    bool isNeedConfig(const QString &workspace, ConfigureParam &param);
    void checkConfigInfo(const QString &buildType, const QString &directory);
    void configProject(const ConfigureParam *param);

    void readConfig(const QString &filePath, ConfigureParam &param);
    void saveConfig(const QString &filePath, const ConfigureParam &param);
    bool updateProjectInfo(dpfservice::ProjectInfo &info, const ConfigureParam *param);
signals:
    void configureDone(const dpfservice::ProjectInfo &info);

private:
    explicit ConfigUtil(QObject *parent = nullptr);
    ~ConfigUtil();

    ConfigUtilPrivate *const d;
};

} //namespace config

#endif // CONFIGUTIL_H
