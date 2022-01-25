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
#include "config.h"
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QtConcurrent>

/*!
 * 程序运行有两种状态:
 * 1. 程序运行于/usr/bin 则成为installed状态
 *     (1). 在installed状态下运行程序时，分别对应加载全局配置文件、插件资源等(安装时的资源路径)。
 *     (2). 还会加载用户目录下会存在相应副本，用于用户临时更改的任何配置文件、插件资源(用户使用生成，用户作用域)。
 *     这么做的原因是，区别于用户绑定的插件、或者配置项。即切换用户后在用户目录下存在不同的插件场景
 * 2. 程序运行于非/usr/bin 则统称为builded状态，该状态下默认指向源码构建。
 *
 * 对于class CustomPaths接口user/global而言，两种组态外部调用者并不关心。
 * 所以内部将通过程序运行时分别两种组态以达到不同的组合运行效果
 */

class PathMode
{
    PathMode() = delete;
    Q_DISABLE_COPY(PathMode)
public:
    static QString installed(CustomPaths::Flags flags);
    static QString builded(CustomPaths::Flags flags);
    static bool isRunAppBuilded();
    static bool isInstalled(CustomPaths::Flags flags, const QString &path);
    static QString usreCachePath();
    static QString userConfigurePath();
};

QString PathMode::installed(CustomPaths::Flags flags)
{
    switch (flags) {
    case CustomPaths::Applition:
        return RUNTIME_INSTALL_RPEFIX;
    case CustomPaths::DependLibs:
        return LIBRARY_INSTALL_PREFIX;
    case CustomPaths::Plugins:
        return PLUGIN_INSTALL_RPEFIX;
    case CustomPaths::Tools:
        return QString(LIBRARY_INSTALL_PREFIX) + QDir::separator() + "tools";
    case CustomPaths::Packages:
        return QString(LIBRARY_INSTALL_PREFIX) + QDir::separator() + "packages";
    case CustomPaths::Resources:
        return QString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "resource";
    case CustomPaths::Configures:
        return QString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "configures";
    case CustomPaths::Scripts:
        return QString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "scripts";
    case CustomPaths::Translations:
        return QString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "translations";
    case CustomPaths::Templates:
        return QString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "template";
    default:
        return "";
    }
}

bool PathMode::isInstalled(CustomPaths::Flags flags, const QString &path)
{
    switch (flags) {
    case CustomPaths::Applition:
        return RUNTIME_INSTALL_RPEFIX == path;
    default:
        return false;
    }
}

QString PathMode::builded(CustomPaths::Flags flags)
{
    switch (flags) {
    case CustomPaths::Applition:
        return RUNTIME_BUILD_RPEFIX;
    case CustomPaths::DependLibs:
        return LIBRARY_BUILD_PREFIX;
    case CustomPaths::Plugins:
        return PLUGIN_BUILD_RPEFIX;
    case CustomPaths::Tools:
        return RUNTIME_BUILD_RPEFIX;
    case CustomPaths::Resources:
        return QString(PROJECT_SOURCE_RPEFIX) + QDir::separator() + "resource";
    case CustomPaths::Configures:
        return QString(PROJECT_SOURCE_RPEFIX) + QDir::separator() + "configures";
    case CustomPaths::Scripts:
        return QString(PROJECT_SOURCE_RPEFIX) + QDir::separator() + "scripts";
    default:
        return "";
    }
}

QString PathMode::usreCachePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
}

QString PathMode::userConfigurePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString CustomPaths::endSeparator(const QString &path)
{
    if (!path.endsWith(QDir::separator()))
        return path + QDir::separator();
    return path;
}

QString CustomPaths::projectGeneratePath(const QString &path)
{
    auto result = endSeparator(path) + "build";
    if (!QDir(result).exists()) {
        QDir().mkdir(result);
    }
    return result;
}

void CustomPaths::checkDir(const QString &path)
{
    if (!QDir(path).exists()) {
        QDir().mkpath(path);
    }
}

QString CustomPaths::user(CustomPaths::Flags flage)
{
    switch (flage) {
    case Applition:
        return qApp->applicationDirPath();
    case Plugins:
        return PathMode::usreCachePath() + QDir::separator() + "plugins";
    case Tools:
        return PathMode::usreCachePath() + QDir::separator() + "tools";
    case Extensions:
        return PathMode::usreCachePath() + QDir::separator() + "extensions";
    case Configures:
        return PathMode::userConfigurePath() + QDir::separator() + "configures";
    default:
        return "";
    }
}

QString CustomPaths::global(CustomPaths::Flags flags)
{
    if (installed()) {
        return PathMode::installed(flags);
    } else {
        return PathMode::builded(flags);
    }
}

bool CustomPaths::installed()
{
    return PathMode::isInstalled(CustomPaths::Applition, QCoreApplication::applicationDirPath());
}
