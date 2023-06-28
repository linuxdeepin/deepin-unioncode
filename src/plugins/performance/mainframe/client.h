// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
