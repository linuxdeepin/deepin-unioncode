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
#include "listener/listener.h"
#include "listener/private/listener_p.h"
#include "pluginmanager_p.h"
#include "lifecycle/private/pluginmetaobject_p.h"
#include "lifecycle/plugin.h"

#include "log/frameworklog.h"

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate {
    static QMutex mutex;
} // namespace GlobalPrivate

PluginManagerPrivate::PluginManagerPrivate(PluginManager *qq)
    : q(qq)
{
    dpfCheckTimeBegin();
    dpfCheckTimeEnd();
}

PluginManagerPrivate::~PluginManagerPrivate()
{
    stopPlugins();
}

QString PluginManagerPrivate::pluginIID() const
{
    return pluginLoadIID;
}

void PluginManagerPrivate::setPluginIID(const QString &pluginIID)
{
    pluginLoadIID = pluginIID;
}

QStringList PluginManagerPrivate::pluginPaths() const
{
    return pluginLoadPaths;
}

void PluginManagerPrivate::setPluginPaths(const QStringList &pluginPaths)
{
    pluginLoadPaths = pluginPaths;
}

QStringList PluginManagerPrivate::servicePaths() const
{
    return serviceLoadPaths;
}

void PluginManagerPrivate::setServicePaths(const QStringList &servicePaths)
{
    serviceLoadPaths = servicePaths;
}

void PluginManagerPrivate::setPluginEnable(const PluginMetaObject &meta, bool enabled)
{
    return setting.setPluginEnable(meta,enabled);
}

/*!
 * \brief 获取插件的元数据，线程安全
 * \param name
 * \param version
 * \return
 */
PluginMetaObjectPointer PluginManagerPrivate::pluginMetaObj(const QString &name,
                                                            const QString &version)
{
    dpfCheckTimeBegin();

    auto controller = QtConcurrent::run([=](){
        QMutexLocker lock(&GlobalPrivate::mutex);
        int size = readQueue.size();
        int idx = 0;
        while (idx < size) {
            if (!version.isEmpty()) {
                if (readQueue[idx]->d->version == version
                        && readQueue[idx]->d->name == name) {
                    return readQueue[idx];
                }
            } else {
                if (readQueue[idx]->d->name == name) {
                    return readQueue[idx];
                }
            }
            idx++;
        }
        return PluginMetaObjectPointer(nullptr);
    });
    controller.waitForFinished();

    dpfCheckTimeBegin();

    return controller.result();
}

