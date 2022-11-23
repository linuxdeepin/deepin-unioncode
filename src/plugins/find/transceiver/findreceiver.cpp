/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "findreceiver.h"
#include "common/common.h"
#include "services/project/projectservice.h"

FindReceiver::FindReceiver(QObject *parent)
    : dpf::EventHandler(parent)
{

}

dpf::EventHandler::Type FindReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList FindReceiver::topics()
{
    return { project.topic };
}

void FindReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.activedProject.name) {
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(
                    event.property(project.activedProject.pKeys[0]));
        QString workspace = projectInfo.workspaceFolder();
        QString language = projectInfo.language();
        emit FindEventTransmit::instance()->sendProjectPath(workspace, language);
    } else if(event.data() == project.openProject.name) {
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(
                    event.property(project.openProject.pKeys[0]));
        QString workspace = projectInfo.workspaceFolder();
        QString language = projectInfo.language();
        emit FindEventTransmit::instance()->sendProjectPath(workspace, language);
    } else if (event.data() == project.deletedProject.name){
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(
                    event.property(project.deletedProject.pKeys[0]));
        QString workspace = projectInfo.workspaceFolder();
        emit FindEventTransmit::instance()->sendRemovedProject(workspace);
    }  else if (event.data() == editor.selectedFile.name) {
        QString filePath = event.property(editor.selectedFile.pKeys[0]).toString();
        bool actived = event.property(editor.selectedFile.pKeys[1]).toBool();
        emit FindEventTransmit::instance()->sendCurrentEditFile(filePath, actived);
    }
}

FindEventTransmit::FindEventTransmit(QObject *parent)
    : QObject(parent)
{
}

FindEventTransmit::~FindEventTransmit()
{

}

FindEventTransmit* FindEventTransmit::instance()
{
    static FindEventTransmit instance;
    return &instance;
}
