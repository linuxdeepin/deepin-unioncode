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

#ifndef MAVENCONFIGUTIL_H
#define MAVENCONFIGUTIL_H

#include "services/project/projectservice.h"

#include <QObject>
#include <QVector>
#include <QMap>

namespace mavenConfig {

extern const QString kJrePath;
extern const QString kJreExecute;
extern const QString kLaunchConfigPath;
extern const QString kLaunchPackageFile;
extern const QString kDapPackageFile;

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
    ItemInfo jdkVersion;
    ItemInfo mavenVersion;
    QString mainClass;
    QString jrePath;
    QString jreExecute;
    QString launchConfigPath;
    QString launchPackageFile;
    QString dapPackageFile;
    bool detailInfo = true;

    friend QDataStream &operator<<(QDataStream &stream, const ConfigureParam &data)
    {
        stream << data.kit;
        stream << data.language;
        stream << data.projectPath;
        stream << data.jdkVersion;
        stream << data.mavenVersion;
        stream << data.mainClass;
        stream << data.jrePath;
        stream << data.jreExecute;
        stream << data.launchConfigPath;
        stream << data.launchPackageFile;
        stream << data.dapPackageFile;
        stream << data.detailInfo;

        return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, ConfigureParam &data)
    {
        stream >> data.kit;
        stream >> data.language;
        stream >> data.projectPath;
        stream >> data.jdkVersion;
        stream >> data.mavenVersion;
        stream >> data.mainClass;
        stream >> data.jrePath;
        stream >> data.jreExecute;
        stream >> data.launchConfigPath;
        stream >> data.launchPackageFile;
        stream >> data.dapPackageFile;
        stream >> data.detailInfo;

        return stream;
    }

    void clear()
    {
        kit.clear();
        language.clear();
        projectPath.clear();
        jdkVersion.clear();
        mavenVersion.clear();
        mainClass.clear();
        jrePath.clear();
        jreExecute.clear();;
        launchConfigPath.clear();;
        launchPackageFile.clear();;
        dapPackageFile.clear();;
        detailInfo = true;
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

} //namespace mavenConfig

#endif // MAVENCONFIGUTIL_H
