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
#include "optionutils.h"
#include "common/util/custompaths.h"

#include <QFile>
#include <QDir>
#include <QDebug>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>

OptionUtils::OptionUtils(QObject *parent)
    : QObject(parent)
{

}

OptionUtils::~OptionUtils()
{

}

QString OptionUtils::getJsonFilePath()
{
    return CustomPaths::user(CustomPaths::Flags::Configures) \
            + QDir::separator() \
            + QString("optionparam.support");
}

bool OptionUtils::readJsonRootObject(const QString &fileName, QJsonObject &rootObject)
{
    QFile file(fileName);
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

    rootObject = doc.object();
    return true;
}

bool OptionUtils::convertMapToJson(const QMap<QString, QVariant> &map, QJsonObject &jsonObject)
{
    foreach (QString key, map.keys()) {
        QVariant value = map.value(key);
        convert(key, value, jsonObject);
    }
    return true;
}

void OptionUtils::convert(const QString &key, const QVariant &variant, QJsonObject &jsonObject)
{
    switch (variant.type()) {
    case QVariant::String:
    {
        jsonObject.insert(key, variant.toString());
        break;
    }
    case QVariant::Map:
    {
        QJsonObject object;
        const QVariantMap varMap = variant.toMap();
        const QVariantMap::const_iterator cend = varMap.constEnd();
        for (QVariantMap::const_iterator iter = varMap.constBegin(); iter != cend; ++iter) {
            convert(iter.key(), iter.value(), object);
        }
        jsonObject.insert(key, object);
        break;
    }
    case QVariant::List:
    {
        QJsonObject object;
        foreach (const QVariant &var, variant.toList()) {
            Q_UNUSED(var)
            //TODO
        }
        jsonObject.insert(key, object);
        break;
    }
    default:
        break;
    }
}

bool OptionUtils::convertJsonToMap(const QJsonObject &jsonObject, QMap<QString, QVariant> &map)
{
    foreach (QString key, jsonObject.keys()) {
        QJsonValue value = jsonObject.value(key);
        convert(key, value, map);
    }
    return true;
}

void OptionUtils::convert(const QString &key, const QJsonValue &variant, QMap<QString, QVariant> &map)
{
    switch (variant.type()) {
    case QJsonValue::String:
    {
        map.insert(key, variant.toString());
        break;
    }
    case QJsonValue::Object:
    {
        QMap<QString, QVariant> varMap;
        const QJsonObject varObject = variant.toObject();
        foreach (QString varKey, varObject.keys()) {
            QJsonValue value = varObject.value(varKey);
            convert(varKey, value, varMap);
        }
        map.insert(key, varMap);
        break;
    }
    case QJsonValue::Array:
    {
        QMap<QString, QVariant> varMap;
        const QJsonArray array = variant.toArray();
        //TODO
        map.insert(key, varMap);
        break;
    }
    default:
        break;
    }
}

bool OptionUtils::writeJsonSection(const QString &fileName, const QString &parentNode, const QString &itemNode,
                            const QMap<QString, QVariant> &map)
{
    QJsonObject jsonObject;
    bool ret = convertMapToJson(map, jsonObject);
    if (!ret) {
        return false;
    }

    QJsonObject rootObject;
    QJsonObject parentObject;
    if (readJsonRootObject(fileName, rootObject)) {
        if (rootObject.find(parentNode)->toObject().isEmpty()) {
            rootObject.insert(parentNode, parentObject);
        } else {
            parentObject = rootObject.value(parentNode).toObject();
        }
    }

    if (!parentObject.find(itemNode)->toObject().isEmpty()) {
        parentObject.remove(itemNode);
    }

    parentObject.insert(itemNode, jsonObject);
    rootObject.insert(parentNode, parentObject);

    QJsonDocument doc;
    doc.setObject(rootObject);
    QString jsonStr(doc.toJson(QJsonDocument::Indented));

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(jsonStr.toUtf8());
    file.close();

    return true;
}


bool OptionUtils::readJsonSection(const QString &fileName, const QString &parentNode, const QString &itemNode,
                           QMap<QString, QVariant> &map)
{
    QJsonObject rootObject;
    if (!readJsonRootObject(fileName, rootObject))
        return false;

    QJsonObject parentObject = rootObject.value(parentNode).toObject();
    if (parentObject.isEmpty())
        return false;

    QJsonObject nodeObject = parentObject.value(itemNode).toObject();
    if (nodeObject.isEmpty())
        return false;

    bool ret = convertJsonToMap(nodeObject, map);
    if (!ret) {
        return false;
    }

    return true;
}

bool OptionUtils::readAll(const QString &fileName, QMap<QString, QVariant> &map)
{
    QJsonObject rootObject;
    if (!readJsonRootObject(fileName, rootObject))
        return false;

    bool ret = convertJsonToMap(rootObject, map);
    if (!ret) {
        return false;
    }

    return true;
}


bool OptionUtils::writeAll(const QString &fileName, const QMap<QString, QVariant> &map)
{
    QJsonObject rootObject;
    bool ret = convertMapToJson(map, rootObject);
    if (!ret) {
        return false;
    }

    QJsonDocument doc;
    doc.setObject(rootObject);
    QString jsonStr(doc.toJson(QJsonDocument::Indented));

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(jsonStr.toUtf8());
    file.close();

    return true;
}
