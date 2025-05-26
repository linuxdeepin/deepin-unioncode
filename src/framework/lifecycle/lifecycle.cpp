// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lifecycle.h"
#include "pluginmetaobject.h"

#include <QtConcurrent>
#include <QFutureWatcher>

DPF_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(PluginManager, pluginManager);

/** @brief 设置插件身份标识，与Qt插件的IID标识接口保持一致
 * 你可以在任何的代码块中使用，如下
 * @code
 * LifeCycle::setPluginIID("org.deepin.plugin.[XXX]")
 * @encode
 * @param QString 可以传入任意标识IID的字符
 * @return void
 */
PluginManager *LifeCycle::getPluginManagerInstance()
{
    return pluginManager;
}

void LifeCycle::setPluginIID(const QString &pluginIID)
{
    return pluginManager->setPluginIID(pluginIID);
}

/** @brief 获取插件身份标识
 * 如果当前未设置任意代码
 * @code
 * LifeCycle::setPluginIID("org.deepin.plugin.[XXX]")
 * @encode
 * @param QString 可以传入任意标识IID的字符
 * @return void
 */
QString LifeCycle::pluginIID()
{
    return pluginManager->pluginIID();
}

/** @brief 获取设置的插件路径
 * @return QStringList 返回插件路径列表
 */
QStringList LifeCycle::pluginPaths()
{
    return pluginManager->pluginPaths();
}

/** @brief 设置的插件路径
 * @param const QStringList &pluginPaths 设置插件的路径
 * @return void
 */
void LifeCycle::setPluginPaths(const QStringList &pluginPaths)
{
    return pluginManager->setPluginPaths(pluginPaths);
}

/** @brief 获取插件服务的路径
 * @return QStringList 返回插件服务路径的列表
 */
QStringList LifeCycle::servicePaths()
{
    return pluginManager->servicePaths();
}

/** @brief 设置服务的路径列表路径
 * @param const QStringList &servicePaths 设置插件服务的路径列表
 * @return void
 */
void LifeCycle::setServicePaths(const QStringList &servicePaths)
{
    return pluginManager->setServicePaths(servicePaths);
}

PluginMetaObjectPointer LifeCycle::pluginMetaObj(const QString &pluginName,
                                                 const QString version)
{
    return pluginManager->pluginMetaObj(pluginName, version);
}

/** @brief 读取所有的插件的元数据
 * @pre {
 *  在使用该函数之前应该先调用setPluginIID与setPluginPaths函数，否则执行该函数没有任何意义
 * }
 * @details 该函数执行将自动扫描设置的插件路径下所有符合IID要求的插件，同时读取插件元数据。
 *  当插件管理器PluginManager读取到相关插件元数据后，
 *  内部将产生PluginMetaObject元数据对象，
 *  可参阅文件pluginmetaobject.h/.cpp
 * @return void
 */
bool LifeCycle::readPlugins()
{
    return pluginManager->readPlugins();
}

/** @brief 加载所有插件
 * @pre {
 *  需要先执行readPlugins，否则将无法加载任何插件
 * }
 * @details 内部使用QPluginLoader对读取到插件元数据的插件进行插件加载
 *  该函数将调用 PluginManager中如下函数
 * @code
 *  loadPlugins();
 *  initPlugins();
 *  startPlugins();
 * @endcode
 * 详情可参阅 class PluginManager
 */
static QFutureWatcher<bool> watcher;
bool LifeCycle::loadPlugins()
{
    QObject::connect(&watcher, &QFutureWatcher<bool>::finished, [&]() {
        if (watcher.result()) {
            pluginManager->initPlugins();
            pluginManager->startPlugins();
        } else {
            qCritical() << "Failed, Load other plugin error!";
        }
    });

    QFuture<bool> future = QtConcurrent::run([=]() -> bool {
        return pluginManager->loadPlugins();
    });

    watcher.setFuture(future);

    return true;
}

/** @brief 卸载所有插件
 * @pre {
 *  需要先执行loadPlugins，否则调用无意义
 * }
 * @details 内部将执行释放Plugin接口指针，
 *  随后内部将执行QPluginLoader unloader函数，参照Qt特性
 * 详情可参阅 class PluginManager中stopPlugins函数
 */
void LifeCycle::shutdownPlugins()
{
    pluginManager->stopPlugins();
}

void LifeCycle::setSettings(PluginSetting *settings)
{
    pluginManager->setSettings(settings);
}

bool LifeCycle::loadPlugin(PluginMetaObjectPointer &pointer)
{
    if (!pluginManager->loadPlugin(pointer))
        return false;
    if (!pluginManager->initPlugin(pointer))
        return false;
    if (!pluginManager->startPlugin(pointer))
        return false;

    return true;
}

void LifeCycle::shutdownPlugin(PluginMetaObjectPointer &pointer)
{
    pluginManager->stopPlugin(pointer);
}

DPF_END_NAMESPACE
