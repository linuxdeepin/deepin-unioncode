/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
    if (event.data() == project.activedProject.name) {
        QVariant proInfoVar = event.property(project.activedProject.pKeys[0]);
        dpfservice::ProjectInfo proInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        TreeViewKeeper::instance()->treeView()->setProjectInfo(proInfo);
    } else if (event.data() == project.deletedProject.name) {
        TreeViewKeeper::instance()->treeView()->setProjectInfo({});
    } else if (event.data() == project.createdProject.name) {
        QVariant proInfoVar = event.property(project.activedProject.pKeys[0]);
        dpfservice::ProjectInfo proInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        TreeViewKeeper::instance()->treeView()->setProjectInfo(proInfo);
    }
}
