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
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThread>

#include "unistd.h"

static QString packageInstallPath(const QString &python)
{
    auto getVersion = [](const QString &output) -> QString {
        static QRegularExpression regex(R"((\d{1,3}(?:\.\d{1,3}){0,2}))");
        if (output.isEmpty())
            return "";

        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch())
            return match.captured(1);

        return "";
    };

    QProcess process;
    process.start(python, { "--version" });
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString version = getVersion(output);
    if (version.isEmpty()) {
        output = process.readAllStandardError();
        version = getVersion(output);
        if (version.isEmpty()) {
            int index = python.lastIndexOf('/') + 1;
            output = python.mid(index, python.length() - index);
            version = getVersion(output);
        }
    }
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
           + "/.unioncode/packages/Python" + version;
}

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

    QStringList options;
    options << "-c" << param;
    auto env = d->process.processEnvironment();
    env.insert("PYTHONPATH", packageInstallPath(pythonExecute));
    d->process.setProcessEnvironment(env);
    d->process.start("/bin/bash", options);
    d->process.waitForStarted();
    QThread::msleep(500);  // The port may not start listening immediately when Python starts, resulting in the IDE being unable to connect. Wait for 500ms.
}

void PythonDebugger::slotReceiveClientInfo(const QString &ppid,
                                           const QString &kit,
                                           const QString &pythonExecute,
                                           const QString &fileName,
                                           const QString &projectPath,
                                           const QString &projectCachePath)
{
    if (ppid != getppid())
        return;
    d->port = 0;
    d->process.close();
    initialize(pythonExecute, fileName, projectCachePath);
    emit sigSendToClient(ppid, d->port, kit, projectPath);
}


