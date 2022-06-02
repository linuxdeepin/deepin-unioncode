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
    return QStringList() << T_PROJECT;
}

void FindReceiver::eventProcess(const dpf::Event &event)
{
    if (event.topic() == T_PROJECT) {
        if (event.data() == D_ACTIVED || event.data() == D_CRETED) {
            dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(event.property(P_PROJECT_INFO));
            QString filePath = projectInfo.workspaceFolder();
            QString language = projectInfo.language();
            emit FindEventTransmit::getInstance()->sendProjectPath(filePath, language);
        } else if (event.data() == D_DELETED){
            dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(event.property(P_PROJECT_INFO));
            QString filePath = projectInfo.workspaceFolder();
            emit FindEventTransmit::getInstance()->sendRemovedProject(filePath);
        } else if (event.data() == D_FILE_OPENDOCUMENT) {
            QString filePath = event.property(P_FILEPATH).toString();
            bool actived = event.property(P_OPRATETYPE).toBool();
            emit FindEventTransmit::getInstance()->sendCurrentEditFile(filePath, actived);
        }
    }
}


static FindEventTransmit *instance = nullptr;

FindEventTransmit::FindEventTransmit(QObject *parent)
    : QObject(parent)
{
}

FindEventTransmit::~FindEventTransmit()
{
    if (instance)
        delete instance;
}

FindEventTransmit* FindEventTransmit::getInstance()
{
    if (!instance) {
        instance = new FindEventTransmit();
    }
    return instance;
}
