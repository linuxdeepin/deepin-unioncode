// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "custompaths.h"
#include "fileoperation.h"
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

using FO = FileOperation;
class PathMode
{
    PathMode() = delete;
    Q_DISABLE_COPY(PathMode)
public:
    static QString installed(CustomPaths::Flags flags);
    static QString builded(CustomPaths::Flags flags);
    static bool isRunAppBuilded();
    static bool isInstalled(CustomPaths::Flags flags, const QString &path);
    static QString userHome();
    static QString usreCachePath();
    static QString userConfigurePath();
    static QString userDataPath();
};

QString formatString(QString str)
{
    if (str.back() == QDir::separator()) {
        str.chop(1);
    }
    return str;
}

QString PathMode::installed(CustomPaths::Flags flags)
{
    switch (flags) {
    case CustomPaths::Applition:
        return formatString(RUNTIME_INSTALL_RPEFIX);
    case CustomPaths::DependLibs:
        return formatString(LIBRARY_INSTALL_PREFIX);
    case CustomPaths::Plugins:
        return formatString(PLUGIN_INSTALL_RPEFIX);
    case CustomPaths::Tools:
        return formatString(LIBRARY_INSTALL_PREFIX) + QDir::separator() + "tools";
    case CustomPaths::Packages:
        return formatString(LIBRARY_INSTALL_PREFIX) + QDir::separator() + "packages";
    case CustomPaths::Resources:
        return formatString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "resource";
    case CustomPaths::Configures:
        return formatString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "configures";
    case CustomPaths::Scripts:
        return formatString(LIBRARY_INSTALL_PREFIX) + QDir::separator() + "scripts";
    case CustomPaths::Translations:
        return formatString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "translations";
    case CustomPaths::Templates:
        return formatString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "templates";
    case CustomPaths::Models:
        return formatString(SOURCES_INSTALL_RPEFIX) + QDir::separator() + "models";
    default:
        return "";
    }
}

bool PathMode::isInstalled(CustomPaths::Flags flags, const QString &path)
{
    switch (flags) {
    case CustomPaths::Applition:
        return RUNTIME_INSTALL_RPEFIX == path || path.startsWith(LIBRARY_INSTALL_PREFIX); //debugAdapter installed in '/usr/lib/xxx/deepin-unioncode/tools' not '/usr/bin'
    default:
        return false;
    }
}

QString PathMode::userHome()
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString PathMode::builded(CustomPaths::Flags flags)
{
    switch (flags) {
    case CustomPaths::Applition:
        return formatString(RUNTIME_BUILD_RPEFIX);
    case CustomPaths::DependLibs:
        return formatString(LIBRARY_BUILD_PREFIX);
    case CustomPaths::Plugins:
        return formatString(PLUGIN_BUILD_RPEFIX);
    case CustomPaths::Tools:
        return formatString(RUNTIME_BUILD_RPEFIX);
    case CustomPaths::Resources:
        return formatString(PROJECT_SOURCE_RPEFIX) + QDir::separator() + "resource";
    case CustomPaths::Configures:
        return formatString(ASSETS_SOURCE_RPEFIX) + QDir::separator() + "configures";
    case CustomPaths::Scripts:
        return formatString(PROJECT_SOURCE_RPEFIX) + QDir::separator() + "scripts";
    case CustomPaths::Templates:
        return formatString(ASSETS_SOURCE_RPEFIX) + QDir::separator() + "templates";
    case CustomPaths::Translations:
        return formatString(ASSETS_SOURCE_RPEFIX) + QDir::separator() + "translations";
    case CustomPaths::Models:
        return formatString(ASSETS_SOURCE_RPEFIX) + QDir::separator() + "models";
    default:
        return "";
    }
}

QString PathMode::usreCachePath()
{
    return FO::checkCreateDir(FO::checkCreateDir(userHome(), ".cache"), "deepin-unioncode");
}

QString PathMode::userConfigurePath()
{
    return FO::checkCreateDir(FO::checkCreateDir(userHome(), ".config"), "deepin-unioncode");
}

QString PathMode::userDataPath()
{
    return FO::checkCreateDir(FO::checkCreateDir(userHome(), ".data"), "deepin-unioncode");
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

QString CustomPaths::lspRuntimePath(const QString &language)
{
    QString lspRuntimePath = FO::checkCreateDir(CustomPaths::user(CustomPaths::Tools), "lsp");
    if (language.isEmpty()) {
        return lspRuntimePath;
    } else {
        QString languageTemp = language;
        languageTemp = languageTemp.replace(QDir::separator(), "_");
        return FO::checkCreateDir(lspRuntimePath, languageTemp);
    }
}

bool CustomPaths::checkDir(const QString &path)
{
    if (!QDir(path).exists()) {
        return QDir().mkpath(path);
    }
    return false;
}

QString CustomPaths::projectCachePath(const QString &projectPath)
{
    return FO::checkCreateDir(projectPath, ".unioncode");
}

QString CustomPaths::user(CustomPaths::Flags flag)
{
    switch (flag) {
    case Applition:
        return qApp->applicationDirPath();
    case Plugins:
        return FileOperation::checkCreateDir(PathMode::usreCachePath(), "plugins");
    case Tools:
        return FileOperation::checkCreateDir(PathMode::usreCachePath(), "tools");
    case Extensions:
        return FileOperation::checkCreateDir(PathMode::usreCachePath(), "extensions");
    case Configures:
        return FileOperation::checkCreateDir(PathMode::userConfigurePath(), "configures");
    case Scripts:
        return FileOperation::checkCreateDir(PathMode::userConfigurePath(), "Scripts");
    case Templates:
        return FileOperation::checkCreateDir(PathMode::userConfigurePath(), "templates");
    case Models:
        return FileOperation::checkCreateDir(PathMode::userConfigurePath(), "models");
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
