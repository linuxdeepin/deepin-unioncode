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
#ifndef SERVERPROXY_H
#define SERVERPROXY_H

#include "route.h"
#include "jsonrpccpp/server.h"
#include "jsonrpccpp/server/connectors/tcpsocketserver.h"

class ServerProxy : public jsonrpc::AbstractServer<ServerProxy>
{
public:
  ServerProxy(jsonrpc::TcpSocketServer &server);
  void fromRequest(const Json::Value &request, Json::Value &response);
  void fromNotify(const Json::Value &request);
private:
  bool jsonObjsContainsId(QVector<QJsonObject> &jsonObjs, int id);
  bool jsonObjsContainsMethod(QVector<QJsonObject> &jsonObjs, const QString &methedName);
  bool jsonObjContainsMethod(const QJsonObject &jsonObj, const QString &methodName);
  bool jsonObjIsResult(const QJsonObject &jsonObj);
};

#endif // SERVERPROXY_H