/*!
 * \brief 加载一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::loadPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d->state >= PluginMetaObject::State::Loaded) {
        dpfCritical() << "State error: " << pluginMetaObj->d->state;
        return false;
    }
    if (pluginMetaObj.isNull()) {
        dpfCritical() << "Failed load plugin is nullptr";
        return false;
    }

    auto controller = QtConcurrent::run([=]() {

        QMutexLocker lock(&GlobalPrivate::mutex);

        bool result = pluginMetaObj->d->loader->load();
        pluginMetaObj->d->plugin = QSharedPointer<Plugin>
                (qobject_cast<Plugin*>(pluginMetaObj->d->loader->instance()));
        if (!pluginMetaObj->d->plugin.isNull()) {
            pluginMetaObj->d->state = PluginMetaObject::State::Loaded;
            dpfDebug() << "Loaded plugin: " << pluginMetaObj->d->name;
        } else {
            pluginMetaObj->d->error = "Failed get plugin instance is nullptr: "
                    + pluginMetaObj->d->loader->errorString();
            dpfCritical() << pluginMetaObj->d->error;
            result = false;
        }
        return result;
    });
    controller.waitForFinished();

    if (!controller.result()) {
        dpfCritical() << "Failed load plugin: " << pluginMetaObj->name();
        return false;
    }

    dpfCheckTimeEnd();
    return true;
}

/*!
 * \brief 初始化一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::initPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d->state >= PluginMetaObject::State::Initialized) {
        dpfCritical() << "State error: " << pluginMetaObj->d->state;
        return false;
    }

    if (pluginMetaObj.isNull()) {
        dpfCritical() << "Failed init plugin is nullptr";
        return false;
    }

    auto pluginInterface = pluginMetaObj->plugin();

    if (pluginInterface.isNull()) {
        dpfCritical() << "Failed init plugin interface is nullptr";
        return false;
    }

    //线程互斥
    QMutexLocker lock(&GlobalPrivate::mutex);

    //    auto controller = QtConcurrent::run([=](){
    pluginMetaObj->d->plugin->initialize();
    dpfDebug() << "Initialized plugin: " << pluginMetaObj->d->name;
    //    });
    //    controller.waitForFinished();

    pluginMetaObj->d->state = PluginMetaObject::State::Initialized;

    dpfCheckTimeEnd();
    return true;
}

/*!
 * \brief 启动一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
bool PluginManagerPrivate::startPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d->state >= PluginMetaObject::State::Started) {
        dpfCritical() << "State error: " << pluginMetaObj->d->state;
        return false;
    }

    if (pluginMetaObj.isNull()) {
        dpfCritical() << "Failed start plugin is nullptr";
        return false;
    }

    auto pluginInterface = pluginMetaObj->plugin();

    if (pluginInterface.isNull()) {
        dpfCritical() << "Failed start plugin interface is nullptr";
        return false;
    }

    //线程互斥
    QMutexLocker lock(&GlobalPrivate::mutex);

    if (pluginMetaObj->d->plugin->start()) {
        pluginMetaObj->d->state = PluginMetaObject::State::Started;
        dpfDebug() << "Started plugin: " << pluginMetaObj->d->name;
    } else {
        pluginMetaObj->d->error = "Failed start plugin in function start() logic";
        dpfCritical() << pluginMetaObj->d->error.toLocal8Bit().data();
    }

    dpfCheckTimeEnd();
    return true;
}

/*!
 * \brief 停止并卸载一个插件，线程安全，可单独使用
 * \param pluginMetaObj
 */
void PluginManagerPrivate::stopPlugin(PluginMetaObjectPointer &pluginMetaObj)
{
    dpfCheckTimeBegin();

    //流程互斥
    if (pluginMetaObj->d->state >= PluginMetaObject::State::Stoped)
        return;

    //线程互斥
    QMutexLocker lock(&GlobalPrivate::mutex);

    Plugin::ShutdownFlag stFlag = pluginMetaObj->d->plugin->stop();
    pluginMetaObj->d->state = PluginMetaObject::State::Stoped;

    if (stFlag == Plugin::ShutdownFlag::Async) {

        dpfDebug() << "async stop" << pluginMetaObj->d->plugin->
                      metaObject()->className();

        pluginMetaObj->d->state = PluginMetaObject::State::Stoped;

        QObject::connect(pluginMetaObj->d->plugin.data(), &Plugin::asyncStopFinished,
                         pluginMetaObj->d->plugin.data(), [=]() {
            pluginMetaObj->d->plugin = nullptr;

            if (!pluginMetaObj->d->loader->unload()) {
                dpfDebug() << pluginMetaObj->d->loader->errorString();
            }

            pluginMetaObj->d->state = PluginMetaObject::State::Shutdown;
            dpfDebug() << "shutdown" << pluginMetaObj->d->loader->fileName();

        },Qt::ConnectionType::DirectConnection); //同步信号直接调用无需等待

    } else {

        if (pluginMetaObj->d->plugin) {
            dpfDebug() << "sync stop" << pluginMetaObj->d->plugin->
                          metaObject()->className();

            pluginMetaObj->d->plugin = nullptr;
            pluginMetaObj->d->state = PluginMetaObject::State::Stoped;
        }

        if (!pluginMetaObj->d->loader->unload()) {
            dpfDebug() << pluginMetaObj->d->loader->errorString();
        }

        pluginMetaObj->d->state = PluginMetaObject::State::Shutdown;
        dpfDebug() << "shutdown" << pluginMetaObj->d->loader->fileName();
    }

    dpfCheckTimeEnd();
}

/*!
 * \brief 读取所有插件的Json源数据
 * \return
 */
