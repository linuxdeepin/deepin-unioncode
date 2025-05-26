// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdata.h"

QVariant SettingData::value(const QString &group, const QString &key, const QVariant &dv) const
{
    return values.value(group).value(key, dv);
}

void SettingData::setValue(const QString &group, const QString &key, const QVariant &value)
{
    if (!values.contains(group)) {
        values.insert(group, { { key, value } });
        return;
    }

    values[group][key] = value;
}
