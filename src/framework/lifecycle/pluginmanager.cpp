// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/pluginmanager_p.h"
#include "pluginmanager.h"

DPF_USE_NAMESPACE

/** @brief PluginManager的构造函数
 */
PluginManager::PluginManager()
    : d(new PluginManagerPrivate(this))
{

}

/** @brief setPluginIID 设置插件IID
 * @param const QString &pluginIID
 *  可以设置任意字符作为当前IID，需要与与加载插件IID一致
 * @return void
 */
void PluginManager::setPluginIID(const QString &pluginIID)
{
    d->setPluginIID(pluginIID);
}

/** @brief setPluginPaths 设置插件加载的路径
 * @param const QStringList &pluginPaths 传入路径列表
 * @return void
 */
void PluginManager::setPluginPaths(const QStringList &pluginPaths)
{
    d->setPluginPaths(pluginPaths);
}

/** @brief setServicePaths 设置当前插件服务加载的路径
 * @param const QStringList &servicePaths 传入路径列表
 * @return void
 */
void PluginManager::setServicePaths(const QStringList &servicePaths)
{
    d->setServicePaths(servicePaths);
}

PluginMetaObjectPointer PluginManager::pluginMetaObj(const QString &pluginName, const QString version) const
{
    return d->pluginMetaObj(pluginName, version);
}

bool PluginManager::loadPlugin(PluginMetaObjectPointer &pointer)
{
    return d->loadPlugin(pointer);
}

bool PluginManager::initPlugin(PluginMetaObjectPointer &pointer)
{
    return d->initPlugin(pointer);
}

bool PluginManager::startPlugin(PluginMetaObjectPointer &pointer)
{
    return d->startPlugin(pointer);
}

void PluginManager::stopPlugin(PluginMetaObjectPointer &pointer)
{
    return d->stopPlugin(pointer);
}

/** @brief readPlugins 读取插件元数据
 * @details [此函数是线程安全的]
 *  读取当前设置插件路径下的所有插件元数据
 * @return void
 */
bool PluginManager::readPlugins()
{
   return d->readPlugins();
}

/** @brief loadPlugins 加载所有的插件
 * @pre 需要先执行readPlugins方法
 * @details [此函数是线程安全的]
 * 加载所有插件，内部使用Qt类QPluginLoader
 * @return void
 */
bool PluginManager::loadPlugins()
{
    return d->loadPlugins();
}

/** @brief initPlugins 执行所有插件initialized接口
 * @pre 需要先执行loadPlugins方法
 * @details [此函数是线程安全的]
 * 调用插件接口Plugin中initialized函数，
 * @return void
 */
void PluginManager::initPlugins()
{
    d->initPlugins();
}

/** @brief startPlugins 执行所有插件代码Plugin::start接口
 * @pre 执行initPlugins后插件执行的函数，
 * 否则可能遇到未知崩溃，
 * 因为该操作的存在是未定义的。
 * @details [此函数是线程安全的]
 * 调用插件接口Plugin中start函数
 * @return void
 */
void PluginManager::startPlugins()
{
    d->startPlugins();
}

/** @brief stopPlugins 执行所有插件代码Plugin::stop接口
 * @pre 执行startPlugins后插件执行的函数，
 * 否则可能遇到未知崩溃，
 * 因为该操作的存在是未定义的。
 * @details [此函数是线程安全的]
 * 调用插件接口Plugin中stop函数，并且卸载加载的插件。
 * @return void
 */
void PluginManager::stopPlugins()
{
    d->stopPlugins();
}

void PluginManager::setSettings(PluginSetting *settings)
{
    d->setSettings(settings);
}

void PluginManager::writeSettings()
{
    d->writeSettings();
}

QHash<QString, QQueue<PluginMetaObjectPointer> > PluginManager::pluginCollections()
{
    return d->pluginCategories;
}

/** @brief pluginIID 获取插件标识IID
 * @return QString 返回设置的IID，默认为空
 */
QString PluginManager::pluginIID() const
{
    return d->pluginIID();
}

/** @brief pluginPaths 获取插件路径
 * @return QString 返回设置的插件路径，默认为空
 */
QStringList PluginManager::pluginPaths() const
{
    return d->pluginPaths();
}

/** @brief servicePaths 获取插件服务路径
 * @return QString 返回设置的插件服务路径，默认为空
 */
QStringList PluginManager::servicePaths() const
{
    return d->servicePaths();
}
