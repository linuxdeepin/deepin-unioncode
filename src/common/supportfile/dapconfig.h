// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAPCONFIG_H
#define DAPCONFIG_H

#include <QString>

namespace support_file {

struct JavaDapPluginConfig {
    QString launchPackageFile;
    QString launchConfigPath;
    QString dapPackageFile;
    QString jrePath;
    QString jreExecute;
};

class DapSupportConfig final
{
public:
    static QString globalPath();
    static QString userPath();
    static bool readFromSupportFile(const QString &dapSupportPath, const QString &arch, JavaDapPluginConfig &javaconfig, const QString &configDir);

private:
    explicit DapSupportConfig();
};

}
#endif // DAPCONFIG_H
