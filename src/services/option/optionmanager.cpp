// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "optionmanager.h"
#include "optionutils.h"
#include "optiondatastruct.h"

#include <QVariant>

static OptionManager *m_instance = nullptr;

class OptionManagerPrivate final
{
    friend class OptionManager;

    QMap<QString, QVariant> dataMap;
};

OptionManager::OptionManager(QObject *parent)
    : QObject(parent)
    , d(new OptionManagerPrivate())
{
    updateData();
}

OptionManager::~OptionManager()
{
    if (d) {
        delete d;
    }
}

OptionManager *OptionManager::getInstance()
{
    if (!m_instance) {
        m_instance = new OptionManager();
    }
    return m_instance;
}

void OptionManager::updateData()
{
    OptionUtils::readAll(OptionUtils::getJsonFilePath(), d->dataMap);
}

QString OptionManager::getMavenToolPath()
{
    QMap<QString, QVariant> map = d->dataMap.value(option::CATEGORY_MAVEN).toMap();
    return map.value("Maven").toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getJdkToolPath()
{
    QMap<QString, QVariant> map = d->dataMap.value(option::CATEGORY_JAVA).toMap();
    return map.value("JDK").toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getGradleToolPath()
{
    QMap<QString, QVariant> map = d->dataMap.value(option::CATEGORY_GRADLE).toMap();
    int useWrapper = map.value("Gradle").toMap().value("useWrapper").toInt();
    if (useWrapper) {
        return "./gradlew";
    }

    return map.value("Gradle").toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getPythonToolPath()
{
    QMap<QString, QVariant> map = d->dataMap.value(option::CATEGORY_PYTHON).toMap();
    return map.value("Interpreter").toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getCMakeToolPath()
{
    QMap<QString, QVariant> map = d->dataMap.value(option::CATEGORY_CMAKE).toMap();
    return map.value("Kits").toMap().value("cmake").toMap().value("path").toString();
}

QString OptionManager::getNinjaToolPath()
{
    QMap<QString, QVariant> map = d->dataMap.value(option::CATEGORY_NINJA).toMap();
    return map.value("Ninja").toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getCxxDebuggerToolPath()
{
    QMap<QString, QVariant> map = d->dataMap.value(option::CATEGORY_CMAKE).toMap();
    return map.value("Kits").toMap().value("debugger").toMap().value("path").toString();
}

QString OptionManager::getJSToolPath()
{
    QMap<QString, QVariant> map = d->dataMap.value(option::CATEGORY_JS).toMap();
    return map.value("Interpreter").toMap().value("version").toMap().value("path").toString();
}

QString OptionManager::getToolPath(const QString &kit)
{
    if (kit == option::CATEGORY_CMAKE) {
        return getCMakeToolPath();
    } else if (kit == option::CATEGORY_NINJA) {
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

QVariant OptionManager::getValue(const QString &category, const QStringList &properties)
{
    QVariant tmp = d->dataMap.value(category).toMap();
    for (auto property : properties) {
        tmp = tmp.toMap().value(property);
    }
    return tmp;
}

