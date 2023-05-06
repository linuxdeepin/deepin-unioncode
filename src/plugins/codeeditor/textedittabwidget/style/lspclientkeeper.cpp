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
