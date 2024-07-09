// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "framework/lifecycle/plugin.h"
#include "framework/lifecycle/pluginsetting.h"
#include "framework/framework_global.h"

#include <QPluginLoader>
#include <QSettings>
#include <QObject>
#include <QQueue>
#include <QSharedData>
#include <QDirIterator>
#include <QScopedPointer>

DPF_BEGIN_NAMESPACE

class PluginManagerPrivate;

/*
 * @class PluginManager 插件管理器
 * @details 提供插件加载与卸载
 * 其中重要的特性为：plugin IID (插件身份标识) 可参阅Qt插件规范；
 * 此处目前只支持Plugin接口插件的动态库形式插件加载
 */
class DPF_EXPORT PluginManager: public QObject
{
    Q_OBJECT
    friend class PluginManagerPrivate;
    QSharedPointer<PluginManagerPrivate> d;

public:
    explicit PluginManager();
    QString pluginIID() const;
    QStringList pluginPaths() const;
    QStringList servicePaths() const;
    void setPluginIID(const QString &pluginIID);
    void setPluginPaths(const QStringList &pluginPaths);
    void setServicePaths(const QStringList &servicePaths);

    bool readPlugins();
    bool loadPlugins();
    void initPlugins();
    void startPlugins();
    void stopPlugins();
    void setSettings(PluginSetting *settings);
    void writeSettings();
    const QHash<QString, QQueue<PluginMetaObjectPointer>> &pluginCollections() const;

    PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                             const QString version = "") const;

    bool loadPlugin(PluginMetaObjectPointer &pointer);
    bool initPlugin(PluginMetaObjectPointer &pointer);
    bool startPlugin(PluginMetaObjectPointer &pointer);
    void stopPlugin(PluginMetaObjectPointer &pointer);

signals:
    void pluginsChanged();
};

DPF_END_NAMESPACE

#endif // PLUGINMANAGER_H
