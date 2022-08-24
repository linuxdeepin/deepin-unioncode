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
