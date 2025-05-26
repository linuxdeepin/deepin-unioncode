// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLSETTINGS_H
#define BINARYTOOLSETTINGS_H

#include <QSettings>

class BinaryToolsSetting
{
public:
    BinaryToolsSetting();
    BinaryToolsSetting(const QString &fileName, QObject *parent = nullptr);

    void setValue(const QString &key, const QVariant &value);
    QVariant getValue(const QString &key, const QVariant &defaultValue);
    QVariant getValue(const QString &key);
    void deleteKey(const QString &key);
    void replaceKey(const QString &oldKey, const QString &newKey);
    bool hasKey(const QString &key);
    void allValues();

private:
    QSettings *settings = nullptr;
};

#endif // BINARYTOOLSETTINGS_H
