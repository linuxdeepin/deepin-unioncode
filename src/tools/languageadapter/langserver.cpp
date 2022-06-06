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
#include "langserver.h"
#include "setting.h"
#include "route.h"
#include "handlerfrontend.h"

#include <QTcpSocket>
#include <QRunnable>
#include <QtConcurrent>

#include <iostream>

LangServer::LangServer(quint16 port)
{
    if (!listen(QHostAddress::Any, port)) {
        std::cerr << "Failed, listen " << errorString().toUtf8().toStdString()
                  << std::endl;
        abort();
    }
}

void LangServer::incomingConnection(qintptr handle)
{
    auto tcpSocket = new QTcpSocket();
    tcpSocket->setSocketDescriptor(handle);
    tcpSocket->setReadBufferSize(8192);
    auto frontend = new HandlerFrontend;
    frontend->bind(tcpSocket);
}

