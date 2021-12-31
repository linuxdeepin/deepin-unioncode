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
#include "pluginmetaobject.h"
#include "private/pluginmetaobject_p.h"

#include <QDebug>

DPF_BEGIN_NAMESPACE

/**
 * @brief PluginMetaObject::version
 * 获取插件元数据中插件版本
 * @return
 */
QString PluginMetaObject::version() const
{
    return d->version;
}

/**
 * @brief PluginMetaObject::compatVersion
 * 获取插件元数据中兼容版本
 * @return
 */
QString PluginMetaObject::compatVersion() const
{
    return d->compatVersion;
}

/**
 * @brief PluginMetaObject::vendor
 * 获取插件元数据中插件所有者
 * @return
 */
QString PluginMetaObject::vendor() const
{
    return d->vendor;
}

/**
 * @brief PluginMetaObject::copyright
 * 获取插件元数据中的插件版权
 * @return
 */
QString PluginMetaObject::copyright() const
{
    return d->copyright;
}

/**
 * @brief PluginMetaObject::license
 * 获取插件元数据中开源许可协议
 * @return
 */
QStringList PluginMetaObject::license() const
{
    return d->license;
}

/**
 * @brief PluginMetaObject::description
 * 获取插件元数据中插件描述
 * @return
 */
QString PluginMetaObject::description() const
{
    return d->description;
}

/**
 * @brief PluginMetaObject::category
 * 获取插件元数据中插件类别
 * @return
 */
QString PluginMetaObject::category() const
{
    return d->category;
}

/**
 * @brief PluginMetaObject::urlLink
 * 获取插件元数据中插件主页链接
 * @return
 */
QString PluginMetaObject::urlLink() const
{
    return d->urlLink;
}

/**
 * @brief PluginMetaObject::depends
 * 获取插件元数据中插件启动依赖
 * @return
 */
QList<PluginDepend> PluginMetaObject::depends() const
{
    return d->depends;
}

/**
 * @brief PluginMetaObject::pluginState
 * 获取插件当前状态
 * @return
 */
PluginMetaObject::State PluginMetaObject::pluginState() const
{
    return d->state;
}

/**
 * @brief PluginMetaObject::plugin
 * 获取插件对象接口
 * @return
 */
QSharedPointer<Plugin> PluginMetaObject::plugin()
{
    return d->plugin;
}

/**
 * @brief PluginMetaObject::errorString
 * 获取插件加载错误信息
 * @return
 */
QString PluginMetaObject::errorString()
{
    return d->error;
}

/**
 * @brief 默认构造函数
 */
PluginMetaObject::PluginMetaObject()
    : d(new PluginMetaObjectPrivate(this))
{

}

/**
 * @brief 拷贝构造函数
 */
PluginMetaObject::PluginMetaObject(const PluginMetaObject &meta)
{
    d->iid = meta.iid();
    d->name = meta.name();
    d->version = meta.version();
    d->compatVersion = meta.compatVersion();
    d->category = meta.category();
    d->license = meta.license();
    d->description = meta.description();
    d->urlLink = meta.urlLink();
    d->depends = meta.depends();
    d->state = pluginState();
    d->plugin = plugin();
    d->loader = meta.d->loader;
}
/**
  * @brief 赋值拷贝
  */
PluginMetaObject &PluginMetaObject::operator =(const PluginMetaObject &meta)
{
    d->iid = meta.iid();
    d->name = meta.name();
    d->version = meta.version();
    d->compatVersion = meta.compatVersion();
    d->category = meta.category();
    d->license = meta.license();
    d->description = meta.description();
    d->urlLink = meta.urlLink();
    d->depends = meta.depends();
    d->state = pluginState();
    d->plugin = plugin();
    d->loader = meta.d->loader;
    return *this;
}

/**
 * @brief PluginMetaObject::fileName
 *  获取插件文件路径名称
 * @return
 */
QString PluginMetaObject::fileName() const
{
    return d->loader->fileName();
}

/**
 * @brief PluginMetaObject::iid
 *  获取插件元数据中iid
 * @return
 */
QString PluginMetaObject::iid() const
{
    return d->iid;
}

/**
 * @brief PluginMetaObject::name
 *  获取插件元数据中名称
 * @return
 */
QString PluginMetaObject::name() const
{
    return d->name;
}

/**
 * @brief PluginDepend::PluginDepend
 *  构造函数
 */
PluginDepend::PluginDepend()
{

}

/**
 * @brief PluginDepend::PluginDepend
 * 拷贝构造
 * @param depend
 */
PluginDepend::PluginDepend(const PluginDepend &depend)
{
    pluginName = depend.name();
    pluginVersion = depend.version();
}

/**
 * @brief PluginDepend::operator =
 * 赋值拷贝
 * @param depend
 * @return
 */
PluginDepend &PluginDepend::operator =(const PluginDepend &depend)
{
    pluginName = depend.name();
    pluginVersion = depend.version();
    return *this;
}


QT_BEGIN_NAMESPACE
/**
 * @brief operator <<
 * 重定向全局Debug打印PluginDepend对象的函数
 * @param out
 * @param depend
 * @return
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginDepend &depend)
{
    DPF_USE_NAMESPACE
    out << "PluginDepend(" <<  QString("0x%0").arg(qint64(&depend),0,16) << "){";
    out << PLUGIN_NAME << " : " << depend.name() << "; ";
    out << PLUGIN_VERSION << " : " << depend.version() << "; ";
    out << "}";
    return out;
}

/**
 * @brief operator <<
 * 重定向全局Debug打印PluginMetaObject对象的函数
 * @param out
 * @param metaObj
 * @return
 */
Q_CORE_EXPORT QDebug operator<< (QDebug out, const DPF_NAMESPACE::PluginMetaObject &metaObj)
{
    DPF_USE_NAMESPACE
    out << "PluginMetaObject(" << QString("0x%0").arg(qint64(&metaObj),0,16) << "){";
    out << "IID" << ":" << metaObj.iid() << "; ";
    out << PLUGIN_NAME << ":" << metaObj.name() << "; ";
    out << PLUGIN_VERSION << ":" << metaObj.version() << "; ";
    out << PLUGIN_COMPATVERSION << ":" << metaObj.compatVersion() << "; ";
    out << PLUGIN_CATEGORY << ":" << metaObj.category() << "; ";
    out << PLUGIN_VENDOR << ":" << metaObj.vendor() << "; ";
    out << PLUGIN_COPYRIGHT << ":" << metaObj.copyright() << "; ";
    out << PLUGIN_DESCRIPTION << ":" << metaObj.description() << "; ";
    out << PLUGIN_LICENSE << ":" << metaObj.license() << "; ";
    out << PLUGIN_URLLINK << ":" << metaObj.urlLink() << "; ";
    out << PLUGIN_DEPENDS << ":" << metaObj.depends() << ";";
    out << "}";
    return out;
}

/**
 * @brief operator <<
 * 重定向全局Debug入口函数
 * @param out
 * @param pointer
 * @return
 */
Q_CORE_EXPORT QDebug operator<<(QDebug out, const DPF_NAMESPACE::PluginMetaObjectPointer &pointer)
{
    out << *pointer;
    return out;
}

QT_END_NAMESPACE

DPF_END_NAMESPACE
