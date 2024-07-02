// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMETAOBJECT_H
#define PLUGINMETAOBJECT_H

#include "framework/framework_global.h"
#include "framework/lifecycle/plugindepend.h"

#include <QSharedData>
#include <QPluginLoader>

DPF_BEGIN_NAMESPACE

/**
 * @brief PluginMetaT1 模板类
 * @details 此模板类为扩展特性，可实现不同插件元数据
 *  目前只是预留接口
 * @tparam 传入插件对象接口例如 Plugin
 */
template <class T>
class PluginMetaT1 : public QSharedData
{
    Q_DISABLE_COPY(PluginMetaT1)
public:
    PluginMetaT1(){}
};

/** 插件json元文件示例
 * @code
 * {
 *   "Name" : "more",
 *   "Version" : "4.8.2",
 *   "CompatVersion" : "4.8.0",
 *   "Category" : "more",
 *   "Description" : "The core plugin for the Qt IDE.",
 *   "Vendor" : "The Qt Company Ltd",
 *   "Copyright" : "(C) 2019 The Qt Company Ltd",
 *   "License" : [
 *       "https://www.gnu.org/licenses/gpl-3.0.html"
 *   ],
 *   "UrlLink" : "http://www.qt.io",
 *   "Depends" : [
 *       {"Name" : "core", "Version" : "4.8.2"},
 *       {"Name" : "other", "Version" : "4.8.2"}
 *   ]
 * }
 * @endcode
 */

class Plugin;
class PluginContext;
class PluginService;

/**
 * @brief The PluginMetaObject class
 *  插件元数据对象
 * @details 该类与SharedPointer配套使用时是线程安全的
 */
class PluginMetaObjectPrivate;
class PluginMetaObject final : public PluginMetaT1<Plugin>
{
    QSharedPointer<PluginMetaObjectPrivate> d;

    friend class PluginManager;
    friend class PluginManagerPrivate;
    friend Q_CORE_EXPORT QDebug operator<< (QDebug, const PluginMetaObject &);

public:

    enum State {
        Invalid,     /// 插件未操作获得任何状态
        Reading,     /// 插件正在读取Json
        Readed,      /// 插件读取Json完毕
        Loading,     /// 插件正在加载
        Loaded,      /// 插件已加载
        Initialized, /// 插件已经操作Initialized函数
        Started,     /// 插件已经操作Start函数
        Stoped,      /// 插件已停操作Stop函数
        Shutdown,    /// 插件卸载并已经释放
    };

    PluginMetaObject();
    PluginMetaObject(const PluginMetaObject &meta);
    PluginMetaObject& operator = (const PluginMetaObject &meta);
    QString fileName() const;
    QString iid() const;
    QString name() const;
    QString version() const;
    QString compatVersion() const;
    QString category() const;
    QString vendor() const;
    QString copyright() const;
    QStringList license() const;
    QString description() const;
    QString urlLink() const;
    QList<PluginDepend> depends() const;
    QList<PluginInstallDepend> installDepends() const;
    State pluginState() const;
    QSharedPointer<Plugin> plugin();
    bool isEnabledBySettings();
    bool isDisabledBySettings();
    void setEnabledBySettings(bool value);

    //state
    State state() const;
    QString errorString();
};

typedef QSharedPointer<DPF_NAMESPACE::PluginMetaObject> PluginMetaObjectPointer;

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<< (QDebug, const DPF_NAMESPACE::PluginMetaObject &);
Q_CORE_EXPORT QDebug operator<< (QDebug, const DPF_NAMESPACE::PluginMetaObjectPointer &);
#endif //QT_NO_DEBUG_STREAM
QT_END_NAMESPACE

DPF_END_NAMESPACE

#endif // PLUGINMETAOBJECT_H
