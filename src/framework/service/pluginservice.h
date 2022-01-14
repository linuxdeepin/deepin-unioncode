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
#ifndef PLUGINSERVICE_H
#define PLUGINSERVICE_H

#include "framework/log/frameworklog.h"
#include "framework/framework_global.h"

#include <QObject>
#include <QHash>

#include <functional>

#define DPF_INTERFACE(ret, method, ...) std::function<ret(__VA_ARGS__)> method

DPF_BEGIN_NAMESPACE

class PluginService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginService)
public:
    explicit PluginService(QObject *parent = nullptr): QObject(parent) {}
    virtual ~PluginService(){}
};

DPF_END_NAMESPACE

#endif // PLUGINSERVICE_H
