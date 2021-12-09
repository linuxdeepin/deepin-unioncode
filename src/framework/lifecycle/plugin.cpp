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
#include "plugin.h"

DPF_USE_NAMESPACE

/**
 * @brief Plugin::Plugin 插件默认构造接口
 */
Plugin::Plugin()
{

}

/**
 * @brief Plugin::~Plugin 插件析构函数
 */
Plugin::~Plugin()
{

}

/**
 * @brief Plugin::initialized 插件初始化接口
 * @details 此函数是多线程执行，内部可用于一些线程安全的函数、类操作
 * @return void
 */
void Plugin::initialize()
{

}

/**
 * @brief Plugin::stop
 * @return PluginMetaObject::ShutDownFlag 释放的方式
 * 目前支持Sync(同步)与Async(异步)
 * 如果使用Async，那么插件的构建者应当发送信号
 * @code
 * emit asyncStopFinished
 * @endcode
 * 否则将导致内存泄露，或者无法卸载插件。
 */
Plugin::ShutdownFlag Plugin::stop()
{
    return ShutdownFlag::Sync;
}
