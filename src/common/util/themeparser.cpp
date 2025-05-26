// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "themeparser.h"

#include <QFile>
#include <QMetaEnum>
#include <QJsonDocument>
#include <QDebug>

ThemeParser::ThemeParser(QObject *parent)
    : QObject(parent)
{
}

bool ThemeParser::loadThemeFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists(fileName))
        return false;

    QJsonObject obj;
    if (file.open(QFile::ReadOnly)) {
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << error.errorString();
            return false;
        }

        fileObj = doc.object();
    }

    return true;
}

void ThemeParser::setTheme(ThemeParser::Theme theme)
{
    defaultTheme = theme;
}

QJsonValue ThemeParser::value(const QString &group, ThemeProperty property) const
{
    static auto themeMetaEnum = QMetaEnum::fromType<Theme>();
    auto themeObj = fileObj.value(themeMetaEnum.valueToKey(defaultTheme)).toObject();
    if (themeObj.isEmpty() || !themeObj.contains(group))
        return {};

    static auto propertyMetaEnum = QMetaEnum::fromType<ThemeProperty>();
    QString value = propertyMetaEnum.valueToKey(property);
    auto propertyObj = themeObj.value(group).toObject();
    if (propertyObj.isEmpty() || !propertyObj.contains(value))
        return {};

    return propertyObj.value(value);
}
