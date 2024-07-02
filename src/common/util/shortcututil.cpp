// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
