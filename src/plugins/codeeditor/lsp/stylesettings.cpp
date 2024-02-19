// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylesettings.h"

#include <QFile>
#include <QJsonObject>
#include <QDebug>

StyleSettings::StyleSettings(QObject *parent)
    : QObject(parent)
{
}

StyleSettings::StyleSettings(const StyleSettings &other)
{
    themeMap = other.themeMap;
}

StyleSettings::~StyleSettings()
{
}

StyleSettings &StyleSettings::operator=(const StyleSettings &other)
{
    themeMap = other.themeMap;
    return *this;
}

bool StyleSettings::load(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists(filePath))
        return false;

    if (file.open(QFile::ReadOnly)) {
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error != QJsonParseError::NoError)
            qCritical() << error.errorString();

        parseThemeInfo(doc.object());
    }

    return true;
}

QJsonValue StyleSettings::value(const QString &theme, const QString &key)
{
    if (!themeMap.contains(theme))
        return {};

    return themeMap[theme].value(key);
}

void StyleSettings::parseThemeInfo(const QJsonObject &obj)
{
    parseThemeInfo(Theme::get()->Light, obj);
    parseThemeInfo(Theme::get()->Dark, obj);
}

void StyleSettings::parseThemeInfo(const QString &theme, const QJsonObject &obj)
{
    if (!obj.contains(theme))
        return;

    auto themeObj = obj.value(theme).toObject();
    if (themeObj.isEmpty())
        return;

    themeMap[theme] = themeObj;
}
