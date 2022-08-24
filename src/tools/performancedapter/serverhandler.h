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
