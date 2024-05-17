// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorsettings.h"
#include "settingsdefine.h"

#include "services/option/optionutils.h"

#include <QMap>

class EditorSettingsPrivate
{
public:
    struct Settings
    {
        QString group;
        QVariantMap data;
    };

    explicit EditorSettingsPrivate(EditorSettings *qq);
    ~EditorSettingsPrivate();

    void loadConfig();
    void loadConfig(const QString &node, const QString &group, const QVariantMap &map);
    void saveConfig();

    EditorSettings *q;

    QMap<QString, QList<Settings>> settingDatas;
    QTimer changeTimer;
    bool isLoad { false };
    QStringList nodeList;
};

EditorSettingsPrivate::EditorSettingsPrivate(EditorSettings *qq)
    : q(qq)
{
    nodeList << Node::FontColor;
    nodeList << Node::Behavior;
    nodeList << Node::MimeTypeConfig;
}

EditorSettingsPrivate::~EditorSettingsPrivate()
{
    saveConfig();
}

void EditorSettingsPrivate::loadConfig()
{
    isLoad = true;

    for (const auto &node : nodeList) {
        QMap<QString, QVariant> map;
        OptionUtils::readJsonSection(OptionUtils::getJsonFilePath(),
                                     EditorConfig, node, map);

        auto iter = map.begin();
        for (; iter != map.end(); ++iter)
            loadConfig(node, iter.key(), iter.value().toMap());
    }

    isLoad = false;
}

void EditorSettingsPrivate::loadConfig(const QString &node, const QString &group, const QVariantMap &map)
{
    if (map.isEmpty())
        return;

    auto iter = map.begin();
    for (; iter != map.end(); ++iter)
        q->setValue(node, group, iter.key(), iter.value());
}

void EditorSettingsPrivate::saveConfig()
{
    auto iter = settingDatas.begin();
    for (; iter != settingDatas.end(); ++iter) {
        QMap<QString, QVariant> map;
        const auto &settingList = settingDatas.value(iter.key());
        for (const auto &settings : settingList) {
            map.insert(settings.group, settings.data);
        }
        OptionUtils::writeJsonSection(OptionUtils::getJsonFilePath(),
                                          EditorConfig, iter.key(), map);
    }
}

EditorSettings::EditorSettings(QObject *parent)
    : QObject(parent),
      d(new EditorSettingsPrivate(this))
{
    d->changeTimer.setInterval(200);
    d->changeTimer.setSingleShot(true);
    connect(&d->changeTimer, &QTimer::timeout, this, &EditorSettings::valueChanged);

    d->loadConfig();
}

EditorSettings::~EditorSettings()
{
    delete d;
}

EditorSettings *EditorSettings::instance()
{
    static EditorSettings ins;
    return &ins;
}

void EditorSettings::setValue(const QString &node, const QString &group, const QString &key, const QVariant &value, bool notify)
{
    EditorSettingsPrivate::Settings st;
    st.group = group;
    st.data.insert(key, value);

    if (d->settingDatas.contains(node)) {
        auto stList = d->settingDatas.value(node);
        int index = -1;
        auto iter = std::find_if(stList.begin(), stList.end(),
                                 [&group, &index](const EditorSettingsPrivate::Settings &st) {
                                     ++index;
                                     return st.group == group;
                                 });
        if (iter == stList.end()) {
            stList.append(st);
            d->settingDatas.insert(node, stList);
        } else {
            iter->data[key] = value;
            stList.replace(index, *iter);
            d->settingDatas.remove(node);
            d->settingDatas.insert(node, stList);
        }
    } else {
        d->settingDatas.insert(node, { st });
    }

    if (!d->isLoad && notify)
        d->changeTimer.start();
}

QVariant EditorSettings::value(const QString &node, const QString &group, const QString &key, const QVariant &defaultValue)
{
    if (!d->settingDatas.contains(node))
        return defaultValue;

    const auto &stList = d->settingDatas.value(node);
    auto iter = std::find_if(stList.begin(), stList.end(),
                             [&group](const EditorSettingsPrivate::Settings &st) {
                                 return st.group == group;
                             });
    if (iter == stList.end())
        return defaultValue;

    return iter->data.value(key, defaultValue);
}

QMap<QString, QVariant> EditorSettings::getMap(const QString &node) 
{
    const auto &stList = d->settingDatas.value(node);
    QMap<QString, QVariant> map;
    for (const auto &st : stList)
        map.insert(st.group, st.data);
    return map;
}

