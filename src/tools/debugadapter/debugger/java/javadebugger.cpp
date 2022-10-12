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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class JavaDebuggerPrivate {
    friend class JavaDebugger;
    QProcess process;
    QString tempBuffer;
    JavaParam javaparam;
    QString configPath;
    int dapRequestId = 0;
    int mainClassRequestId = 0;
    int classPathRequestId = 0;

    int port = 0;
    QString mainClass;
    QString projectName;
    QStringList classPaths;

    int requestId = 1;
    bool initialized = false;
};

JavaDebugger::JavaDebugger(QObject *parent)
    : QObject(parent)
    , d(new JavaDebuggerPrivate())
{
    registerLaunchDAPConnect();
    connect(this, &JavaDebugger::sigResolveClassPath, this, &JavaDebugger::slotResolveClassPath);
    connect(this, &JavaDebugger::sigCheckInfo, this, &JavaDebugger::slotCheckInfo);

    connect(this, &JavaDebugger::sigSendToClient, [](int port, QString mainClass, QString projectName, QStringList classPaths) {
        QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                      "com.deepin.unioncode.interface",
                                                      "send_java_dapport");
        msg << port
            << mainClass
            << projectName
            << classPaths;
        QDBusConnection::sessionBus().send(msg);
    });

    connect(&d->process, &QProcess::readyReadStandardOutput, [this]() {
        QByteArray data = d->process.readAllStandardOutput();

       // qInfo() << "message:" << qPrintable(data);
        parseResult(data);
    });

    connect(&d->process, &QProcess::readyReadStandardError, [this]() {
        QByteArray data = d->process.readAllStandardError();
        //qInfo() << "error:" << qPrintable(data);
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
                          this, SLOT(slotReceivePojectInfo(QString, QString, QString,
                                                           QString, QString, QString, QString)));
    sessionBus.connect(QString(""),
                       "/path",
                       "com.deepin.unioncode.interface",
                       "launch_java_dap",
                       this, SLOT(slotReceivePojectInfo(QString, QString, QString,
                                                        QString, QString, QString, QString)));
}

void JavaDebugger::initialize(const QString &configHomePath,
                              const QString &jreExecute,
                              const QString &launchPackageFile,
                              const QString &launchConfigPath)
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

void JavaDebugger::slotReceivePojectInfo(const QString &workspace,
                                         const QString &configHomePath,
                                         const QString &jrePath,
                                         const QString &jreExecute,
                                         const QString &launchPackageFile,
                                         const QString &launchConfigPath,
                                         const QString &dapPackageFile)
{
    d->port = 0;
    d->mainClass.clear();
    d->projectName.clear();
    d->classPaths.clear();
    d->requestId = 1;

    initialize(configHomePath, jreExecute, launchPackageFile, launchConfigPath);

    int pid = static_cast<int>(QApplication::applicationPid());

    QStringList commandQueue;
    const QString jdkHome = d->configPath + jrePath;
    const QString debugJar = d->configPath + dapPackageFile;
    commandQueue << d->javaparam.getLSPInitParam(d->requestId++, pid, workspace, jdkHome, debugJar);
    commandQueue << d->javaparam.getLSPInitilizedParam(d->requestId++);

    d->dapRequestId = d->requestId++;
    commandQueue << d->javaparam.getLaunchJavaDAPParam(d->dapRequestId);

    d->mainClassRequestId = d->requestId++;
    commandQueue << d->javaparam.getResolveMainClassParam(d->mainClassRequestId, workspace);

    foreach (auto command, commandQueue) {
        executeCommand(command);
    }
}

void JavaDebugger::executeCommand(const QString &command)
{
    int length = command.length();
    QString writeStr = QString("Content-Length:%1\r\n\r\n%2").arg(length).arg(command);
    //qInfo() << writeStr;
    d->process.write(writeStr.toUtf8());
    d->process.waitForBytesWritten();
}

void JavaDebugger::slotResolveClassPath(const QString &mainClass, const QString &projectName)
{
    d->classPathRequestId = d->requestId++;
    QString command = d->javaparam.getResolveClassPathParam(d->classPathRequestId, mainClass, projectName);
    executeCommand(command);
}