bool PluginManagerPrivate::readPlugins()
{
    dpfCheckTimeBegin();

    int currMaxCountThread = QThreadPool::globalInstance()->maxThreadCount();
    if (currMaxCountThread < 4) {
        QThreadPool::globalInstance()->setMaxThreadCount(4);
    }

    // 内部已有线程互斥
    QFuture<void> scanController = QtConcurrent::run(scanfAllPlugin,
                                                     &readQueue,
                                                     pluginLoadPaths,
                                                     pluginLoadIID);
    scanController.waitForFinished();

    QMutexLocker lock(&GlobalPrivate::mutex);

    QFuture<void> mapController = QtConcurrent::map(readQueue.begin(),
                                                    readQueue.end(),
                                                    readJsonToMeta);
    mapController.waitForFinished();

    dpfDebug() << readQueue;

    dpfCheckTimeEnd();
    return readQueue.isEmpty() ? false : true;
}

/*!
 * \brief 扫描所有插件到目标队列
 * \param destQueue
 * \param pluginPaths
 * \param pluginIID
 */
void PluginManagerPrivate::scanfAllPlugin(QQueue<PluginMetaObjectPointer> *destQueue,
                                          const QStringList &pluginPaths,
                                          const QString &pluginIID)
{
    dpfCheckTimeBegin();

    if (pluginIID.isEmpty())
        return;

    for(const QString &path : pluginPaths) {
        QDirIterator dirItera(path, {"*.so", "*.dll"},
                              QDir::Filter::Files,
                              QDirIterator::IteratorFlag::NoIteratorFlags);

        //线程安全
        QMutexLocker lock(&GlobalPrivate::mutex);

        while (dirItera.hasNext()) {
            dirItera.next();

            PluginMetaObjectPointer metaObj(new PluginMetaObject);
            metaObj->d->loader->setFileName(dirItera.path() + QDir::separator() + dirItera.fileName());
            QJsonObject &&metaJson = metaObj->d->loader->metaData();
            QString &&IID = metaJson.value("IID").toString();
            if (pluginIID != IID) {
                continue;
            }

            destQueue->append(metaObj);
            metaObj->d->state = PluginMetaObject::Readed;
        }
    }

    dpfCheckTimeEnd();
}

/*!
 * \brief 同步json到定义类型
 * \param metaObject
 */
void PluginManagerPrivate::readJsonToMeta(const PluginMetaObjectPointer &metaObject)
{
    dpfCheckTimeBegin();

    metaObject->d->state = PluginMetaObject::Reading;

    QJsonObject &&jsonObj = metaObject->d->loader->metaData();

    if (jsonObj.isEmpty()) return;

    QString &&iid = jsonObj.value("IID").toString();
    if (iid.isEmpty()) return;
    metaObject->d->iid = iid;

    QJsonObject &&metaData = jsonObj.value("MetaData").toObject();

    QString &&name = metaData.value(PLUGIN_NAME).toString();
    if (name.isEmpty()) return;
    metaObject->d->name = name;

    QString &&version = metaData.value(PLUGIN_VERSION).toString();
    metaObject->d->version = version;

    QString &&compatVersion = metaData.value(PLUGIN_COMPATVERSION).toString();
    metaObject->d->compatVersion = compatVersion;

    QString &&category = metaData.value(PLUGIN_CATEGORY).toString();
    metaObject->d->category = category;

    QJsonArray &&licenseArray = metaData.value(PLUGIN_LICENSE).toArray();
    auto licenItera = licenseArray.begin();
    while (licenItera != licenseArray.end()) {
        metaObject->d->license.append(licenItera->toString());
        ++ licenItera;
    }

    QString &&copyright = metaData.value(PLUGIN_COPYRIGHT).toString();
    metaObject->d->copyright = copyright;

    QString &&vendor = metaData.value(PLUGIN_VENDOR).toString();
    metaObject->d->vendor = vendor;

    QString &&description = metaData.value(PLUGIN_DESCRIPTION).toString();
    metaObject->d->description = description;

    QString &&urlLink = metaData.value(PLUGIN_URLLINK).toString();
    metaObject->d->urlLink = urlLink;

    QJsonArray &&dependsArray = metaData.value(PLUGIN_DEPENDS).toArray();
    auto itera = dependsArray.begin();
    while (itera != dependsArray.end()) {
        QJsonObject &&dependObj = itera->toObject();
        QString &&dependName = dependObj.value(PLUGIN_NAME).toString();
        QString &&dependVersion = dependObj.value(PLUGIN_VERSION).toString();
        PluginDepend depends;
        depends.pluginName = dependName;
        depends.pluginVersion = dependVersion;
        metaObject->d->depends.append(depends);
        ++ itera;
    }

    metaObject->d->state = PluginMetaObject::Readed;

    dpfCheckTimeEnd();
}

