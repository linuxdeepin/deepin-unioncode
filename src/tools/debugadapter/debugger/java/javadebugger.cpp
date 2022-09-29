/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer:
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
#include "javadebugger.h"

#include "serverinfo.h"
#include "javaparam.h"
#include "common/util/processutil.h"

#include <QProcess>
#include <QDebug>
#include <QRegularExpression>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QApplication>
#include <QDir>

class JavaDebuggerPrivate {
    friend class JavaDebugger;
    QProcess process;
    QString tempBuffer;
    JavaParam javaparam;    
    QString configPath;
    int dapRequestId = 0;
    int requestId = 1;
    bool initialized = false;
};

JavaDebugger::JavaDebugger(QObject *parent)
    : QObject(parent)
    , d(new JavaDebuggerPrivate())
{
    registerLaunchDAPConnect();

    connect(this, &JavaDebugger::sigSendDAPPort, [](int port) {
        QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                      "com.deepin.unioncode.interface",
                                                      "send_java_dapport");
        msg << port;
        QDBusConnection::sessionBus().send(msg);
    });

    connect(&d->process, &QProcess::readyReadStandardOutput, [this]() {
        QByteArray data = d->process.readAllStandardOutput();

       // qInfo() << "message:" << qPrintable(data);
        parseDAPPort(data);
    });

    connect(&d->process, &QProcess::readyReadStandardError, [this]() {
        QByteArray data = d->process.readAllStandardError();
        qInfo() << "error:" << qPrintable(data);
    });
}

JavaDebugger::~JavaDebugger()
{

}

void JavaDebugger::registerLaunchDAPConnect()
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    sessionBus.disconnect(QString(""),
                          "/path",
                          "com.deepin.unioncode.interface",
                          "launch_java_dap",
                          this, SLOT(slotReceivePojectInfo(QString, QString, QString, QString,
                                                           QString, QString, QString, QString)));
    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "launch_java_dap",
                       this, SLOT(slotReceivePojectInfo(QString, QString, QString, QString,
                                                        QString, QString, QString, QString)));
}

void JavaDebugger::initialize(QString configHomePath,
                              QString jreExecute,
                              QString launchPackageFile,
                              QString launchConfigPath)
{
    if (d->initialized)
        return;

    d->configPath = QDir::homePath() + configHomePath;
    int startPort = 6000;

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

    QString validPort = QString::number(startPort);

    QString javaPath = d->configPath + jreExecute;
    QString launcherPath = d->configPath + launchPackageFile;
    QString configLinuxPath = d->configPath + launchConfigPath;
    QString heapDumpPath = d->configPath + "data/heapdump/headdump.java";
    QString dataPath = d->configPath + "data/jdt_ws";

    QString param = d->javaparam.getInitBackendParam(validPort,
                                                     javaPath,
                                                     launcherPath,
                                                     heapDumpPath,
                                                     configLinuxPath,
                                                     dataPath);
    qInfo() << validPort;
    QStringList options;
    options << "-c" << param;
    d->process.start("/bin/bash", options);
    d->process.waitForStarted();

    d->initialized = true;
}

void JavaDebugger::slotReceivePojectInfo(QString workspace,
                                         QString triggerFile,
                                         QString configHomePath,
                                         QString jrePath,
                                         QString jreExecute,
                                         QString launchPackageFile,
                                         QString launchConfigPath,
                                         QString dapPackageFile)
{
    initialize(configHomePath, jreExecute, launchPackageFile, launchConfigPath);

    d->requestId = 1;
    int pid = static_cast<int>(QApplication::applicationPid());

    QStringList commandQueue;
    const QString jdkHome = d->configPath + jrePath;
    const QString debugJar = d->configPath + dapPackageFile;
    commandQueue << d->javaparam.getLSPInitParam(d->requestId++, pid, workspace, triggerFile, jdkHome, debugJar);
    commandQueue << d->javaparam.getLSPInitilizedParam(d->requestId++);
    d->dapRequestId = d->requestId++;
    commandQueue << d->javaparam.getLaunchJavaDAPParam(d->dapRequestId);

    foreach (auto command, commandQueue) {
        executeCommand(command);
    }
}

void JavaDebugger::executeCommand(QString command)
{
    int length = command.length();
    QString writeStr = QString("Content-Length:%1\r\n\r\n%2").arg(length).arg(command);
    qInfo() << writeStr;
    d->process.write(writeStr.toUtf8());
    d->process.waitForBytesWritten();
}


void JavaDebugger::parseDAPPort(const QString &content)
{
    const auto PORT_REG = QRegularExpression(R"({"jsonrpc":"2.0","id":([0-9]+),"result":([0-9]+)})",
                                                  QRegularExpression::NoPatternOption);
    // ex: {"jsonrpc":"2.0","id":3,"result":33097}
    qInfo() << content << endl;
    QRegularExpressionMatch regMatch;
    if ((regMatch = PORT_REG.match(content)).hasMatch()) {
        qInfo() << regMatch;
        int requestId = regMatch.captured(1).trimmed().toInt();
        if (d->dapRequestId == requestId) {
            int port = regMatch.captured(2).trimmed().toInt();
            emit sigSendDAPPort(port);
        }
    }
}
