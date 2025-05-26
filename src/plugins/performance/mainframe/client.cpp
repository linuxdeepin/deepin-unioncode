// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "client.h"

Client::Client(const std::string &ipToConnect, const unsigned int &port)
    : QObject ()
    , tcpSocketClient(ipToConnect, port)
{

}

void Client::initialzation(int pid)
{
    jsonrpc::Client sender(tcpSocketClient);
    Json::Value params(Json::objectValue);
    params["processId"] = Json::Value(int(pid));
    Json::Value orgData(Json::objectValue);
    orgData["params"] = params;
    qInfo()  << "initialization -->";
    try {
        initialzationResult(sender.CallMethod("initialization", orgData));
    } catch (jsonrpc::JsonRpcException &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Client::pullData()
{
    jsonrpc::Client sender(tcpSocketClient);
    Json::Value params(Json::objectValue);
    Json::Value orgData(Json::objectValue);
    orgData["params"] = params;
    qInfo() << qApp->thread() << QThread::currentThread();
    qInfo() << "pullData -->";
    try {
        Json::Value result = sender.CallMethod("pullData", orgData);
        if (!result.empty()) {
            pullDataResult(result);
        }
    } catch (jsonrpc::JsonRpcException &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Client::shutdown()
{
    jsonrpc::Client sender(tcpSocketClient);
    Json::Value params(Json::objectValue);
    Json::Value orgData(Json::objectValue);
    orgData["params"] = params;
    qInfo() << "shutdown -->";
    try {
        shutdownResult(sender.CallMethod("shutdown", orgData));
    } catch (jsonrpc::JsonRpcException &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Client::exit()
{
    jsonrpc::Client sender(tcpSocketClient);
    Json::Value params(Json::objectValue);
    Json::Value orgData(Json::objectValue);
    orgData["params"] = params;
    std::cout << "exit -->" << std::endl;
    try {
        sender.CallNotification("exit", orgData);
    } catch (jsonrpc::JsonRpcException &e) {
        std::cerr << e.what() << std::endl;
    }
}
