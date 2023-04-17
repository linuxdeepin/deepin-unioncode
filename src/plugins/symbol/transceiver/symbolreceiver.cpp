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
#include "symbolreceiver.h"
#include "mainframe/symboltreeview.h"
#include "mainframe/symbolkeeper.h"

#include "common/common.h"

#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

typedef FileOperation FO;

SymbolReceiver::SymbolReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<SymbolReceiver> ()
{

}

dpf::EventHandler::Type SymbolReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList SymbolReceiver::topics()
{
    return {symbol.topic, project.topic}; //绑定menu 事件
}

void SymbolReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.activedProject.name) {
        QString projectInfoKey = project.activedProject.pKeys[0];
        dpfservice::ProjectInfo info = qvariant_cast<dpfservice::ProjectInfo>
                (event.property(projectInfoKey));
        QString workspace = info.workspaceFolder();
        QString language = info.language();
        QString storage = FO::checkCreateDir(FO::checkCreateDir(workspace, ".unioncode"), "symbol");
        SymbolKeeper::instance()->doParse({workspace, language, storage});
        SymbolKeeper::instance()->treeView()->setRootPath(storage);
    } else if (event.data() == symbol.parse.name) { // "workspace", "language", "storage"
        bool bSuccess = event.property("success").toBool();
        if(bSuccess) {
            QString workspace = event.property("workspace").toString();
            QString language = event.property("language").toString();
            QString storage = event.property("storage").toString();
            SymbolKeeper::instance()->doParse({workspace, language, storage});
            SymbolKeeper::instance()->treeView()->setRootPath(storage);
        }
    }
}
