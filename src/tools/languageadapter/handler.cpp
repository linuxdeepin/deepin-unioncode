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
#include "handler.h"

Handler::Handler()
    : QObject()
{

}

Handler::~Handler()
{
    if (device) {
        auto process = qobject_cast<QProcess *> (device);
        if (process) {
            process->kill();
            process->close();
            delete process;
            device = nullptr;
        }
    }
    if (device) {
        auto socket = qobject_cast<QTcpSocket *>(device);
        if (socket) {
            socket->disconnectFromHost();
            socket->close();
            delete socket;
            device = nullptr;
        }
    }
}

void Handler::bind(QProcess *qIODevice)
{
    device = qIODevice;
    QObject::connect(qIODevice, &QProcess::readyReadStandardOutput,
                     this, &Handler::doReadAll,
                     Qt::UniqueConnection);
}

void Handler::bind(QTcpSocket *qIODevice)
{
    device = qIODevice;
    QObject::connect(qIODevice, &QTcpSocket::readyRead,
                     this, &Handler::doReadAll,
                     Qt::UniqueConnection);
}

void Handler::nowToWrite(const QByteArray &array)
{
    if (device) {
        device->write(array);
        device->waitForBytesWritten(3000);
    }
}

void Handler::doReadAll()
{
    auto array = device->readAll();
    filterData(array);
    emit nowReadedAll(array);
}
