// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef THEMEPARSER_H
#define THEMEPARSER_H

#include <QObject>
#include <QJsonObject>

class ThemeParser : public QObject
{
    Q_OBJECT
public:
    enum Theme {
        Light,
        Dark
    };
    Q_ENUM(Theme)

    enum ThemeProperty {
        BackgroundColor,
        ForegroundColor
    };
    Q_ENUM(ThemeProperty)

    explicit ThemeParser(QObject *parent = nullptr);

    bool loadThemeFile(const QString &fileName);
    void setTheme(Theme theme);
    QJsonValue value(const QString &group, ThemeProperty property) const;

private:
    QJsonObject fileObj;
    Theme defaultTheme { Light };
};

#endif   // THEMEPARSER_H
