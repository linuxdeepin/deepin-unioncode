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
#include "builderreceiver.h"
#include "mainframe/buildmanager.h"
#include "common/common.h"
#include "services/project/projectservice.h"

BuilderReceiver::BuilderReceiver(QObject *parent)
    : dpf::EventHandler(parent)
{

}

dpf::EventHandler::Type BuilderReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList BuilderReceiver::topics()
{
    return QStringList() << T_PROJECT << T_DEBUGGER;
}

void BuilderReceiver::eventProcess(const dpf::Event &event)
{
    if (event.topic() == T_PROJECT) {
        if (event.data() == D_ACTIVED || event.data() == D_CRETED) {
            dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(event.property(P_PROJECT_INFO));
            BuildManager::instance()->setActivedProjectInfo(projectInfo.kitName(), projectInfo.workspaceFolder());
        } else if (event.data() == D_DELETED){
            dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(event.property(P_PROJECT_INFO));
            BuildManager::instance()->clearActivedProjectInfo();
        }
    } else if (event.topic() == T_DEBUGGER) {
       if (event.data() == D_DEBUG_EXECUTION_START) {
           BuildManager::instance()->buildProject();
       }
   }
}

