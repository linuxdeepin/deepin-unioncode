/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "appbus_p.h"

#include "log/frameworklog.h"

#include <QDataStream>
#include <QFileInfo>
#include <QtConcurrent>
#include <QDirIterator>

DPF_USE_NAMESPACE

AppBusPrivate::AppBusPrivate(AppBus *dd)
    : QObject(dd)
    , q(dd)
    , tryPingString("static void AppBusPrivate::tryPing(const QString &serverName)")
{
    //隐藏tmp目录下的套接字文件
    appServerName = "." + QCoreApplication::applicationName() + QString("-%0.%1")
            .arg(QCoreApplication::applicationPid())
            .arg(metaObject()->className());

    //当前master加入
    onlineServers[appServerName] = nullptr;

    for (auto val : scanfUseBusApp())
    {
        auto localSocket = new QLocalSocket;
        localSocket->setServerName(val);
        onlineServers[val] = localSocket;

        localSocket->connectToServer(QIODevice::ReadWrite); //双向通信
        QByteArray writeArray;
        QDataStream oStream(&writeArray, QIODevice::WriteOnly);
        QList<QString> onlineServerKeys = onlineServers.keys();
        oStream << onlineServerKeys;
        localSocket->write(writeArray,writeArray.size());
        localSocket->waitForBytesWritten(10);
    }

    server.setMaxPendingConnections(INT_MAX);
    //允许跨用户组交换数据
    server.setSocketOptions(QLocalServer::WorldAccessOption);

    server.listen(appServerName);

    QObject::connect(&server, &QLocalServer::newConnection,
                     this, &AppBusPrivate::procNewConnection,
                     Qt::ConnectionType::DirectConnection);

    dpfCritical() << "now online servers: "
                  << onlineServers.keys();

    if (!server.isListening())
        dpfCritical() << server.errorString();
    else
        dpfCritical() << server.serverName()
                      << "Master listening..." ;
}

void AppBusPrivate::procNewConnection()
{
    dpfCritical() << "new connect socket";
    QLocalSocket *newSocket = server.nextPendingConnection();
    newSocket->waitForReadyRead();
    QByteArray array = newSocket->readAll();
    if (isTryPing(array)) {
        newSocket->write(tryPingString.toLocal8Bit());
        newSocket->waitForBytesWritten();
        newSocket->close();
        return;
    }

    if (isKeepAlive(array)) {

        QByteArray tmpArray = array;
        QStringList onlines;
        QDataStream stream(&tmpArray,QIODevice::ReadOnly);
        stream >> onlines;

        for (auto val : onlines) {
            //接收到的路由表里面等于当前服务
            if (val == server.serverName())
                continue;

            //新增项直接保存
            if (onlineServers[val] == nullptr) {
                onlineServers[val] = newSocket;
                dpfCritical() << "insert new server"
                              << val << onlineServers[val];
                q->newCreateAppBus(val);
            }
        }
        dpfDebug() << "now onlineServers: "
                   << onlineServers;

        dpfCritical() << "now online servers:"
                      << onlineServers.keys();

        dpfCritical() << "now online servers count: "
                      << onlineServers.count();
        return;
    }


    newSocket->close();
}

QStringList AppBusPrivate::scanfUseBusApp()
{
    QString tmp = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];
    //异步迭代
    auto controller = QtConcurrent::run([=]()
    {
        QStringList fileNames;
        QDirIterator itera(tmp, QDir::NoSymLinks
                           | QDir::System
                           | QDir::Hidden
                           | QDir::NoDotAndDotDot);
        while (itera.hasNext()) {
            itera.next();
            QString fileName = itera.fileName();

            if (QFileInfo(tmp + "/" + fileName).suffix()
                    != metaObject()->className()) {
                continue;
            }

            if (fileName == appServerName)
            {
                dpfCritical() << "mime: " << fileName;
                continue;
            }

            if (!tryPing(fileName)) {
                continue;
            }

            fileNames << fileName;
        }
        return fileNames;
    });
    controller.waitForFinished();
    return controller.result(); //数据同步
}

bool AppBusPrivate::tryPing(const QString &serverName)
{
    dpfCritical() << "try ping: " << serverName;
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.isOpen()) {
        socket.write(tryPingString.toLocal8Bit());
        socket.waitForBytesWritten(); //等待数据发送完毕
        if (socket.waitForReadyRead(10)) { //等待数据接收
            QByteArray readArray = socket.readAll();
            dpfCritical() << readArray;
            if (tryPingString == readArray) { //数据认证成功
                return true;
            }
        }
    }

    QString tmp = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];
    QFile::remove(tmp + "/" + serverName);
    return false;
}

bool AppBusPrivate::isTryPing(const QByteArray &array)
{
    return tryPingString == array;
}

bool AppBusPrivate::isKeepAlive(const QByteArray &array)
{
    QByteArray tmpArray = array;
    QList<QString> onlines;
    QDataStream stream(&tmpArray, QIODevice::ReadOnly);
    stream >> onlines;
    return onlines.contains(server.serverName());
}

