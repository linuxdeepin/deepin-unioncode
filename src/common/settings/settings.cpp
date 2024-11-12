// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"
#include "settingdata.h"

#include <QDir>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>

class SettingsPrivate
{
public:
    void fromJsonFile(const QString &fileName, SettingData *data);
    void fromJson(const QByteArray &json, SettingData *data);
    QByteArray toJson(const SettingData &data);
    void enableSync(bool enable);
    bool makeFilePath(const QString &fileName);
    bool isRemovable(const QString &group, const QString &key) const;

public:
    bool autoSync { false };
    bool syncEnabled { false };   // true if the file has cached data
    QTimer *syncTimer { nullptr };
    QString settingFile;
    SettingData settingData;
    SettingData defaultData;
    QSet<QString> syncGroupExclude;   // when auto sync, exclude some group
};

void SettingsPrivate::fromJsonFile(const QString &fileName, SettingData *data)
{
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.exists() && !makeFilePath(fileName)) {
        qWarning() << "The setting file does not exist and failed to be created : " << fileName;
        return;
    }

    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open the setting file. Error: " << file.errorString();
        return;
    }

    const QByteArray &json = file.readAll();
    if (json.isEmpty()) {
        qWarning() << "The setting file is empty.";
        return;
    }

    fromJson(json, data);
}

void SettingsPrivate::fromJson(const QByteArray &json, SettingData *data)
{
    QJsonParseError error;
    const QJsonDocument &doc = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse the setting file: " << error.errorString();
        return;
    }

    const QJsonObject &groupList = doc.object();
    for (auto grpIter = groupList.constBegin(); grpIter != groupList.constEnd(); ++grpIter) {
        const QJsonValue &value = grpIter.value();
        if (!value.isObject())
            continue;

        const QJsonObject &groupObj = value.toObject();
        QVariantHash hash = groupObj.toVariantHash();
        data->values[grpIter.key()] = hash;
    }
}

QByteArray SettingsPrivate::toJson(const SettingData &data)
{
    QJsonObject obj;
    for (auto iter = data.values.constBegin(); iter != data.values.constEnd(); ++iter) {
        const QString &key = iter.key();
        if (!syncGroupExclude.contains(key))
            obj.insert(key, QJsonValue(QJsonObject::fromVariantHash(iter.value())));
    }

    return QJsonDocument(obj).toJson();
}

void SettingsPrivate::enableSync(bool enable)
{
    if (syncEnabled == enable)
        return;

    syncEnabled = enable;
    if (!autoSync || !syncTimer)
        return;

    if (QThread::currentThread() == syncTimer->thread()) {
        if (enable) {
            syncTimer->start();
        } else {
            syncTimer->stop();
        }
    } else {
        syncTimer->metaObject()->invokeMethod(syncTimer, enable ? "start" : "stop", Qt::QueuedConnection);
    }
}

bool SettingsPrivate::makeFilePath(const QString &fileName)
{
    QFileInfo info(fileName);
    if (!QFile::exists(info.absolutePath())) {
        bool ok = QDir().mkpath(info.absolutePath());
        if (!ok)
            return false;
    }

    QFile file(fileName);
    return file.open(QIODevice::WriteOnly);
}

bool SettingsPrivate::isRemovable(const QString &group, const QString &key) const
{
    return settingData.values.value(group).contains(key);
}

Settings::Settings(QObject *parent)
    : QObject(parent),
      d(new SettingsPrivate)
{
}

Settings::Settings(const QString &defaultFile, const QString &settingFile, QObject *parent)
    : Settings(parent)
{
    d->settingFile = settingFile;
    d->fromJsonFile(settingFile, &d->settingData);
    d->fromJsonFile(defaultFile, &d->defaultData);
}

Settings::~Settings()
{
    if (d->syncTimer)
        d->syncTimer->stop();

    if (d->syncEnabled)
        sync();

    delete d;
}

QStringList Settings::groupList() const
{
    QSet<QString> groupList;
    groupList += QSet<QString>::fromList(d->settingData.values.keys());
    groupList += QSet<QString>::fromList(d->defaultData.values.keys());

    return groupList.toList();
}

QStringList Settings::keyList(const QString &group) const
{
    QSet<QString> keyList;
    const auto &&kg = d->settingData.values.value(group);
    const auto &&dkg = d->defaultData.values.value(group);

    keyList += QSet<QString>::fromList(kg.keys());
    keyList += QSet<QString>::fromList(dkg.keys());

    return keyList.toList();
}

