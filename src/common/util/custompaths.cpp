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
