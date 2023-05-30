/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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

#include "binarytoolssetting.h"

BinaryToolsSetting::BinaryToolsSetting()
{
    settings = new QSettings;
}

BinaryToolsSetting::BinaryToolsSetting(const QString &fileName, QObject *parent)
{
    settings = new QSettings(fileName, QSettings::IniFormat, parent);
}

void BinaryToolsSetting::setValue(const QString &key, const QVariant &value)
{
    settings->setValue(key, value);
}

QVariant BinaryToolsSetting::getValue(const QString &key, const QVariant &defaultValue)
{
    return  settings->value(key, defaultValue);
}

QVariant BinaryToolsSetting::getValue(const QString &key)
{
    return settings->value(key);
}

void BinaryToolsSetting::deleteKey(const QString &key)
{
    settings->remove(key);
}

void BinaryToolsSetting::replaceKey(const QString &oldKey, const QString &newKey)
{
    settings->setValue(newKey ,settings->value(oldKey));
    settings->remove(oldKey);
}

bool BinaryToolsSetting::hasKey(const QString &key)
{
    return settings->allKeys().contains(key);
}

void BinaryToolsSetting::allValues()
{
    QStringList keys = settings->allKeys();
    for (QString key : keys) {
        QVariant temp = settings->value(key);
    }
}
