// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filebrowserreceiver.h"
#include "mainframe/treeviewkeeper.h"

#include "services/project/projectservice.h"

#include "common/common.h"

FileBrowserReceiver::FileBrowserReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<FileBrowserReceiver> ()
{

}

dpf::EventHandler::Type FileBrowserReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList FileBrowserReceiver::topics()
{
    return {project.topic}; //绑定menu 事件
}

void FileBrowserReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.activatedProject.name) {
        QVariant proInfoVar = event.property(project.activatedProject.pKeys[0]);
        dpfservice::ProjectInfo proInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        TreeViewKeeper::instance()->treeView()->setProjectInfo(proInfo);
    } else if (event.data() == project.deletedProject.name) {
        TreeViewKeeper::instance()->treeView()->setProjectInfo({});
    } else if (event.data() == project.createdProject.name) {
        QVariant proInfoVar = event.property(project.activatedProject.pKeys[0]);
        dpfservice::ProjectInfo proInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        TreeViewKeeper::instance()->treeView()->setProjectInfo(proInfo);
    }
}
