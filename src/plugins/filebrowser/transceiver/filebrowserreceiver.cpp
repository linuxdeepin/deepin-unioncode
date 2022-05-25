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

static QStringList subTopics{ T_PROJECT };

FileBrowserReceiver::FileBrowserReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<FileBrowserReceiver> ()
{

}

QStringList FileBrowserReceiver::topics()
{
    return subTopics; //绑定menu 事件
}

void FileBrowserReceiver::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic())) {
        qCritical() << event;
        abort();
    }
    if (event.topic() == T_PROJECT) {
        projectEvent(event);
    }
}

void FileBrowserReceiver::projectEvent(const dpf::Event &event)
{
    if (event.data() == D_ACTIVED) {
        auto projectInfoVar = event.property(P_PROJECT_INFO);
        if (projectInfoVar.canConvert<dpfservice::ProjectInfo>()) {
            auto proInfo = qvariant_cast<dpfservice::ProjectInfo>(projectInfoVar);
            TreeViewKeeper::instance()->treeView()->setProjectInfo(proInfo);
        }
    }

    if (event.data() == D_DELETED) {
        TreeViewKeeper::instance()->treeView()->setProjectInfo({});
    }

    if (event.data() == D_CRETED) {
        auto projectInfoVar = event.property(P_PROJECT_INFO);
        if (projectInfoVar.canConvert<dpfservice::ProjectInfo>()) {
            auto proInfo = qvariant_cast<dpfservice::ProjectInfo>(projectInfoVar);
            TreeViewKeeper::instance()->treeView()->setProjectInfo(proInfo);
        }
    }
}