bool Settings::contains(const QString &group, const QString &key) const
{
    if (key.isEmpty()) {
        if (d->settingData.values.contains(group))
            return true;

        return d->defaultData.values.contains(group);
    }

    if (d->settingData.values.value(group).contains(key))
        return true;

    return d->defaultData.values.value(group).contains(key);
}

QVariant Settings::value(const QString &group, const QString &key, const QVariant &defaultValue)
{
    auto value = d->settingData.value(group, key, QVariant::Invalid);
    if (value.isValid())
        return value;

    return d->defaultData.value(group, key, defaultValue);
}

QVariant Settings::defaultValue(const QString &group, const QString &key, const QVariant &defaultValue)
{
    return d->defaultData.value(group, key, defaultValue);
}

void Settings::setValue(const QString &group, const QString &key, const QVariant &value, bool notify)
{
    bool changed = false;
    if (d->isRemovable(group, key)) {
        changed = d->settingData.value(group, key) != value;
    } else {
        changed = this->value(group, key, value) != value;
    }

    if (!changed)
        return;

    d->settingData.setValue(group, key, value);
    d->enableSync(true);

    if (notify)
        Q_EMIT valueChanged(group, key, value);
}

void Settings::removeGroup(const QString &group)
{
    if (!d->settingData.values.contains(group))
        return;

    const auto &groupValue = d->settingData.values.take(group);
    d->enableSync(true);

    for (auto iter = groupValue.cbegin(); iter != groupValue.cend(); ++iter) {
        const auto &value = this->value(group, iter.key());
        if (value != iter.value())
            Q_EMIT valueChanged(group, iter.key(), value);
    }
}

void Settings::remove(const QString &group, const QString &key)
{
    if (!d->settingData.values.value(group).contains(key))
        return;

    const auto &oldValue = d->settingData.values[group].take(key);
    d->enableSync(true);

    const auto &newValue = value(group, key);
    if (oldValue != newValue)
        Q_EMIT valueChanged(group, key, newValue);
}

void Settings::load(const QString &defaultFile, const QString &settingFile)
{
    d->defaultData.values.clear();
    d->settingData.values.clear();

    d->settingFile = settingFile;
    d->fromJsonFile(defaultFile, &d->defaultData);
    d->fromJsonFile(settingFile, &d->settingData);
}

void Settings::clear()
{
    if (d->settingData.values.isEmpty())
        return;

    const auto cacheValues = d->settingData.values;
    d->settingData.values.clear();
    d->enableSync(true);

    auto iter = cacheValues.cbegin();
    for (; iter != cacheValues.cend(); ++iter) {
        const auto &values = iter.value();
        for (auto iv = values.cbegin(); iv != values.cend(); ++iv) {
            const auto &newValue = value(iter.key(), iv.key());
            if (newValue != iv.value())
                Q_EMIT valueChanged(iter.key(), iv.key(), newValue);
        }
    }
}

void Settings::reload()
{
    d->settingData.values.clear();
    d->fromJsonFile(d->settingFile, &d->settingData);
}

bool Settings::sync()
{
    if (!d->syncEnabled)
        return true;

    const QByteArray &json = d->toJson(d->settingData);
    QFile file(d->settingFile);
    if (!file.open(QFile::WriteOnly))
        return false;

    bool ok = file.write(json) == json.size();
    if (ok)
        d->enableSync(false);
    file.close();
    return ok;
}

bool Settings::isAutoSync()
{
    return d->autoSync;
}

void Settings::setSyncExcludeGroup(const QString &group, bool sync)
{
    if (!sync)
        d->syncGroupExclude.insert(group);
    else
        d->syncGroupExclude.remove(group);
}

void Settings::setAutoSync(bool autoSync)
{
    if (d->autoSync == autoSync)
        return;

    d->autoSync = autoSync;
    if (autoSync) {
        if (d->syncEnabled)
            sync();

        if (!d->syncTimer) {
            d->syncTimer = new QTimer(this);
            d->syncTimer->moveToThread(thread());
            d->syncTimer->setSingleShot(true);
            d->syncTimer->setInterval(1000);

            connect(d->syncTimer, &QTimer::timeout, this, &Settings::sync);
        }
    } else {
        if (d->syncTimer) {
            d->syncTimer->stop();
            d->syncTimer->deleteLater();
            d->syncTimer = nullptr;
        }
    }
}
