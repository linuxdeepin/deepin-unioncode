/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef LIFECYCLE_H
#define LIFECYCLE_H

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

    static bool loadPlugin(PluginMetaObjectPointer &pointer);
    static void shutdownPlugin(PluginMetaObjectPointer &pointer);
};

DPF_END_NAMESPACE

#endif // LIFECYCLE_H
