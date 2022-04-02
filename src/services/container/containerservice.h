/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xiaozaihu<xiaozaihu@uniontech.com>
 *
 * Maintainer: xiaozaihu<xiaozaihu@uniontech.com>
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
#ifndef CONTAINERSERVICE_H
#define CONTAINERSERVICE_H

#include <framework/framework.h>

#include <QMap>

namespace dpfservice {
class ContainerService final : public dpf::PluginService, dpf::AutoServiceRegister<ContainerService>
{
    Q_OBJECT
    Q_DISABLE_COPY(ContainerService)
public:
    static QString name();

    explicit ContainerService(QObject *parent = nullptr);

    // 添加容器挂载点
    DPF_INTERFACE(void, addContainerMount, const QString& containrId);
    // 添加容器网络
    DPF_INTERFACE(void, addContainerNetwork, const QString& containerId);
    // 添加容器根文件系统
    DPF_INTERFACE(void, addConntainerBundle, const QString& containerId);
    // 添加容器配置文件
    DPF_INTERFACE(void, addContainerConfig, const QString& containerId);
    // 运行容器虚拟环境
    DPF_INTERFACE(void, addContainerVirtenv, const QString& containerId);
    // 杀死容器虚拟环境
    DPF_INTERFACE(void, killContainerVirtenv, const QString& containerId);
};

} // namespace dpfservice

#endif // CONTAINERSERVICE_H
