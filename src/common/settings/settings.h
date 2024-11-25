// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariant>

class SettingsPrivate;
class Settings : public QObject
{
    Q_OBJECT
public:
    Settings(QObject *parent = nullptr);
    explicit Settings(const QString &defaultFile, const QString &settingFile, QObject *parent = nullptr);
    ~Settings();

    QStringList groupList() const;
    QStringList keyList(const QString &group) const;
    bool contains(const QString &group, const QString &key) const;
    QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) const;
    QVariant defaultValue(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setValue(const QString &group, const QString &key, const QVariant &value, bool notify = false);
    void removeGroup(const QString &group);
    void remove(const QString &group, const QString &key);
    void load(const QString &defaultFile, const QString &settingFile);
    void clear();
    void reload();
    bool sync();
    bool isAutoSync();
    void setSyncExcludeGroup(const QString &group, bool sync = false);

public Q_SLOTS:
    void setAutoSync(bool autoSync);

Q_SIGNALS:
    void valueChanged(const QString &group, const QString &key, const QVariant &value);

private:
    SettingsPrivate *const d;
};

#endif   // SETTINGS_H
