// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    return { project.topic, editor.topic };
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
    } else if (event.data() == editor.openedFile.name) {
        QString filePath = event.property(editor.openedFile.pKeys[0]).toString();
        emit FindEventTransmit::instance()->sendCurrentEditFile(filePath, true);
    } else if (event.data() == editor.closedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        emit FindEventTransmit::instance()->sendCurrentEditFile(filePath, false);
    } else if (event.data() == editor.switchedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        emit FindEventTransmit::instance()->sendCurrentEditFile(filePath, true);
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
