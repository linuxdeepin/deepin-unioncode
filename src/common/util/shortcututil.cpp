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

#include "shortcututil.h"

#include <QFile>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

ShortcutUtil::ShortcutUtil(QObject *parent)
    : QObject(parent)
{

}

ShortcutUtil::~ShortcutUtil()
{

}

bool ShortcutUtil::readFromJson(const QString &qsFilePath, QMap<QString, QStringList> &mapShortcutItem)
{
    QFile file(qsFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (QJsonParseError::NoError != parseError.error) {
        return false;
    }

    if (!doc.isObject())
        return false;

    QJsonObject rootObject = doc.object();
    mapShortcutItem.clear();
    for (auto key : rootObject.keys()) {
        if (!rootObject.value(key).isArray())
            continue;
        QJsonArray valueArray = rootObject.value(key).toArray();
        if (valueArray.count() < 2)
            continue;

        QStringList qsListValue = {valueArray.first().toString(), valueArray.last().toString()};
        mapShortcutItem[key] = qsListValue;
    }

    return true;
}

bool ShortcutUtil::writeToJson(const QString &qsFilePath, const QMap<QString, QStringList> &mapShortcutItem)
{
    QJsonObject rootObject;
    QMap<QString, QStringList>::const_iterator iter = mapShortcutItem.begin();
    for (; iter != mapShortcutItem.end(); ++iter)
    {
        QString qsID = iter.key();
        QString qsDesc = iter.value().first();
        QString qsShortcut = iter.value().last();

        QJsonArray valueArray;
        valueArray.append(QJsonValue(qsDesc));
        valueArray.append(QJsonValue(qsShortcut));

        rootObject.insert(qsID, valueArray);
    }

    QJsonDocument doc;
    doc.setObject(rootObject);
    QString jsonStr(doc.toJson(QJsonDocument::Indented));

    QFile file(qsFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(jsonStr.toUtf8());
    file.close();

    return true;
}
