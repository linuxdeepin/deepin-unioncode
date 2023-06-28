// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lspclientkeeper.h"

#include <QDir>
#include <QCoreApplication>

//#define AUTO_LANUCH_LSP_SERVER

#define LANGUAGE_ADAPTER_NAME  "languageadapter"
#define LANGUAGE_ADAPTER_PATH CustomPaths::global(CustomPaths::Tools) \
    + QDir::separator() + LANGUAGE_ADAPTER_NAME

LSPClientKeeper::LSPClientKeeper()
{
#ifdef AUTO_LANUCH_LSP_SERVER
    while (ProcessUtil::portOverhead(defPort)) {
        defPort ++;
    }
    QString currDefPort = QString::number(defPort);
    QString currPid = QString::number(qApp->applicationPid());
    lspServerProc = new QProcess;
    lspServerProc->setProgram(LANGUAGE_ADAPTER_PATH);
    lspServerProc->setArguments({"--port", currDefPort,
                                 "--parentPid", currPid});
    lspServerProc->setReadChannelMode(QProcess::SeparateChannels);
    lspServerProc->start();
    while (!lspServerProc->canReadLine()) {
        lspServerProc->waitForReadyRead();
        QByteArray lineData = lspServerProc->readLine();
        qInfo() << "[lsp server]:" << lineData;
        if ("Server started successfully\n" == lineData) {
            QObject::connect(lspServerProc, &QProcess::readyRead, [=](){
                qInfo() << qPrintable(lspServerProc->readAll());
            });
            break;
        }
    }
    qInfo() << lspServerProc << lspServerProc->state();
    if (lspServerProc->state() != QProcess::ProcessState::Running)
        qInfo() << lspServerProc->errorString() << LANGUAGE_ADAPTER_PATH;
#endif
}

LSPClientKeeper::~LSPClientKeeper()
{
    if (client)
        delete client;
}

LSPClientKeeper *LSPClientKeeper::instance()
{
    static LSPClientKeeper ins;
    return &ins;
}

newlsp::Client *LSPClientKeeper::get(const newlsp::ProjectKey &key)
{
    if (!key.isValid())
        return nullptr;

    if (client) {
        qApp->metaObject()->invokeMethod(client, "selectLspServer", Q_ARG(const newlsp::ProjectKey &, key));
    } else {
        client = new newlsp::Client();
        qApp->metaObject()->invokeMethod(client, "selectLspServer", Q_ARG(const newlsp::ProjectKey &, key));
    }

    if (!projectKeys.contains(key)) {
        QString complieDB_Path = QString::fromStdString(key.workspace) + QDir::separator() + ".unioncode";
        qApp->metaObject()->invokeMethod(client, "initRequest", Q_ARG(const QString &, complieDB_Path));
        projectKeys.append(key);
    }

    return client;
}
