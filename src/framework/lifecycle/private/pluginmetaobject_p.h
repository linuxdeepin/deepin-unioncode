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
#ifndef PLUGINMETAOBJECT_P_H
#define PLUGINMETAOBJECT_P_H

#include "framework/framework_global.h"
#include "framework/lifecycle/pluginmetaobject.h"

#include <QString>
#include <QStringList>
#include <QSharedPointer>

DPF_BEGIN_NAMESPACE

/// @brief PLUGIN_NAME 插件名称Key
const char PLUGIN_NAME[] = "Name";
/// @brief PLUGIN_VERSION 插件版本Key
const char PLUGIN_VERSION[] = "Version";
/// @brief PLUGIN_VERSION 插件兼容版本Key
const char PLUGIN_COMPATVERSION[] = "CompatVersion";
/// @brief PLUGIN_VERSION 插件类型Key
const char PLUGIN_CATEGORY[] = "Category";
/// @nrief PLUGIN_VENDOR 插件作者
const char PLUGIN_VENDOR[] = "Vendor";
/// @nrief PLUGIN_VENDOR 插件所持有的公司
const char PLUGIN_COPYRIGHT[] = "Copyright";
/// @nrief PLUGIN_VENDOR 插件描述
const char PLUGIN_DESCRIPTION[] = "Description";
/// @nrief PLUGIN_VENDOR 插件开源协议
const char PLUGIN_LICENSE[] = "License";
/// @nrief PLUGIN_VENDOR 插件主页链接地址
const char PLUGIN_URLLINK[] = "UrlLink";
/// @nrief PLUGIN_VENDOR 插件依赖
const char PLUGIN_DEPENDS[] = "Depends";

class PluginMetaObject;
class PluginMetaObjectPrivate
{
    friend class PluginManager;
    friend class PluginManagerPrivate;
    friend class PluginMetaObject;
    PluginMetaObject const *q;
public:
    QString iid;
    QString name;
    QString version;
    QString compatVersion;
    QString vendor;
    QString copyright;
    QStringList license;
    QString description;
    QString urlLink;
    QString category;
    QString error;
    PluginMetaObject::State state;
    QList<PluginDepend> depends;
    QSharedPointer<Plugin> plugin;
    QSharedPointer<QPluginLoader> loader;
    QSharedPointer<PluginContext> context;
    bool enabledBySettings = true;
    bool disabledBySettings = false;

    explicit PluginMetaObjectPrivate(PluginMetaObject * q)
        : q(q)
        , loader(new QPluginLoader(nullptr))
    {

    }
};

DPF_END_NAMESPACE

#endif // PLUGINMETAOBJECT_P_H
