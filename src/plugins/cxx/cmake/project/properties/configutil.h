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
    QString activeTargetName;
    QStringList allTargetNames;
    QStringList buildArguments;

    friend QDataStream &operator<<(QDataStream &stream, const StepItem &data)
    {
        stream << data.type;
        stream << data.activeTargetName;
        stream << data.allTargetNames;
        stream << data.buildArguments;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, StepItem &data)
    {
        int type = 0;
        stream >> type;
        data.type = static_cast<StepType>(type);
        stream >> data.activeTargetName;
        stream >> data.allTargetNames;
        stream >> data.buildArguments;

        return stream;
    }
};

struct EnvironmentItem {
    bool enable = true;
    QMap<QString, QString> environments;

    void setQDebugLevel(bool enable)
    {
        enableQDebugLevel = enable;
        QString loggingValue = enableQDebugLevel ? "*.debug=true" : "*.debug=false";
        environments.insert("QT_LOGGING_RULES", loggingValue);
    }

    bool isQDebugLevelEnable() const
    {
        return enableQDebugLevel;
    }

    EnvironmentItem() {
        initEnvironments();
    }

    QStringList toList() const
    {
        QStringList envList;
        for (auto it = environments.begin() ; it != environments.end() ; it++) {
            envList.append(it.key() + "=" + it.value());
        }
        return envList;
    }

    void initEnvironments()
    {
        environments.clear();

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        foreach (auto key, env.keys()) {
            environments.insert(key, env.value(key));
        }
        setQDebugLevel(enableQDebugLevel);
    }

    friend QDataStream &operator<<(QDataStream &stream, const EnvironmentItem &data)
    {
        stream << data.enable;
        stream << data.enableQDebugLevel;
        stream << data.environments;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, EnvironmentItem &data)
    {
        stream >> data.enable;
        stream >> data.enableQDebugLevel;
        stream >> data.environments;

        return stream;
    }

private:
    bool enableQDebugLevel = true;
};

struct TargetRunConfigure {
    QString targetName;
    QString targetPath;
    QString arguments;
    QString workDirectory;
    EnvironmentItem env;

    friend QDataStream &operator<<(QDataStream &stream, const TargetRunConfigure &data)
    {
        stream << data.targetName;
        stream << data.targetPath;
        stream << data.arguments;
        stream << data.workDirectory;
        stream << data.env;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, TargetRunConfigure &data)
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
    QVector<TargetRunConfigure> targetsRunConfigure;

    friend QDataStream &operator<<(QDataStream &stream, const RunConfigure &data)
    {
        stream << data.defaultTargetName;
        stream << data.targetsRunConfigure;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, RunConfigure &data)
    {
        stream >> data.defaultTargetName;
        stream >> data.targetsRunConfigure;

        return stream;
    }
};

struct BuildConfigue {
    EnvironmentItem env;
    QVector<StepItem> steps;

    friend QDataStream &operator<<(QDataStream &stream, const BuildConfigue &data)
    {
        stream << data.env;
        stream << data.steps;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, BuildConfigue &data)
    {
        stream >> data.env;
        stream >> data.steps;

        return stream;
    }
};

struct BuildTypeConfigure {
    ConfigType type = Unknown;
    QString directory;
    BuildConfigue buildConfigure;
    RunConfigure runConfigure;

    friend QDataStream &operator<<(QDataStream &stream, const BuildTypeConfigure &data)
    {
        stream << data.type;
        stream << data.directory;
        stream << data.buildConfigure;
        stream << data.runConfigure;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, BuildTypeConfigure &data)
    {
        int type = 0;
        stream >> type;
        data.type = static_cast<ConfigType>(type);
        stream >> data.directory;
        stream >> data.buildConfigure;
        stream >> data.runConfigure;

        return stream;
    }
};

using BuildTypeConfigures = QVector<BuildTypeConfigure>;
struct ProjectConfigure {
    QString kit;
    QString language;
    QString workspace;
    ConfigType defaultType = Unknown;
    ConfigType tempSelType = Unknown;
    BuildTypeConfigures buildTypeConfigures; // debug、release e.g

    friend QDataStream &operator<<(QDataStream &stream, const ProjectConfigure &data)
    {
        stream << data.kit;
        stream << data.language;
        stream << data.workspace;
        int type = data.defaultType;
        stream << type;
        type = data.tempSelType;
        stream << type;
        stream << data.buildTypeConfigures;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, ProjectConfigure &data)
    {
        stream >> data.kit;
        stream >> data.language;
        stream >> data.workspace;
        int type = 0;
        stream >> type;
        data.defaultType = static_cast<ConfigType>(type);
        stream >> type;
        data.tempSelType = static_cast<ConfigType>(type);
        stream >> data.buildTypeConfigures;

        return stream;
    }

    void clear()
    {
        kit.clear();
        language.clear();
        workspace.clear();
        defaultType = Unknown;
        tempSelType = Unknown;
        buildTypeConfigures.clear();
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

    ProjectConfigure *getConfigureParamPointer();

    QString getNameFromType(ConfigType type);
    ConfigType getTypeFromName(QString name);

    dpfservice::ProjectInfo createProjectInfo(const ProjectConfigure *param);
    bool isNeedConfig(const QString &workspace, ProjectConfigure &param);
    void checkConfigInfo(const QString &buildType, const QString &directory);
    void configProject(const ProjectConfigure *param);

    void readConfig(const QString &filePath, ProjectConfigure &param);
    void saveConfig(const QString &filePath, const ProjectConfigure &param);
    bool updateProjectInfo(dpfservice::ProjectInfo &info, const ProjectConfigure *param);

signals:
    void configureDone(const dpfservice::ProjectInfo &info);

private:
    explicit ConfigUtil(QObject *parent = nullptr);
    ~ConfigUtil();

    ConfigUtilPrivate *const d;
};

} //namespace config

#endif // CONFIGUTIL_H
