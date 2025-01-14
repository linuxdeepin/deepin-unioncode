// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGUTIL_H
#define CONFIGUTIL_H

#include "services/project/projectservice.h"

#include <QObject>
#include <QVector>
#include <QMap>

constexpr char kVersion[] { "version" };
constexpr char kConfigVersion[] { "1.0" };
constexpr char kKit[] { "kit" };
constexpr char kKitId[] { "kitId" };
constexpr char kLanguage[] { "language" };
constexpr char kWorkspace[] { "workspace" };
constexpr char kDefaultType[] { "defaultType" };
constexpr char kTempSelType[] { "tempSelType" };
constexpr char kBuildTypeConfigures[] { "buildTypeConfigures" };
constexpr char kType[] { "type" };
constexpr char kDirectory[] { "directory" };
constexpr char kBuildConfigure[] { "buildConfigure" };
constexpr char kEnvironmentItem[] { "environmentItem" };
constexpr char kEnable[] { "enable" };
constexpr char kEnvironments[] { "environments" };
constexpr char kSteps[] { "steps" };
constexpr char kActiveTargetName[] { "activeTargetName" };
constexpr char kAllTargetNames[] { "allTargetNames" };
constexpr char kBuildArguments[] { "buildArguments" };
constexpr char kRunConfigure[] { "runConfigure" };
constexpr char kDefaultTargetName[] { "defaultTargetName" };
constexpr char kTargetsRunConfigure[] { "targetsRunConfigure" };
constexpr char kTargetName[] { "targetName" };
constexpr char kTargetPath[] { "targetPath" };
constexpr char kArguments[] { "arguments" };
constexpr char kWorkDirectory[] { "workDirectory" };
constexpr char kRunInTermal[] { "runInTermal" };

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

struct StepItem
{
    StepType type = Build;
    QString activeTargetName;
    QStringList allTargetNames;
    QStringList buildArguments;

    inline void fromJson(const QJsonObject &obj)
    {
        auto arrayToStringList = [](const QJsonArray &array) {
            QStringList list;
            for (const auto &value : array) {
                list << value.toString();
            }
            return list;
        };

        activeTargetName = obj.value(kActiveTargetName).toString();
        type = static_cast<StepType>(obj.value(kType).toInt());
        allTargetNames = arrayToStringList(obj.value(kAllTargetNames).toArray());
        buildArguments = arrayToStringList(obj.value(kBuildArguments).toArray());
    }

    inline QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(kType, type);
        obj.insert(kActiveTargetName, activeTargetName);
        obj.insert(kAllTargetNames, QJsonArray::fromStringList(allTargetNames));
        obj.insert(kBuildArguments, QJsonArray::fromStringList(buildArguments));
        return obj;
    }
};

struct EnvironmentItem
{
    bool enable = true;
    QVariantMap environments;

    EnvironmentItem()
    {
        initEnvironments();
    }

    QStringList toList() const
    {
        QStringList envList;
        for (auto it = environments.begin(); it != environments.end(); it++) {
            envList.append(it.key() + "=" + it.value().toString());
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
    }

    inline void fromJson(const QJsonObject &obj)
    {
        enable = obj.value(kEnable).toBool();
        environments = obj.value(kEnvironments).toVariant().toMap();
    }

    inline QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(kEnable, enable);
        obj.insert(kEnvironments, QJsonObject::fromVariantMap(environments));
        return obj;
    }
};

struct TargetRunConfigure
{
    QString targetName;
    QString targetPath;
    QString arguments;
    QString workDirectory;
    EnvironmentItem env;
    bool runInTermal { false };

    inline void fromJson(const QJsonObject &obj)
    {
        targetName = obj.value(kTargetName).toString();
        targetPath = obj.value(kTargetPath).toString();
        arguments = obj.value(kArguments).toString();
        workDirectory = obj.value(kWorkDirectory).toString();
        runInTermal = obj.value(kRunInTermal).toBool();
        env.fromJson(obj.value(kEnvironmentItem).toObject());
    }

    inline QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(kTargetName, targetName);
        obj.insert(kTargetPath, targetPath);
        obj.insert(kArguments, arguments);
        obj.insert(kWorkDirectory, workDirectory);
        obj.insert(kRunInTermal, runInTermal);
        obj.insert(kEnvironmentItem, env.toJson());
        return obj;
    }

    bool operator == (const TargetRunConfigure &t) const
    {
        return this->targetName == t.targetName;
    }
};

