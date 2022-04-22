/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:    xiaozaihu<xiaozaihu@uniontech.com>
 *
 * Maintainer: xiaozaihu<xiaozaihu@uniontech.com>
 *
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
#ifndef CONTAINERPLUGIN_H
#define CONTAINERPLUGIN_H
#include <framework/framework.h>
#include "services/container/containerservice.h"
#include "mainframe/virtualconsole.h"
#include <QMap>
#include <QString>
class Container : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "container.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

    dpfservice::ContainerError addProject(const QString& projectPath);
    dpfservice::ContainerError initContainer(const QString& projectPath);
    dpfservice::ContainerError stopContainer(const QString& projectPath);
    dpfservice::ContainerError removeProject(const QString& projectPath);
    dpfservice::ContainerError execContainerCommand(const QString &projectPath,  const QString &text);
private:
    QString containerId;
    QString scriptsPath;
    QMap<QString, QString> pathUuidMap;
    VirtualConsole *virtualConsole{nullptr};
};

#endif // CONTAINERPLUGIN_H
