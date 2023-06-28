// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    int port = 0;
};

PythonDebugger::PythonDebugger(QObject *parent)
    : QObject(parent)
    , d(new PythonDebuggerPrivate())
{
    registerLaunchDAPConnect();

    connect(this, &PythonDebugger::sigSendToClient, [](const QString &uuid, int port, const QString &kit,
            const QString &projectPath) {
        QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                      "com.deepin.unioncode.interface",
                                                      "dapport");
        QMap<QString, QVariant> param;
        param.insert("workspace", projectPath);
        msg << uuid
            << port
            << kit
            << param;
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
                          this, SLOT(slotReceiveClientInfo(QString, QString, QString, QString, QString, QString)));
    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "launch_python_dap",
                       this, SLOT(slotReceiveClientInfo(QString, QString, QString, QString, QString, QString)));
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

void PythonDebugger::slotReceiveClientInfo(const QString &uuid,
                                           const QString &kit,
                                           const QString &pythonExecute,
                                           const QString &fileName,
                                           const QString &projectPath,
                                           const QString &projectCachePath)
{
    d->port = 0;
    d->process.close();
    initialize(pythonExecute, fileName, projectCachePath);
    emit sigSendToClient(uuid, d->port, kit, projectPath);
}


