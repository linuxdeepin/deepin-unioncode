// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        if (!language.isEmpty() && !storage.isEmpty()) {
            SymbolKeeper::instance()->doParse({workspace, language, storage});
        }
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
