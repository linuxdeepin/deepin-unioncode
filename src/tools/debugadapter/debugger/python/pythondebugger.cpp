/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "pythondebugger.h"

#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QApplication>
#include <QDir>

class PythonDebuggerPrivate {
    friend class PythonDebugger;
    QProcess process;
    QString kit;
    int port = 0;
};

PythonDebugger::PythonDebugger(QObject *parent)
    : QObject(parent)
    , d(new PythonDebuggerPrivate())
{
    registerLaunchDAPConnect();

    connect(this, &PythonDebugger::sigSendToClient, [](const QString &uuid, int port, const QString &kit) {
        QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                      "com.deepin.unioncode.interface",
                                                      "dapport");
        msg << uuid
            << port
            << kit
            << QMap<QString, QVariant>();
        QDBusConnection::sessionBus().send(msg);
    });

    connect(&d->process, &QProcess::readyReadStandardOutput, [this]() {
        QByteArray data = d->process.readAllStandardOutput();
        //qInfo() << "message:" << qPrintable(data);
    });

    connect(&d->process, &QProcess::readyReadStandardError, [this]() {
        QByteArray data = d->process.readAllStandardError();
        //qInfo() << "error:" << qPrintable(data);
    });
}

PythonDebugger::~PythonDebugger()
{
    if (d)
        delete d;
}

void PythonDebugger::registerLaunchDAPConnect()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.disconnect(QString(""),
                          "/path",
                          "com.deepin.unioncode.interface",
                          "launch_python_dap",
                          this, SLOT(slotReceiveClientInfo(QString, QString, QString, QString, QString)));
    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "launch_python_dap",
                       this, SLOT(slotReceiveClientInfo(QString, QString, QString, QString, QString)));
}

void PythonDebugger::initialize(const QString &pythonExecute,
                                const QString &fileName,
                                const QString &projectCachePath)
{
    int startPort = 7000;
    auto checkPortFree = [](int port) {
        QProcess process;
        QString cmd = QString("fuser %1/tcp").arg(port);
        process.start(cmd);
        process.waitForFinished();
        QString ret = process.readAll();
        if (ret.isEmpty())
            return true;
        return false;
    };

    while (startPort) {
        if (checkPortFree(startPort)) {
           break;
        }
        startPort--;
    }

    d->port = startPort;
    QString validPort = QString::number(d->port);
    QString pid = QString::number(QApplication::applicationPid());
    QString logFolder = projectCachePath + "/dap/pylog";
    QString param = pythonExecute + " -m debugpy --listen " + validPort +
            " --wait-for-client --log-to " + logFolder + " " + fileName + " --pid " + pid;
    qInfo() << param;

    QStringList options;
    options << "-c" << param;
    d->process.start("/bin/bash", options);
    d->process.waitForStarted();
}

void PythonDebugger::slotReceiveClientInfo(const QString &uuid, const QString &kit,
                                           const QString &pythonExecute,
                                           const QString &fileName,
                                           const QString &projectCachePath)
{
    d->port = 0;
    d->kit = kit;
    d->process.close();
    initialize(pythonExecute, fileName, projectCachePath);
    emit sigSendToClient(uuid, d->port, d->kit);
}


