// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
