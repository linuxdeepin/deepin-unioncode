/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#include "actionanalysereceiver.h"
#include "mainframe/analysekeeper.h"
#include "mainframe/configure.h"

#include "common/common.h"

ActionAnalyseReceiver::ActionAnalyseReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<ActionAnalyseReceiver> ()
{

}

dpf::EventHandler::Type ActionAnalyseReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ActionAnalyseReceiver::topics()
{
    return {actionanalyse.topic, symbol.topic};
}

void ActionAnalyseReceiver::eventProcess(const dpf::Event &event)
{
    if (Configure::enabled()) {
        if (event.data() == actionanalyse.analyse.name) { // "workspace", "language", "storage"
            QString workspaceKey = actionanalyse.analyseDone.pKeys[0]; // workspace
            QString languageKey = actionanalyse.analyseDone.pKeys[1]; // language
            QString storageKey = actionanalyse.analyseDone.pKeys[2]; // storage
            QString dataKey = actionanalyse.analyseDone.pKeys[3];
            QString workspace = event.property(workspaceKey).toString();
            QString language = event.property(languageKey).toString();
            QString storage = event.property(storageKey).toString();
            QVariant var = event.property(dataKey);
            AnalysedData analyData = var.value<AnalysedData>();
            AnalyseKeeper::instance()->doAnalyse({workspace, language, storage});
        } else if (event.data() == symbol.parseDone.name) {
            bool bSuccess = event.property("success").toBool();
            if(bSuccess) {
                QString workspace = event.property("workspace").toString();
                QString language = event.property("language").toString();
                const QString &storage = workspace;
                AnalyseKeeper::instance()->doAnalyse({workspace, language, storage});
            }
        }
    }
}
