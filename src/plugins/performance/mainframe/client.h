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
#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtConcurrent>

#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/tcpsocketclient.h>

#include <iostream>

class Client : public QObject
{
    Q_OBJECT
    jsonrpc::TcpSocketClient tcpSocketClient;
public:
    Client(const std::string &ipToConnect, const unsigned int &port);

public slots:
    void initialzation(int pid);
    void pullData();
    void shutdown();
    void exit();

signals:
    void initialzationResult(const Json::Value &result);
    void pullDataResult(const Json::Value &result);
    void shutdownResult(const Json::Value &result);
};

#endif // CLIENT_H
