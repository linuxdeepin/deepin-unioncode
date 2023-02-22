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
#ifndef DAPCONFIG_H
#define DAPCONFIG_H

#include <QString>

namespace support_file {

struct JavaDapPluginConfig {
    QString configHomePath;
    QString launchPackageName;
    QString launchPackagePath;
    QString dapPackageName;
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
