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

#define LANGUAGE_ADAPTER_NAME  "languageadapter"
#define LANGUAGE_ADAPTER_PATH CustomPaths::global(CustomPaths::Tools) \
    + QDir::separator() + LANGUAGE_ADAPTER_NAME

static unsigned int defPort = 3307;

LSPClientKeeper::LSPClientKeeper()
{
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
}

LSPClientKeeper::~LSPClientKeeper()
{
    auto itera = clients.begin();
    while (itera != clients.end()) {

        QThread *cliTh = itera.value().second;
        if (cliTh->isRunning()) {
            cliTh->exit();
            cliTh->deleteLater();
        }

        lsp::Client *client = itera.value().first;
        if (client) {
            delete client;
        }

        itera = clients.erase(itera);
    }

    if (lspServerProc) {
        lspServerProc->kill();
        lspServerProc->waitForFinished();
        delete lspServerProc;
        lspServerProc = nullptr;
    }
}

LSPClientKeeper *LSPClientKeeper::instance()
{
    static LSPClientKeeper ins;
    return &ins;
}

void LSPClientKeeper::initClient(const lsp::Head &head, const QString &complie)
{
    if (head.isValid()) {
        auto client = new lsp::Client(defPort);
        auto *clientTh = new QThread();
        client->moveToThread(clientTh);
        clientTh->start();

        client->setHead(head);
        qApp->metaObject()->invokeMethod(client, "initRequest", Q_ARG(const QString &, complie));
        clients[head] = {client, clientTh};
    }
}

void LSPClientKeeper::shutdownClient(const lsp::Head &head)
{
    auto client = clients[head].first;
    if (client) {
        qApp->metaObject()->invokeMethod(client, "shutdownRequest");
        qApp->metaObject()->invokeMethod(client, "exitRequest");
        clients.remove(head);
    }
}

lsp::Client *LSPClientKeeper::get(const lsp::Head &head)
{
    return clients[head].first;
}
