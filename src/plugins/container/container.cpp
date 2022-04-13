/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "container.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "containerwidget.h"
#include "common/common.h"
#include "common/util/processutil.h"
#include <QDir>
#include <QFile>
#include <QUuid>

using namespace dpfservice;
void Container::initialize()
{
    qInfo() << __FUNCTION__;
    //发布Container到edit导航栏界面布局
    if (QString(getenv("TERM")).isEmpty()) {
        setenv("TERM", "xterm-256color", 1);
    }
    setenv("SHELL","/bin/bash",1);

    // 发布容器服务
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(ContainerService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool Container::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        windowService->addContextWidget("virtenv", new AbstractWidget(ContainerWidget::instance()));
    }

    return true;
}

dpf::Plugin::ShutdownFlag Container::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

ContainerError Container::addProject(const QString &projectPath)
{
    qInfo() << __FUNCTION__;
    auto cachePath = CustomPaths::user(CustomPaths::Flags::Configures);
    qInfo() << cachePath;
    QFile containersUuid(cachePath + QDir::separator() + QString{"containers.uuid"});
    QString containerId = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    if (containersUuid.open(QIODevice::ReadWrite)) {
        QString pathUuidPair = projectPath + " " + containerId;
        containersUuid.write(pathUuidPair.toStdString().c_str());
        pathUuidMap[projectPath] = pathUuidPair.toStdString().c_str();
    }
    containersUuid.close();
}

ContainerError Container::initContainer(const QString &projectPath)
{
    qInfo() << __FUNCTION__;

    auto cachePath = CustomPaths::user(CustomPaths::Flags::Configures);
    qInfo() << cachePath;
    QDir cacheDir(cachePath + QDir::separator() + QString{"platform"});
    if (!cacheDir.exists()) {
        cacheDir.mkdir(cachePath + QDir::separator() + QString{"platform"} + QDir::separator() + QString{"{amd64, arm64, mips64, loongarch64}"});
    }

    // create base rootfs image for amd64 platform
    ProcessUtil::execute("cd", QStringList{cachePath + QString{"platform/amd64"} });
    QDir rootfs("rootfs");
    if (!rootfs.exists()) {
        // execute debootstrap to generate the rootfs
        ProcessUtil::execute("debootstrap", QStringList{"--arch=amd64", "eagle", "rootfs", "https://pools.uniontech.com/uos"});
    }

    // query container id by project path
    auto containerId = pathUuidMap.find(projectPath).value();

    // prepare container network
    ProcessUtil::execute("bash", QStringList{CustomPaths::global(CustomPaths::Flags::Scripts) + QDir::separator() + "container-ifup.sh", containerId});

    // run container by container id
    ProcessUtil::execute("bash", QStringList{CustomPaths::global(CustomPaths::Flags::Scripts) + QDir::separator() + "container-up.sh", containerId});
}

ContainerError Container::stopContainer(const QString &projectPath)
{
    qInfo() << __FUNCTION__;
    auto containerId = pathUuidMap[projectPath];

    // kill container by container id
    ProcessUtil::execute("bash", QStringList{CustomPaths::global(CustomPaths::Flags::Scripts) + QDir::separator() + "container-down.sh", containerId});

    // remove conainer network
    ProcessUtil::execute("bash", QStringList{CustomPaths::global(CustomPaths::Flags::Scripts) + QDir::separator() + "container-ifdown.sh", containerId});
}

ContainerError Container::removeProject(const QString &projectPath)
{
    qInfo() << __FUNCTION__;

    pathUuidMap.remove(projectPath);

    auto cachePath = CustomPaths::user(CustomPaths::Flags::Configures);
    QFile containersUuid(cachePath + QDir::separator() + QString{"containers.uuid"});
    if (containersUuid.exists()) {
        // sed '/parten/d' containers.uuid
        QString args = "/" + projectPath.split("/").last() + "/d";
        ProcessUtil::execute("sed", QStringList{args, QString{"containers.uuid"} });
    }
    containersUuid.close();
}

ContainerError Container::execContainerCommand(const QString &projectPath, const QString &command, const QStringList &arguments)
{
    qInfo() << __FUNCTION__;

    auto containerId = pathUuidMap[projectPath];
    ProcessUtil::execute("runc exec", QStringList{containerId, command} +  arguments);
}

