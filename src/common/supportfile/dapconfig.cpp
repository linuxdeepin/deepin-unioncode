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
#include "dapconfig.h"
#include "util/custompaths.h"
#include "util/processutil.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

namespace support_file {

DapSupportConfig::DapSupportConfig()
{

}

QString DapSupportConfig::globalPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + QString("dapconfig.support");
}

QString DapSupportConfig::userPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    return result + QString("dapconfig.support");
}

bool DapSupportConfig::readFromSupportFile(const QString &filePath, const QString &arch, JavaDapPluginConfig &javaconfig, const QString &configHome)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (QJsonParseError::NoError != parseError.error)
        return false;

    if (!doc.isObject())
        return false;

    QJsonObject rootObject = doc.object();
    if (!rootObject.contains(arch))
        return false;

    QJsonObject archObject = rootObject.value(arch).toObject();
    if (archObject.isEmpty())
        return false;

    QJsonObject valueObject = archObject.value("java_config_path").toObject();
    if (valueObject.isEmpty())
        return false;

    javaconfig.launchPackageFile = configHome + valueObject.value("launch_package_file").toString();
    javaconfig.launchConfigPath = configHome + valueObject.value("launch_config_path").toString();
    javaconfig.dapPackageFile = configHome + valueObject.value("dap_package_file").toString();
    QString jrePath = valueObject.value("jre_path").toString();
    if (jrePath.isEmpty()) {
        QString ret = ProcessUtil::execute({"which java"}, true);
        auto getLink = [](QString &file)->QString{
            QString ret = ProcessUtil::execute({QString("ls -lrt %1").arg(file)}, true);
            if (!ret.isEmpty()) {
                ret = ret.split(" -> ").back();
            }
            return ret;
        };
        QString etc = getLink(ret);
        QString link = getLink(etc);
        javaconfig.jreExecute = link;
        javaconfig.jrePath = link.split("bin/java").front();
    } else {
        javaconfig.jrePath = configHome + jrePath;
        javaconfig.jreExecute = configHome + valueObject.value("jre_execute").toString();
    }
    return true;
}

} // namespace support_file
