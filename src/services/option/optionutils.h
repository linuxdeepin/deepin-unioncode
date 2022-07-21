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
#ifndef OPTIONUTILS_H
#define OPTIONUTILS_H

#include <QObject>

class OptionUtils : public QObject
{
    Q_OBJECT
public:
    static QString getJsonFilePath();

    static bool writeJsonSection(const QString &fileName, const QString &parentNode, const QString &itemNode,
                          const QMap<QString, QVariant> &map);
    static bool readJsonSection(const QString &fileName, const QString &parentNode, const QString &itemNode,
                         QMap<QString, QVariant> &map);

    static bool readAll(const QString &fileName, QMap<QString, QVariant> &map);
    static bool writeAll(const QString &fileName, const QMap<QString, QVariant> &map);
signals:

public slots:

private:
    explicit OptionUtils(QObject *parent = nullptr);
    virtual ~OptionUtils();

    static bool readJsonRootObject(const QString &fileName, QJsonObject &rootObject);

    static bool convertMapToJson(const QMap<QString, QVariant> &map, QJsonObject &jsonObject);
    static bool convertJsonToMap(const QJsonObject &jsonObject, QMap<QString, QVariant> &map);

    static void convert(const QString &key, const QVariant &variant, QJsonObject &jsonObject);
    static void convert(const QString &key, const QJsonValue &variant, QMap<QString, QVariant> &map);
};

#endif // OPTIONUTILS_H
