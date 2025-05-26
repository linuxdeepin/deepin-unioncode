// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
