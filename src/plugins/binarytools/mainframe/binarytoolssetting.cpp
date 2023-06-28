// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
