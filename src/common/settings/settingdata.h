// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGDATA_H
#define SETTINGDATA_H

#include <QVariant>

class SettingData
{
    friend class SettingsPrivate;
    friend class Settings;

public:
    SettingData() = default;

    QVariant value(const QString &group, const QString &key, const QVariant &dv = QVariant()) const;
    void setValue(const QString &group, const QString &key, const QVariant &value);

private:
    // Set the file's configuration property hash table
    QHash<QString, QVariantHash> values;
};

#endif   // SETTINGDATA_H