/*!
 * \brief 内部使用QPluginLoader加载所有插件
 */
bool PluginManagerPrivate::loadPlugins()
{
    dpfCheckTimeBegin();

    QFuture<void> sortController = QtConcurrent::run(dependsSort,
                                                     &loadQueue,
                                                     &readQueue);
    sortController.waitForFinished();

    bool ret = true;
    QFuture<void> mapController = QtConcurrent::map(readQueue.begin(),
                                                    readQueue.end(),
                                                    [&ret] (PluginMetaObjectPointer &pointer) {
            // 流程互斥
            if (pointer->d->state >= PluginMetaObject::State::Loaded) {
            dpfDebug() << "Is Loaded plugin: "
                       << pointer->d->name
                       << pointer->fileName();
            return;
}

            // 必须执行了读取操作
            if (pointer->d->state != PluginMetaObject::State::Readed) {
            dpfDebug() << "Failed load plugin: "
                       << pointer->d->name
                       << pointer->fileName();
            ret = false;
            return;
}

            //设置当前正在加载的标志位
            pointer->d->state = PluginMetaObject::State::Loading;

            if (!pointer->d->loader->load()) {
            pointer->d->error = "Failed load plugin: " + pointer->d->loader->errorString();
            dpfCritical() << pointer->errorString();
            ret = false;
            return;
}

            pointer->d->plugin = QSharedPointer<Plugin>
            (qobject_cast<Plugin*>(pointer->d->loader->instance()));

            if (pointer.isNull()) {
            pointer->d->error = "Failed get plugin instance is nullptr";
            dpfCritical() << pointer->d->error;
            ret = false;
            return;
}

            pointer->d->state = PluginMetaObject::State::Loaded;
            dpfDebug() << "Loaded plugin: " << pointer->d->name;
});
    mapController.waitForFinished();

    dpfDebug() << loadQueue;

    dpfCheckTimeEnd();
    return ret;
}

/*!
 * \brief 初始化所有插件
 */
void PluginManagerPrivate::initPlugins()
{
    dpfCheckTimeBegin();

    QFuture<void> runController = QtConcurrent::run([=](){
        QQueue<PluginMetaObjectPointer> initQueue = loadQueue;
        QQueue<PluginMetaObjectPointer>::iterator itera = initQueue.begin();
        while(itera != initQueue.end())
        {
            auto pointer = *itera;
            //流程互斥
            if (pointer->d->state >= PluginMetaObject::State::Initialized) {
                dpfDebug() << "Is initialized plugin: "
                           << pointer->d->name
                           << pointer->fileName();
                ++ itera;
                continue;
            }

            if (pointer->d->state != PluginMetaObject::State::Loaded) {
                dpfCritical() << "Failed initialized plugin"
                              << pointer->d->name
                              << pointer->fileName();
                ++ itera;
                continue;
            }

            if (!pointer->d->plugin.isNull()) {
                pointer->d->plugin->initialize();
                dpfDebug() << "Initialized plugin: " << pointer->d->name;
                pointer->d->state = PluginMetaObject::State::Initialized;
            }
            ++ itera;
        }
    });

    runController.waitForFinished();

    // 私有类转发进行Sendler闭包
    emit Listener::instance().d->pluginsInitialized();

    dpfCheckTimeEnd();
}

/*!
 * \brief 拉起插件,仅主线程使用
 */
