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
#include "serverproxy.h"
#include "setting.h"
#include "route.h"

#include <QtAlgorithms>
#include <QRunnable>
#include <QtConcurrent>

#include <iostream>
namespace Private {
static QByteArray readBuffer;
} //namespace Private
using namespace jsonrpc;
ServerProxy::ServerProxy(jsonrpc::TcpSocketServer &server)
    : AbstractServer<ServerProxy>(server)
{
    this->bindAndAddMethod(Procedure("fromRequest", PARAMS_BY_NAME, JSON_STRING, "data", JSON_STRING, NULL),
                           &ServerProxy::fromRequest);
    this->bindAndAddNotification(Procedure("fromNotify", PARAMS_BY_NAME, NULL), &ServerProxy::fromNotify);
}

void ServerProxy::fromRequest(const Json::Value &request, Json::Value &response)
{
    auto workspace = QString::fromStdString(request["workspace"].asString());
    auto language = QString::fromStdString(request["language"].asString());
    auto orginData = QString::fromStdString(request["data"].asString());
    auto orginList = orginData.split("\r\n\r\n");
    QJsonObject orginJsonObject;
    if (orginList.size() == 2) {
        QJsonParseError err;
        orginJsonObject = QJsonDocument::fromJson(orginList[1].toLatin1(), &err).object();
        if (err.error != QJsonParseError::NoError) {
            qCritical() << err.errorString();
        }
    }
    Route::Head head{workspace, language};
    if (orginJsonObject.value("method").toString() == "initialize") {
        auto backend = new Backend(Setting::getInfo(language));
        Route::instance()->saveBackend(head, backend);
    }
    auto backend = Route::instance()->backend(head);
    if (backend) {
        auto requestData = orginList.join("\r\n\r\n").toLatin1();
        qInfo() << "-> to backend request\n" << requestData;
        backend->writeAndWait(requestData);

        while (backend->canRead() || !Private::readBuffer.contains("\"result\":")) {
            Private::readBuffer += backend->readAndWait();
        }
        response["data"] = Json::Value(Private::readBuffer.toStdString());
        Private::readBuffer.clear();
        qInfo() << "<-- front request result\n" << Private::readBuffer;
    } // can find backend main
}

void ServerProxy::fromNotify(const Json::Value &request)
{
    auto workspace = QString::fromStdString(request["workspace"].asString());
    auto language = QString::fromStdString(request["language"].asString());
    auto orginData = QString::fromStdString(request["data"].asString());
    auto orginList = orginData.split("\r\n\r\n");
    QJsonObject orginJsonObject;
    if (orginList.size() == 2) {
        QJsonParseError err;
        orginJsonObject = QJsonDocument::fromJson(orginList[1].toLatin1(), &err).object();
        if (err.error != QJsonParseError::NoError) {
            qCritical() << err.errorString();
        }
    }
    Route::Head head{workspace, language};
    auto backend = Route::instance()->backend(head);
    if (backend) {
        auto requestData = orginList.join("\r\n\r\n").toLatin1();
        qInfo() << "-> to backend notify\n" << requestData;
        backend->writeAndWait(requestData);

        QString methodName = orginJsonObject.value("method").toString();
        if ( methodName == "textDocument/didOpen"
                || methodName == "textDocument/didChange") {
            while (backend->canRead() ||
                   !Private::readBuffer.contains("\"method\":\"textDocument/publishDiagnostics\"")) {
                Private::readBuffer += backend->readAndWait();
            }
        }
    }
}

bool ServerProxy::jsonObjectIsResult(const QJsonObject &jsonObj)
{
    auto keys = jsonObj.keys();
    return keys.contains("id") && keys.contains("result");
}

bool ServerProxy::jsonObjectIsMethod(const QJsonObject &jsonObj)
{
    auto keys = jsonObj.keys();
    return keys.contains("method");
}
