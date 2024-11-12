// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionmanager.h"
#include "optiondatastruct.h"

#include "common/settings/settings.h"
#include "common/util/custompaths.h"

#include <QVariant>
#include <QDir>

class OptionManagerPrivate final
{
public:
    QString configFile() const;

    Settings optionSettings;
};

QString OptionManagerPrivate::configFile() const
{
    const QString configDir = CustomPaths::user(CustomPaths::Flags::Configures);
    const QString oldConfigPath = configDir + QDir::separator() + "optionparam.support";
    const QString newConfigPath = configDir + QDir::separator() + "deepin-unioncode.json";

    QFile oldFile(oldConfigPath);
    if (oldFile.exists())
        oldFile.rename(newConfigPath);

    return newConfigPath;
}

OptionManager::OptionManager(QObject *parent)
    : QObject(parent), d(new OptionManagerPrivate())
{
    d->optionSettings.load("", d->configFile());
}

OptionManager::~OptionManager()
{
    if (d) {
        delete d;
    }
}

OptionManager *OptionManager::getInstance()
{
    static OptionManager ins;
    return &ins;
}

QString OptionManager::getMavenToolPath() const
{
    const auto &maven = d->optionSettings.value(option::CATEGORY_MAVEN, "Maven");
    return maven.toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getJdkToolPath() const
{
    const auto &jdk = d->optionSettings.value(option::CATEGORY_JAVA, "JDK");
    return jdk.toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getGradleToolPath() const
{
    const auto &gradle = d->optionSettings.value(option::CATEGORY_GRADLE, "Gradle");
    int useWrapper = gradle.toMap().value("useWrapper").toInt();
    if (useWrapper) {
        return "./gradlew";
    }

    return gradle.toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getPythonToolPath() const
{
    const auto &python = d->optionSettings.value(option::CATEGORY_PYTHON, "Interpreter");
    return python.toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getNinjaToolPath() const
{
    const auto &ninja = d->optionSettings.value(option::CATEGORY_NINJA, "Ninja");
    return ninja.toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getJSToolPath() const
{
    const auto &js = d->optionSettings.value(option::CATEGORY_JS, "Interpreter");
    return js.toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getToolPath(const QString &kit) const
{
    if (kit == option::CATEGORY_NINJA) {
        return getNinjaToolPath();
    } else if (kit == option::CATEGORY_MAVEN) {
        return getMavenToolPath();
    } else if (kit == option::CATEGORY_GRADLE) {
        return getGradleToolPath();
    } else if (kit == option::CATEGORY_JS) {
        return getJSToolPath();
    } else if (kit == option::CATEGORY_PYTHON) {
        return getPythonToolPath();
    }
    return "";
}

QVariant OptionManager::getValue(const QString &category, const QString &key) const
{
    return d->optionSettings.value(category, key);
}

void OptionManager::setValue(const QString &category, const QString &key, const QVariant &value)
{
    d->optionSettings.setValue(category, key, value);
}
