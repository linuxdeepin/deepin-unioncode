/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
class PluginManager: public QObject
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

    PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                             const QString version = "") const;

    bool loadPlugin(PluginMetaObjectPointer &pointer);
    bool initPlugin(PluginMetaObjectPointer &pointer);
    bool startPlugin(PluginMetaObjectPointer &pointer);
    void stopPlugin(PluginMetaObjectPointer &pointer);
};

DPF_END_NAMESPACE

#endif // PLUGINMANAGER_H