struct RunConfigure
{
    QString defaultTargetName;
    QVector<TargetRunConfigure> targetsRunConfigure;

    inline void fromJson(const QJsonObject &obj)
    {
        defaultTargetName = obj.value(kDefaultTargetName).toString();
        const auto &array = obj.value(kTargetsRunConfigure).toArray();
        for (const auto &value : array) {
            TargetRunConfigure config;
            config.fromJson(value.toObject());
            targetsRunConfigure << config;
        }
    }

    inline QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(kDefaultTargetName, defaultTargetName);

        QJsonArray array;
        for (const auto &config : targetsRunConfigure) {
            array.append(config.toJson());
        }
        obj.insert(kTargetsRunConfigure, array);
        return obj;
    }
};

struct BuildConfigue
{
    EnvironmentItem env;
    QVector<StepItem> steps;

    inline void fromJson(const QJsonObject &obj)
    {
        env.fromJson(obj.value(kEnvironmentItem).toObject());
        const auto &array = obj.value(kSteps).toArray();
        for (const auto &value : array) {
            StepItem item;
            item.fromJson(value.toObject());
            steps << item;
        }
    }

    inline QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(kEnvironmentItem, env.toJson());

        QJsonArray array;
        for (const auto &step : steps) {
            array.append(step.toJson());
        }
        obj.insert(kSteps, array);
        return obj;
    }
};

struct BuildTypeConfigure
{
    ConfigType type = Unknown;
    QString directory;
    BuildConfigue buildConfigure;
    RunConfigure runConfigure;

    inline void fromJson(const QJsonObject &obj)
    {
        type = static_cast<ConfigType>(obj.value(kType).toInt());
        directory = obj.value(kDirectory).toString();
        buildConfigure.fromJson(obj.value(kBuildConfigure).toObject());
        runConfigure.fromJson(obj.value(kRunConfigure).toObject());
    }

    inline QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(kType, type);
        obj.insert(kDirectory, directory);
        obj.insert(kBuildConfigure, buildConfigure.toJson());
        obj.insert(kRunConfigure, runConfigure.toJson());
        return obj;
    }
};

using BuildTypeConfigures = QVector<BuildTypeConfigure>;
struct ProjectConfigure
{
    QString kit;
    QString kitId;
    QString language;
    QString workspace;
    ConfigType defaultType = Unknown;
    ConfigType tempSelType = Unknown;
    BuildTypeConfigures buildTypeConfigures;   // debug、release e.g

    inline bool fromJson(const QJsonObject &obj)
    {
        QString version = obj.value(kVersion).toString();
        if (kConfigVersion != version)
            return false;

        kit = obj.value(kKit).toString();
        kitId = obj.value(kKitId).toString();
        language = obj.value(kLanguage).toString();
        workspace = obj.value(kWorkspace).toString();
        defaultType = static_cast<ConfigType>(obj.value(kDefaultType).toInt());
        tempSelType = static_cast<ConfigType>(obj.value(kTempSelType).toInt());

        const auto &array = obj.value(kBuildTypeConfigures).toArray();
        for (const auto &value : array) {
            BuildTypeConfigure config;
            config.fromJson(value.toObject());
            buildTypeConfigures << config;
        }
        return true;
    }

    inline QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(kVersion, kConfigVersion);
        obj.insert(kKit, kit);
        obj.insert(kKitId, kitId);
        obj.insert(kLanguage, language);
        obj.insert(kWorkspace, workspace);
        obj.insert(kDefaultType, defaultType);
        obj.insert(kTempSelType, tempSelType);

        QJsonArray array;
        for (const auto &config : buildTypeConfigures) {
            array.append(config.toJson());
        }
        obj.insert(kBuildTypeConfigures, array);
        return obj;
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
class ConfigUtil final : public QObject
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

    bool readConfig(const QString &filePath, ProjectConfigure &param);
    void saveConfig(const QString &filePath, const ProjectConfigure &param);
    bool updateProjectInfo(dpfservice::ProjectInfo &info, const ProjectConfigure *param);

    void switchConfigType(const ConfigType &type);
signals:
    void configureDone(const dpfservice::ProjectInfo &info);
    void configTypeSwitched(const ConfigType &type);

private:
    explicit ConfigUtil(QObject *parent = nullptr);
    ~ConfigUtil();

    ConfigUtilPrivate *const d;
};

}   //namespace config

#endif   // CONFIGUTIL_H
