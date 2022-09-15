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
#include "symbolcorereceiver.h"
#include "mainframe/symboltreeview.h"
#include "mainframe/symbolkeeper.h"

#include "common/common.h"

#include "services/window/windowservice.h"

SymbolCoreReceiver::SymbolCoreReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<SymbolCoreReceiver> ()
{

}

dpf::EventHandler::Type SymbolCoreReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList SymbolCoreReceiver::topics()
{
    return {T_PROJECT}; //绑定menu 事件
}

void SymbolCoreReceiver::eventProcess(const dpf::Event &event)
{
    if (event.topic() == T_PROJECT) {
        projectEvent(event);
    }
}

void SymbolCoreReceiver::projectEvent(const dpf::Event &event)
{
    if (event.data() == D_CRETED) {
        auto infoVar = event.property(P_PROJECT_INFO);
        if (infoVar.isValid() && infoVar.canConvert<dpfservice::ProjectInfo>()){
            dpfservice::ProjectInfo proInfo  = qvariant_cast<dpfservice::ProjectInfo>(infoVar);
            QStandardItem *root = nullptr;
            {
                using namespace dpfservice;
                auto &ctx = dpfInstance.serviceContext();
                auto projectService = ctx.service<ProjectService>(ProjectService::name());
                auto generator = projectService->createGenerator<SymbolGenerator>(MWMFA_CXX);
                QObject::connect(generator, &Generator::finished, [=](bool isNormal){
                    SymbolKeeper::instance()->treeView()->viewport()->update();
                });
                root = generator->createRootItem(proInfo);
            }
            SymbolKeeper::instance()->treeView()->appendRoot(root);
        }
    }

    if (event.data() == D_ACTIVED) {
        auto infoVar = event.property(P_PROJECT_INFO);
        if (infoVar.isValid() && infoVar.canConvert<dpfservice::ProjectInfo>()){
            dpfservice::ProjectInfo proInfo  = qvariant_cast<dpfservice::ProjectInfo>(infoVar);
            auto item = SymbolKeeper::instance()->treeView()->itemFromProjectInfo(proInfo);
            if (!item)
                return;
            SymbolKeeper::instance()->treeView()->activeRoot(item);
        }
    }

    if (event.data() == D_DELETED) {
        auto infoVar = event.property(P_PROJECT_INFO);
        if (infoVar.isValid() && infoVar.canConvert<dpfservice::ProjectInfo>()){
            dpfservice::ProjectInfo proInfo  = qvariant_cast<dpfservice::ProjectInfo>(infoVar);
            auto item = SymbolKeeper::instance()->treeView()->itemFromProjectInfo(proInfo);
            if (!item)
                return;
            SymbolKeeper::instance()->treeView()->deleteRoot(item);
        }
    }
}