void PluginManagerPrivate::startPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> startQueue = loadQueue;
    QQueue<PluginMetaObjectPointer>::iterator itera = startQueue.begin();
    while(itera != startQueue.end())
    {
        PluginMetaObjectPointer pointer = *itera;
        //流程互斥
        if (pointer->d->state >= PluginMetaObject::State::Started) {
            dpfDebug() << "Is started plugin:"
                       << pointer->d->name
                       << pointer->fileName();
            ++ itera;
            continue;
        }

        if (pointer->d->state != PluginMetaObject::State::Initialized) {
            dpfCritical() << "Failed start plugin:"
                          << pointer->d->name
                          << pointer->fileName();
            ++ itera;
            continue;
        }

        if (pointer->d->plugin->start()) {
            dpfDebug() << "Started plugin: " << pointer->d->name;
            pointer->d->state = PluginMetaObject::State::Started;
        } else {
            pointer->d->error = "Failed start plugin in function start() logic";
            dpfCritical() << pointer->d->error.toLocal8Bit().data();
        }

        ++ itera;
    }

    // 私有类转发进行Sendler闭包
    emit Listener::instance().d->pluginsStarted();

    dpfCheckTimeEnd();
}

/*!
 * \brief 停止插件,仅主线程
 */
void PluginManagerPrivate::stopPlugins()
{
    dpfCheckTimeBegin();

    QQueue<PluginMetaObjectPointer> stopQueue = loadQueue;
    auto itera = stopQueue.rbegin();
    while(itera != stopQueue.rend())
    {
        PluginMetaObjectPointer pointer = *itera;
        if (pointer->d->state != PluginMetaObject::State::Started) {
            continue;
        }

        Plugin::ShutdownFlag stFlag = pointer->d->plugin->stop();
        pointer->d->state = PluginMetaObject::State::Stoped;

        if (stFlag == Plugin::ShutdownFlag::Async) {

            dpfDebug() << "Stoped async plugin: " << pointer->d->name;

            pointer->d->state = PluginMetaObject::State::Stoped;

            QObject::connect(pointer->d->plugin.data(), &Plugin::asyncStopFinished,
                             pointer->d->plugin.data(), [=]()
            {
                pointer->d->plugin = nullptr;

                if (!pointer->d->loader->unload()) {
                    dpfDebug() << pointer->d->loader->errorString();
                }

                pointer->d->state = PluginMetaObject::State::Shutdown;
                dpfDebug() << "shutdown" << pointer->d->loader->fileName();

            },Qt::ConnectionType::DirectConnection); //同步信号直接调用无需等待

        } else {

            if (pointer->d->plugin) {
                dpfDebug() << "Stoped sync plugin: " << pointer->d->name;
                pointer->d->plugin = nullptr;
                pointer->d->state = PluginMetaObject::State::Stoped;
            }

            if (!pointer->d->loader->unload()) {
                dpfDebug() << pointer->d->loader->errorString();
                continue;
            }

            pointer->d->state = PluginMetaObject::State::Shutdown;
            dpfDebug() << "shutdown" << pointer->d->loader->fileName();
        }

        ++ itera;
    }

    // 私有类转发进行Sendler闭包
    emit Listener::instance().d->pluginsStoped();

    dpfCheckTimeEnd();
}

/*!
 * \brief 按照依赖排序
 * \param dstQueue
 * \param srcQueue
 */
void PluginManagerPrivate::dependsSort(QQueue<PluginMetaObjectPointer> *dstQueue,
                                       QQueue<PluginMetaObjectPointer> *srcQueue)
{
    dpfCheckTimeBegin();

    QMutexLocker lock(&GlobalPrivate::mutex);

    *dstQueue = (*srcQueue);
    typedef PluginMetaObjectPointer EmleTPointer;
    std::sort(dstQueue->begin(), dstQueue->end(), [=](EmleTPointer after, EmleTPointer befor)
    {
        dpfDebug() << after->d->name << befor->d->name;
        if (after->depends().isEmpty()) {
            //前节点没有依赖则保持顺序
            return true;
        } else { //前节点存在依赖
            if(befor->depends().isEmpty()) {
                //后节点为空则调整顺序
                return false;
            } else { //后节点存在依赖
                //遍历后节点依赖
                for (auto depend : befor->d->depends)
                {
                    //后节点依赖存在前节点
                    if (depend.name() == after->name())
                        return true;
                }
                //遍历前节点依赖
                for (auto depend : after->d->depends)
                {
                    //前节点依赖存在后节点
                    if (depend.name() == befor->name())
                        return false;
                }
            }
        }
        dpfDebug() << "Unknown Error from" << Q_FUNC_INFO;
        return false;
    });

    dpfCheckTimeEnd();
}

DPF_END_NAMESPACE
