// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGIN_H
#define PLUGIN_H

#include "framework/framework_global.h"
#include "framework/service/pluginservice.h"

#include <QObject>
#include <QSharedData>

DPF_BEGIN_NAMESPACE


class PluginContext;

/**
 * @brief The Plugin class
 * 插件接口类，用于实现插件,使用方式如下
 * @code
 *  class Core : public Plugin
 *  {
 *      Q_OBJECT
 *      Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE FILE "core.json")
 *  public:
 *      virtual void initialized() override;
 *      virtual bool start() override;
 *      virtual PluginMetaObject::ShutDownFlag stop() override;
 *  };
 * @endcode
 * Q_PLUGIN_METADATA 可参阅Qt宏定义
 * PLUGIN_INTERFACE
 */
class Plugin : public QObject
{
    Q_OBJECT
public:
    enum ShutdownFlag {
        Sync, /// 同步释放标识
        Async, /// 异步释放标识
    };

    explicit Plugin();
    virtual ~Plugin() override;
    virtual void initialize();

    /**
     * @brief start
     * @return bool 如果返回true则表示当前插件start函数执行正常
     * false则代表当前内部执行存在问题
     */
    virtual bool start() = 0;
    virtual ShutdownFlag stop();

signals:

    /**
     * @brief asyncStopFinished 异步释放完成的信号
     */
    void asyncStopFinished();
};

DPF_END_NAMESPACE

#endif // PLUGIN_H
