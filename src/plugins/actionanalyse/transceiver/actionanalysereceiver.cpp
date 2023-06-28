// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