void JavaDebugger::parseResult(const QString &content)
{
    // ex: {"jsonrpc":"2.0","id":3,"result":33097}
    const auto PORT_REG = QRegularExpression(R"({"jsonrpc":"2.0","id":([0-9]+),"result":([0-9]+)})",
                                                  QRegularExpression::NoPatternOption);

    // ex: {"jsonrpc":"2.0","id":3,"result":[{"mainClass":"com.uniontech.App","projectName":"maven_demo",
    //      "filePath":"/home/zhouyi/Desktop/debugJava/new/maven_demo/src/main/java/com/uniontech/App.java"}]}

    // ex: {"jsonrpc":"2.0","id":3,"result":[[],["/home/zhouyi/Desktop/debugJava/new/maven_demo/target/classes"]]}
    const auto CONTENT_REG = QRegularExpression(R"({"jsonrpc":"2.0","id":([0-9]+),"result":\[(.+)\]})",
                                                  QRegularExpression::NoPatternOption);
    //qInfo() << content << endl;
    QRegularExpressionMatch regMatch;
    if ((regMatch = PORT_REG.match(content)).hasMatch()) {
        //qInfo() << regMatch;

        int requestId = regMatch.captured(1).trimmed().toInt();
        if (d->dapRequestId == requestId) {
            d->port = regMatch.captured(2).trimmed().toInt();
            emit sigCheckInfo();
        }
    } else if ((regMatch = CONTENT_REG.match(content)).hasMatch()) {
        //qInfo() << regMatch;
        int requestId = regMatch.captured(1).trimmed().toInt();
        if (d->mainClassRequestId == requestId) {
            QString content = regMatch.captured(2).trimmed();
            if (parseMainClass(content, d->mainClass, d->projectName))
                emit sigResolveClassPath(d->mainClass, d->projectName);
        } else if (d->classPathRequestId == requestId) {
            QString content = regMatch.captured(2).trimmed();
            if (parseClassPath(content, d->classPaths))
                emit sigCheckInfo();
        }
    }
}

bool JavaDebugger::parseMainClass(const QString &content, QString &mainClass, QString &projectName)
{
    if (content.isEmpty())
        return false;

    QString newContent = "{\"result\":[" + content + "]}";
    //qInfo() << newContent;

    //ex : "{\"result\":[{\"mainClass\":\"com.uniontech.App\",\"projectName\":\"maven_demo\",\"filePath\":\"/App.java\"}]}"
    QByteArray data = newContent.toUtf8();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (QJsonParseError::NoError != parseError.error) {
        return false;
    }

    if (!doc.isObject())
        return false;

    QJsonObject rootObject = doc.object();
    QJsonArray array = rootObject.value("result").toArray();
    foreach (auto value, array) {
        QJsonObject object = value.toObject();
        mainClass = object.value("mainClass").toString();
        projectName = object.value("projectName").toString();

        if (!mainClass.isEmpty() && !projectName.isEmpty())
            return true;
    }

    return false;
}

bool JavaDebugger::parseClassPath(const QString &content, QStringList &classPaths)
{
    if (content.isEmpty())
        return false;

    QString newContent = "{\"result\":[" + content + "]}";
    //qInfo() << newContent;

    //ex : "{\"result\":[[],[\"/home/maven_demo/maven_demo/target/classes\"]]}"
    QByteArray data = newContent.toUtf8();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (QJsonParseError::NoError != parseError.error) {
        return false;
    }

    if (!doc.isObject())
        return false;

    QJsonObject rootObject = doc.object();
    QJsonArray array = rootObject.value("result").toArray();
    foreach (auto value, array) {
        QJsonArray subArray = value.toArray();
        foreach (auto subValue, subArray) {
            QString classPath = subValue.toString();
            if (!classPath.isEmpty())
                classPaths.append(classPath);
        }
    }

    if (!classPaths.isEmpty())
        return true;

    return false;
}

void JavaDebugger::slotCheckInfo()
{
    if (d->port > 0
            && !d->mainClass.isEmpty()
            && !d->projectName.isEmpty()
            && !d->classPaths.isEmpty())
        emit sigSendToClient(d->port, d->mainClass, d->projectName, d->classPaths);
}

