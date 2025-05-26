// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
