// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginsetting.h"
#include "private/pluginmetaobject_p.h"

DPF_USE_NAMESPACE

PluginSetting::PluginSetting(QObject *parent)
    : PluginSetting(Scope::UserScope, ORGANIZATION, QCoreApplication::applicationName(), parent)
{
}

PluginSetting::PluginSetting(QSettings::Scope scope, const QString &organization,
                             const QString &application, QObject *parent)
    : QSettings(scope, organization, application, parent)
{
}

void PluginSetting::setPluginEnable(const PluginMetaObject &meta, bool enabled)
{
    beginGroup(meta.name());
    setValue(PLUGIN_VERSION, meta.version());
    setValue(ENABLED, enabled);
    endGroup();
}
