// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONTAINERSERVICE_H
#define CONTAINERSERVICE_H

#include <framework/framework.h>

#include <QMap>

namespace dpfservice {

struct  ContainerInfo
{
    const QString projectPath;
    const QUuid uuid;
    const qint64 pid;
    const QString imageName;
    const QString platformInfo;

    explicit ContainerInfo()
        : projectPath(QString{"project"})
        , uuid(QUuid())
        , pid(0)
        , imageName(QString{})
        , platformInfo{QString{"amd64"}}
    {

    }
    ~ContainerInfo()
    {

    }
};

struct ContainerError
{
    const bool result;
    const QString errString;
    explicit ContainerError()
        : result(false)
        , errString{QString{"failed"}}
    {

    }
    ~ContainerError()
    {

    }
};

class ContainerService final : public dpf::PluginService, dpf::AutoServiceRegister<ContainerService>
{
    Q_OBJECT
    Q_DISABLE_COPY(ContainerService)
public:
    static QString name()
    {
        return "org.deepin.service.ContainerService";
    }

    explicit ContainerService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    // 生成config.json文件 -> projectPath, name changed to container.support
    // 生成UUID
    DPF_INTERFACE(ContainerError, addProject, const QString &projectPath);

    // setp1. scanf container.support, copy that to rootfs exits dir and changed name to config.json
    // setp2. execute runc run "projectPath";
    // setp3. addWidgetContext to mainWindow
    DPF_INTERFACE(ContainerInfo, initContainer, const QString &projectPath);

    // setp1. selection uuid and kill from runc command
    DPF_INTERFACE(ContainerInfo, stopContainer, const QString &projectPath);

    // delete container.support
    // delete UUID
    // remove contex
    DPF_INTERFACE(ContainerError, removeProject, const QString &projectPath);

    // exec interface
    DPF_INTERFACE(ContainerError, execContainerCommand, const QString &projectPath, const QString &text);

};

} // namespace dpfservice

#endif // CONTAINERSERVICE_H
