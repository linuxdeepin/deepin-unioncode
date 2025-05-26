// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORSETTINGS_H
#define EDITORSETTINGS_H

#include <QObject>
#include <QVariantMap>
#include <QTimer>

class EditorSettingsPrivate;
class EditorSettings : public QObject
{
    Q_OBJECT
public:
    static EditorSettings *instance();

    void setValue(const QString &node, const QString &group, const QString &key, const QVariant &value, bool notify = true);
    QVariant value(const QString &node, const QString &group, const QString &key, const QVariant &defaultValue = {});
    QMap<QString, QVariant> getMap(const QString &node);

signals:
    void valueChanged();

private:
    explicit EditorSettings(QObject *parent = nullptr);
    ~EditorSettings();

private:
    EditorSettingsPrivate *const d;
};

#endif   // EDITORSETTINGS_H
