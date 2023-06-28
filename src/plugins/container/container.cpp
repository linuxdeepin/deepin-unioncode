// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "container.h"
#include "mainframe/virtualconsole.h"
#include "mainframe/containerkeeper.h"

#include "services/window/windowservice.h"

#include "common/common.h"
#include "common/util/processutil.h"
#include "base/abstractwidget.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QTabWidget>

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
        windowService->addContextWidget("Virtual", new AbstractWidget(ContainerKeeper::instance()->tabWidget()));
    }

//    auto projectPath="/home/zhxiao/Workspaces/QThreadWorker";
//    addProject(projectPath);
//    static auto initInfo = initContainer(projectPath);
//    execContainerCommand(projectPath, QString{"git clone -b dev https://gerrit.uniontech.com/unioncode.git /usr/src/unioncode\n"});
    return true;
}

dpf::Plugin::ShutdownFlag Container::stop()
{
    qInfo() << __FUNCTION__;
    auto projectPath="/home/zhxiao/Workspaces/QThreadWorker";
    stopContainer(projectPath);
    removeProject(projectPath);
    return Sync;
}

ContainerError Container::addProject(const QString &projectPath)
{
    qInfo() << __FUNCTION__;
    ContainerError error;
    auto cachePath = CustomPaths::user(CustomPaths::Flags::Configures);
    QFile containersUuid(cachePath + QDir::separator() + QString{"containers"});
    QString containerId = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    if (containersUuid.open(QIODevice::ReadWrite | QIODevice::Append)) {
        QString pathUuidPair = projectPath + " " + containerId + "\n";
        containersUuid.write(pathUuidPair.toStdString().c_str());
        pathUuidMap[projectPath] = pathUuidPair.toStdString().c_str();
    }
    containersUuid.close();

    return error;
}

ContainerError Container::initContainer(const QString &projectPath)
{
    qInfo() << __FUNCTION__;
    ContainerError error;
    QFileInfo projectFileInfo(projectPath);
    QFileInfo projectDirInfo(projectFileInfo.filePath());
    QString projectDirName = projectDirInfo.fileName();

    auto tabWidget = ContainerKeeper::instance()->tabWidget();
    if (tabWidget) {
        virtualConsole = new VirtualConsole();
        tabWidget->insertTab(0, virtualConsole, projectDirName);
        tabWidget->setTabToolTip(0, projectPath);
    }

    auto platformPath = CustomPaths::user(CustomPaths::Flags::Configures) + QDir::separator() + QString{"platform"};
    auto amd64PlatformPath = platformPath + QDir::separator() + QString{"amd64"};
    auto arm64PlatformPath = platformPath + QDir::separator() + QString{"arm64"};
    auto mips64PlatformPath = platformPath + QDir::separator() + QString{"mips64"};
    auto amd64PlatformDir = QDir(amd64PlatformPath);
    if (amd64PlatformDir.mkpath(amd64PlatformPath)) {
        qInfo() << "make path $HOME/.config/unioncode/configures/platform/amd64 failed!";
    }
    auto arm64PlatformDir = QDir(arm64PlatformPath);
    if (arm64PlatformDir.mkpath(arm64PlatformPath)) {
        qInfo() << "make path $HOME/.config/unioncode/configures/platform/arm64 failed!";
    }
    auto mips64PlatformDir = QDir(mips64PlatformPath);
    if (mips64PlatformDir.mkpath(mips64PlatformPath)) {
        qInfo() << "make path $HOME/.config/unioncode/configures/platform/mips64 failed!";
    }

    // query container id by project path
    containerId = pathUuidMap.find(projectPath).value().split(" ").last().remove("\n");

    QDir::setCurrent(amd64PlatformPath);
    auto currentPath = QDir::currentPath();

    scriptsPath = CustomPaths::global(CustomPaths::Flags::Scripts);
    auto projectRootPath = QFileInfo(projectPath).path();
    auto containerUp = scriptsPath + QDir::separator() + QString{"container-up.sh"} + " " + QString{"amd64"} + " " + containerId + " " + projectRootPath + "\n";
    qInfo() << containerUp;
    if (virtualConsole) {
        virtualConsole->sendText(containerUp);
        virtualConsole->sendText("apt update\n");
    }

    return error;
}

ContainerError Container::stopContainer(const QString &projectPath)
{
    qInfo() << __FUNCTION__;
    ContainerError error;
    //containerId = pathUuidMap.find(projectPath).value().split(" ").last().remove("\n");

    // kill container by container id
    auto containerDown = scriptsPath + QDir::separator() + QString{"container-down.sh"} + " " + containerId + "\n";
    if (virtualConsole) {
        virtualConsole->sendText("exit\n");
        virtualConsole->sendText(containerDown);
    }

    return error;
}

ContainerError Container::removeProject(const QString &projectPath)
{
    qInfo() << __FUNCTION__;
    ContainerError error;
    pathUuidMap.remove(projectPath);

    auto cachePath = CustomPaths::user(CustomPaths::Flags::Configures);
    QFile containersUuid(cachePath + QDir::separator() + QString{"containers"});
    if (containersUuid.exists()) {
        // sed '/parten/d' containers.uuid
        QString args = "/" + projectPath.split("/").last() + "/d";
        ProcessUtil::execute("sed", QStringList{args, QString{"containers"} });
    }
    containersUuid.close();

    return error;
}

ContainerError Container::execContainerCommand(const QString &projectPath,  const QString &text)
{
    qInfo() << __FUNCTION__;
    ContainerError error;
    Q_UNUSED(projectPath);

    if (virtualConsole) {
        qInfo() << text;
        virtualConsole->sendText(text);
    }

    return  error;
}

