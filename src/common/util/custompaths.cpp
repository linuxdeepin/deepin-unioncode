/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "custompaths.h"
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

QString usreCachePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
}

QString userConfigurePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString CustomPaths::user(CustomPaths::Flage flage)
{
    switch (flage) {
    case Applition:
        return qApp->applicationDirPath();
    case Plugins:
        return usreCachePath() + QDir::separator() + "plugins";
    case Tools:
        return usreCachePath() + QDir::separator() + "tools";
    case Extensions:
        return usreCachePath() + QDir::separator() + "extensions";
    case Configures:
        return userConfigurePath() + QDir::separator() + "configures";
    default:
        return "";
    }
}

bool CustomPaths::installed()
{
    return RUNTIME_INSTALL_RPEFIX == QCoreApplication::applicationDirPath();
}
