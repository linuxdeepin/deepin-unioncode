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

#ifndef BINARYTOOLSETTINGS_H
#define BINARYTOOLSETTINGS_H

#include <QSettings>

class BinaryToolsSetting
{
public:
    BinaryToolsSetting();

    void setValue(const QString &key, const QVariant &value);
    QVariant getValue(const QString &key, const QVariant &defaultValue);
    QVariant getValue(const QString &key);
    void deleteKey(const QString &key);
    void replaceKey(const QString &oldKey, const QString &newKey);
    bool hasKey(const QString &key);
    void allValues();

private:
    QSettings settings;
};

#endif // BINARYTOOLSETTINGS_H
