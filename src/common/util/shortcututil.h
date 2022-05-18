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

#ifndef SHORTCUTUTIL_H
#define SHORTCUTUTIL_H

#include <QObject>

class ShortcutUtil final: public QObject
{
    Q_OBJECT
public:
    static bool readFromJson(const QString &qsFilePath, QMap<QString, QStringList> &mapShortcutItem);
    static bool writeToJson(const QString &qsFilePath, const QMap<QString, QStringList> &mapShortcutItem);
signals:

private:
    ShortcutUtil(QObject *parent = nullptr);
    virtual ~ShortcutUtil();
};

#endif // SHORTCUTUTIL_H
