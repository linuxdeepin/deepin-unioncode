// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGUTIL_H
#define CONFIGUTIL_H

#include "services/project/projectservice.h"

#include <QObject>
#include <QVector>
#include <QMap>

static const QString exeCurrent = QObject::tr("Current File");
static const QString exeEntry = QObject::tr("Automatically obtain the entry file");

namespace config {

enum ExecuteFile {
    CURRENTFILE,
    ENTRYFILE
};

struct ItemInfo {
    QString name;
    QString path;

    friend QDataStream &operator<<(QDataStream &stream, const ItemInfo &data)
    {
        stream << data.name;
        stream << data.path;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, ItemInfo &data)
    {
        stream >> data.name;
        stream >> data.path;

        return stream;
    }

    void clear()
    {
        name.clear();
        path.clear();
    }
};

struct ProjectConfigure {
    QString kit;
    QString language;
    QString projectPath;
    ItemInfo pythonVersion;
    ExecuteFile executeFile;
    bool runInTerminal;

    friend QDataStream &operator<<(QDataStream &stream, const ProjectConfigure &data)
    {
        stream << data.kit;
        stream << data.language;
        stream << data.projectPath;
        stream << data.pythonVersion;
        stream << static_cast<qint8>(data.executeFile);
        stream << data.runInTerminal;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, ProjectConfigure &data)
    {
        stream >> data.kit;
        stream >> data.language;
        stream >> data.projectPath;
        stream >> data.pythonVersion;
        qint8 temp;
        stream >> temp;
        data.executeFile = static_cast<ExecuteFile>(temp);
        stream >> data.runInTerminal;

        return stream;
    }

    void clear()
    {
        kit.clear();
        language.clear();
        projectPath.clear();
        pythonVersion.clear();
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

    QString getConfigPath(const QString &projectPath);

    ProjectConfigure *getConfigureParamPointer();

    bool getProjectInfo(const ProjectConfigure *param, dpfservice::ProjectInfo &info);

    void readConfig(const QString &filePath, ProjectConfigure &param);
    void saveConfig(const QString &filePath, const ProjectConfigure &param);

    void updateProjectInfo(dpfservice::ProjectInfo &info, const ProjectConfigure *param);
signals:

private:
    explicit ConfigUtil(QObject *parent = nullptr);
    ~ConfigUtil();

    ConfigUtilPrivate *const d;
};

} //namespace config

#endif // CONFIGUTIL_H
