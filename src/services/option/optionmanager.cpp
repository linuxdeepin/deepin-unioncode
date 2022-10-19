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

