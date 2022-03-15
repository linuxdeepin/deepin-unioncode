/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "serverinfo.h"

const int kPort = 4711;
ServerInfo::ServerInfo(QObject *parent) : QObject(parent)
{

}

int ServerInfo::port() const
{
    return iPort;
}

void ServerInfo::setPort(int port)
{
    iPort = port;
}

const QString &ServerInfo::gethost() const
{
    return host;
}

void ServerInfo::setHost(QString &_host)
{
    host = _host;
}
