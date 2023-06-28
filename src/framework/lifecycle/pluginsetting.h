// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINSSETTING_H
#define PLUGINSSETTING_H

#include "framework/lifecycle/pluginmetaobject.h"
#include "framework/framework_global.h"

#include <QCoreApplication>
#include <QSettings>

DPF_BEGIN_NAMESPACE

const char ORGANIZATION[] = "deepin";
const char ENABLED[] = "enabled";
const char DISABLED_PLUGINS[] = "Plugins/Disabled";
const char ENABLED_PLUGINS[] = "Plugins/Enabled";

class PluginSetting final : public QSettings
{
    Q_OBJECT
public:
    PluginSetting(Scope scope = Scope::UserScope,
                      const QString &organization = ORGANIZATION,
                      const QString &application = QCoreApplication::applicationName(),
                      QObject *parent = nullptr);
    void setPluginEnable(const PluginMetaObject &meta, bool enabled);
Q_SIGNALS:
    void pluginEnabled(const PluginMetaObject &meta, bool enabled);
};

DPF_END_NAMESPACE

#endif // PLUGINSSETTING_H
