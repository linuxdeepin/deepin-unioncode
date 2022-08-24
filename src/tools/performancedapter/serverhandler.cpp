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
#include "serverhandler.h"

#include <QMetaObject>
#include <QMutex>
#include <QReadLocker>
#include <QWriteLocker>

namespace  {
static QReadWriteLock rwLock;
}

ServerHandler::ServerHandler(TcpSocketServer &server, Tools *tools)
    : AbstractServer<ServerHandler>(server)
    , tools(tools)
{
    qRegisterMetaType<Json::Value>("Json::Value");
    qRegisterMetaType<Json::Value>("Json::Value&");
    this->bindAndAddMethod(Procedure("initialization", PARAMS_BY_NAME, JSON_STRING, "params", JSON_OBJECT, NULL),
                           &ServerHandler::initialization);
    this->bindAndAddMethod(Procedure("pullData", PARAMS_BY_NAME, JSON_STRING, "params", JSON_OBJECT, NULL),
                           &ServerHandler::pullData);
    this->bindAndAddMethod(Procedure("shutdown", PARAMS_BY_NAME, JSON_STRING, "params", JSON_OBJECT, NULL),
                           &ServerHandler::shutdown);
    this->bindAndAddNotification(Procedure("exit", PARAMS_BY_NAME, NULL), &ServerHandler::exit);

    QObject::connect(tools, &Tools::attachData, this, [=](const Json::Value &value){
        std::cout << "update counts from attach process data" << std::endl;
        QWriteLocker lock(&rwLock);
        toolsData = value;
    });
}

void ServerHandler::initialization(const Json::Value &request, Json::Value &response)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << request.toStyledString() << std::endl;
    Json::Value params = request["params"];
    int pid = params["processId"].asInt();

    std::cout << "attach process id:" << pid << std::endl;
    if (!tools) {
        // tools->setAttachPID(pid);
        QObject::metaObject()->invokeMethod(tools, "setAttachPID", Qt::QueuedConnection, Q_ARG(int, pid));
        QObject::metaObject()->invokeMethod(tools, "startAll", Qt::QueuedConnection);
    } else {
        QObject::metaObject()->invokeMethod(tools, "stopAll", Qt::QueuedConnection);
        // tools->setAttachPID(pid);
        QObject::metaObject()->invokeMethod(tools, "setAttachPID", Qt::QueuedConnection, Q_ARG(int, pid));
        QObject::metaObject()->invokeMethod(tools, "startAll", Qt::QueuedConnection);
    }
    response["result"] = {};
}

void ServerHandler::pullData(const Json::Value &request, Json::Value &response)
{
    std::cout << __FUNCTION__ << std::endl;
    Q_UNUSED(request)
    QReadLocker lock(&rwLock);
    response["result"] = toolsData;
}

void ServerHandler::shutdown(const Json::Value &request, Json::Value &response)
{
    std::cout << __FUNCTION__ << std::endl;
    Q_UNUSED(request);
    int pid = tools->attachPID();
    QObject::metaObject()->invokeMethod(tools, "stopAll", Qt::QueuedConnection);
    Json::Value result;
    toolsData.clear();
    result["processId"] = pid;
    response["result"] = result;
}

void ServerHandler::exit(const Json::Value &request)
{
    std::cout << __FUNCTION__ << std::endl;
    Q_UNUSED(request);
    QObject::metaObject()->invokeMethod(tools, "stopAll", Qt::QueuedConnection);
    StopListening();
    toolsData.clear();
    std::cout << "server exit..." << std::endl;
    qApp->exit(0);
}
