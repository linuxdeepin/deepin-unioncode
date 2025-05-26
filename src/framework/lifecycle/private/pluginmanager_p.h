// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_P_H
#define PLUGINMANAGER_P_H

#include "framework/lifecycle/pluginsetting.h"
#include "framework/framework_global.h"

#include <QQueue>
#include <QStringList>
#include <QPluginLoader>
#include <QObject>
#include <QJsonArray>
#include <QSettings>
#include <QDirIterator>
#include <QDebug>
#include <QWriteLocker>
#include <QtConcurrent>

DPF_BEGIN_NAMESPACE

class PluginMetaObject;
class PluginManager;

class PluginManagerPrivate : public QSharedData
{
    Q_DISABLE_COPY(PluginManagerPrivate)
    friend class PluginManager;
    PluginManager *const q;
    QString pluginLoadIID;
    QStringList pluginLoadPaths;
    QStringList serviceLoadPaths;
    QStringList disabledPlugins;
    QStringList enabledPlugins;
    QHash<QString, QQueue<PluginMetaObjectPointer>> pluginCategories;
    QList<PluginMetaObjectPointer> plugins;
    QQueue<PluginMetaObjectPointer> readQueue;
    QQueue<PluginMetaObjectPointer> loadQueue;
    PluginSetting *setting = nullptr;
public:
    typedef QQueue<PluginMetaObjectPointer> PluginMetaQueue;
    explicit PluginManagerPrivate(PluginManager *qq);
    virtual ~PluginManagerPrivate();
    QString pluginIID() const;
    void setPluginIID(const QString &pluginIID);
    QStringList pluginPaths() const;
    void setPluginPaths(const QStringList &pluginPaths);
    QStringList servicePaths() const;
    void setServicePaths(const QStringList &servicePaths);
    void setPluginEnable(const PluginMetaObject& meta, bool enabled);
    PluginMetaObjectPointer pluginMetaObj(const QString &name,const QString &version = "");
    bool loadPlugin(PluginMetaObjectPointer &pluginMetaObj);
    bool initPlugin(PluginMetaObjectPointer &pluginMetaObj);
    bool startPlugin(PluginMetaObjectPointer &pluginMetaObj);
    void stopPlugin(PluginMetaObjectPointer &pluginMetaObj);
    bool readPlugins();
    bool loadPlugins();
    void initPlugins();
    void startPlugins();
    void stopPlugins();
    void setSettings(PluginSetting *s);
    void readSettings();
    void writeSettings();
    static void scanfAllPlugin(PluginMetaQueue &destQueue,
                               const QStringList& pluginPaths,
                               const QString &pluginIID);
    static void readJsonToMeta(const PluginMetaObjectPointer &metaObject);
    QHash<QString, QQueue<PluginMetaObjectPointer>> categories();
    static PluginMetaQueue dependsSort(const PluginMetaQueue &srcQueue);
};

DPF_END_NAMESPACE

#endif // PLUGINMANAGER_P_H
