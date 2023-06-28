// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include "tools.h"

#include <QVector>
#include <QProcess>
#include <QVariant>
#include <QDebug>
#include <QMetaEnum>
#include <QDir>
#include <QCoreApplication>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/tcpsocketserver.h>

#include <iostream>

using namespace jsonrpc;
using namespace std;

class ServerHandler :public QObject, public AbstractServer<ServerHandler>
{
    Q_OBJECT
    Tools *tools{nullptr};
    Json::Value toolsData;
public:
    ServerHandler(TcpSocketServer &server, Tools *tools = nullptr);

    void initialization(const Json::Value &request, Json::Value &response);
    void pullData(const Json::Value &request, Json::Value &response);
    void shutdown(const Json::Value &request, Json::Value &response);
    void exit(const Json::Value &request);
};



#endif // SERVERHANDLER_H
