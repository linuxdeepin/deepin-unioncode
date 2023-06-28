// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LIFECYCLE_H
#define LIFECYCLE_H

#include "pluginmanager.h"
#include "framework/lifecycle/pluginmetaobject.h"
#include "framework/lifecycle/plugin.h"
#include "framework/framework_global.h"

#include <QString>
#include <QObject>

DPF_BEGIN_NAMESPACE

class LifeCycle final : public QObject
{
    Q_OBJECT
public:
    static PluginManager* getPluginManagerInstance();
    static void setPluginIID(const QString &pluginIID);
    static QString pluginIID();
    static QStringList pluginPaths();
    static void setPluginPaths(const QStringList &pluginPaths);
    static QStringList servicePaths();
    static void setServicePaths(const QStringList &servicePaths);
    static PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                                 const QString version = "");
    static bool readPlugins();
    static bool loadPlugins();
    static void shutdownPlugins();
    static void setSettings(PluginSetting *settings);

    static bool loadPlugin(PluginMetaObjectPointer &pointer);
    static void shutdownPlugin(PluginMetaObjectPointer &pointer);
};

DPF_END_NAMESPACE

#endif // LIFECYCLE_H
