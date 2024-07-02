// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
