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
#include <QRegularExpression>

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
        // send to backend
        int id = orginJsonObject.value("id").toInt();
        auto requestData = orginList.join("\r\n\r\n").toLatin1();
        backend->writeAndWait(requestData);
        qInfo() << "-> to backend request\n" << requestData;

        // recv backend and wait
        QVector<QJsonObject> retJsonObjs;
        while (!jsonObjsContainsId(retJsonObjs, id)) {
            backend->readAndWait(retJsonObjs, Private::readBuffer);
        }

        //send data to frontend;
        response["data"] = Json::Value(Private::readBuffer.toStdString());
        qInfo() << "<-- front request result\n" << Private::readBuffer;
        Private::readBuffer.clear();
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

        // send to backend
        auto requestData = orginList.join("\r\n\r\n").toLatin1();
        backend->writeAndWait(requestData);
        qInfo() << "-> to backend notify\n" << requestData;

        // recv wait from backend
        if (jsonObjContainsMethod(orginJsonObject, "textDocument/didOpen")
                || jsonObjContainsMethod(orginJsonObject, "textDocument/didChange")) {
            QVector<QJsonObject> retJsonObjs;
            while (!jsonObjsContainsMethod(retJsonObjs, "textDocument/publishDiagnostics")) {
                if (!backend->readAndWait(retJsonObjs, Private::readBuffer)) {
                    if (Private::readBuffer.contains("\"method\":\"textDocument/publishDiagnostics\""))
                        return;
                }
            }
        }

        if (jsonObjContainsMethod(orginJsonObject, "exit")) {
            Route::instance()->removeBackend(head);
        }
    }
}

bool ServerProxy::jsonObjsContainsId(QVector<QJsonObject> &jsonObj, int id)
{
    for (auto val : jsonObj) {
        if (val.value("id").toInt() == id)
            return true;
    }
    return false;
}

bool ServerProxy::jsonObjsContainsMethod(QVector<QJsonObject> &jsonObjs, const QString &methedName)
{
    for (auto val : jsonObjs) {
        if (val.value("method").toString() == methedName)
            return true;
    }
    return false;
}

bool ServerProxy::jsonObjContainsMethod(const QJsonObject &jsonObj, const QString &methodName)
{
    if (jsonObj.value("method").toString() == methodName)
        return true;
    return false;
}

bool ServerProxy::jsonObjIsResult(const QJsonObject &jsonObj)
{
    auto keys = jsonObj.keys();
    return keys.contains("id") && keys.contains("result");
}
