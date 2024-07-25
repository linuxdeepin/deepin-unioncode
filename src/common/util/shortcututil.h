// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
