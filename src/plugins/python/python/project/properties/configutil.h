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

struct ConfigureParam {
    QString kit;
    QString language;
    QString projectPath;
    ItemInfo pythonVersion;

    friend QDataStream &operator<<(QDataStream &stream, const ConfigureParam &data)
    {
        stream << data.kit;
        stream << data.language;
        stream << data.projectPath;
        stream << data.pythonVersion;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, ConfigureParam &data)
    {
        stream >> data.kit;
        stream >> data.language;
        stream >> data.projectPath;
        stream >> data.pythonVersion;

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

    ConfigureParam *getConfigureParamPointer();

    bool getProjectInfo(const ConfigureParam *param, dpfservice::ProjectInfo &info);

    void readConfig(const QString &filePath, ConfigureParam &param);
    void saveConfig(const QString &filePath, const ConfigureParam &param);

    void updateProjectInfo(dpfservice::ProjectInfo &info, const ConfigureParam *param);
signals:

private:
    explicit ConfigUtil(QObject *parent = nullptr);
    ~ConfigUtil();

    ConfigUtilPrivate *const d;
};

} //namespace config

#endif // CONFIGUTIL_H
