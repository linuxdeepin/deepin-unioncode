#include "custompaths.h"
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

#ifdef LIBRARY_INSTALL_PREFIX
const QString librarysInstallPath = LIBRARY_INSTALL_PREFIX;
#else
const QString globalLibraryInstallPath {"/usr/lib/unioncode"}
#endif

#ifdef SOURCES_INSTALL_RPEFIX
const QString sourcesInstallPath = SOURCES_INSTALL_RPEFIX;
#else
const QString sourcesInstallPath {"/usr/share/unioncode"};
#endif

#ifdef RUNTIME_INSTALL_RPEFIX
const QString runtimeInstallPath = RUNTIME_INSTALL_RPEFIX;
#else
const QString runtimeInstallPath {"/usr/bin"};
#endif

QString CustomPaths::global(CustomPaths::Flage flage)
{
    switch (flage) {
    case Applition:
        return runtimeInstallPath;
    case DependLibs:
        return librarysInstallPath;
    case Plugins:
        return librarysInstallPath + QDir::separator() + "plugins";
    case Tools:
        return librarysInstallPath + QDir::separator() + "tools";
    case Extensions:
        return librarysInstallPath + QDir::separator() + "extensions";
    case Sources:
        return sourcesInstallPath;
    case Configures:
        return sourcesInstallPath + QDir::separator() + "configures";
    case Scripts:
        return sourcesInstallPath + QDir::separator() + "scripts";
    case Translations:
        return sourcesInstallPath + QDir::separator() + "translations";
    case Templates:
        return sourcesInstallPath + QDir::separator() + "templates";
    }
    return "";
}

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
