// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "serverapplication.h"

#include "common/lsp/protocol/newprotocol.h"

#include <QTimer>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QDateTime>

#include <iostream>

namespace newlsp {

static ServerApplication *_globalServer;
class ServerApplicationPrivate
{
    friend class ServerApplication;
    StdinJsonRpcParser *stdinParser{nullptr};
    QTimer *globalPPidWatcher;
};

ServerApplication::~ServerApplication()
{
    _globalServer = nullptr;

    if (d) {
        if (d->stdinParser) {
            d->stdinParser->quit();
            delete d->stdinParser;
        }
        if (d->globalPPidWatcher) {
            d->globalPPidWatcher->stop();
        }
        delete d;
    }
}

void ServerApplication::start()
{
    _globalServer = this;
    QObject::connect(qApp, &QCoreApplication::destroyed, qApp, [=](){
        delete this;
    });
    if (ServerCmdParse::parentPid()) {
        unsigned int ppid = ServerCmdParse::parentPid().value();
        d->globalPPidWatcher = new QTimer;
        d->globalPPidWatcher->setInterval(300);
        QObject::connect(d->globalPPidWatcher, &QTimer::timeout, [=](){
            QProcess process;
#ifdef __linux__
            process.setProgram("ps");
            process.setArguments({"--pid", QString::number(ppid)});
            process.start();
            process.waitForFinished();
            QByteArray queryPidLines = process.readAllStandardOutput();
            if (queryPidLines.isEmpty() || queryPidLines.count('\n') < 2) {
                lspServErr << "not found parent pid:"
                           << std::to_string(ppid)
                           << "qApp quit";
                exit(1001);
            }
#endif
        });
        d->globalPPidWatcher->start();
    }

    auto doStdioLauch = [=](){
        if (!d->stdinParser) {
            d->stdinParser = new StdinJsonRpcParser;
            QObject::connect(d->stdinParser, &StdinJsonRpcParser::readedJsonObject,
                             this, &ServerApplication::identifyJsonObject,
                             Qt::ConnectionType::DirectConnection);
            d->stdinParser->start();
        }
        lspServOut << "Initialization succeeded, mode's stdio";
    };

    if (ServerCmdParse::mode()) {
        std::string mode = ServerCmdParse::mode().value();
        if (mode == newlsp::tcp.toStdString()) {
            lspServErr << "This function is not implemented yet. "
                          "It is started in stdio mode by default";
        }
        if (ServerCmdParse::mode() == newlsp::stdio.toStdString()) {
            doStdioLauch();
        }
    }
    doStdioLauch();
}

ServerApplication::ServerApplication(const QCoreApplication &app)
    : ServerCmdParse (app)
    , d (new ServerApplicationPrivate())
{

}

ServerApplication *ServerApplication::ins()
{
    return _globalServer;
}

ServerApplication::Stderr ServerApplication::err(const std::vector<std::string> &perfixs)
{
    return Stderr(perfixs);
}

ServerApplication::Stdout ServerApplication::out(const std::vector<std::string> &perfixs)
{
    return Stdout(perfixs);
}

QString ServerApplication::toProtocolString(int id, const QString method, const QJsonObject &params)
{
    QJsonObject obj;
    obj[K_ID] = id;
    obj[K_METHOD] = method;
    obj[K_PARAMS] = params;
    return toProtocolString(obj);
}

QString ServerApplication::toProtocolString(const QString &method, const QJsonObject &params)
{
    QJsonObject obj;
    obj[K_METHOD] = method;
    obj[K_PARAMS] = params;
    return toProtocolString(obj);
}

QString ServerApplication::toProtocolString(const QJsonObject &object)
{
    QJsonObject temp = object;
    temp[K_JSON_RPC] = "2.0";
    QString body = QJsonDocument(temp).toJson(QJsonDocument::Compact);
    QString head = QString("Content-Length: ") + QString::number(body.size()) + "\r\n\r\n";
    return head + body;
}

QString ServerApplication::localDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void ServerApplication::jsonrpcJsonOutput(const QJsonObject &obj)
{
    if (!ServerCmdParse::mode() || ServerCmdParse::mode() == newlsp::stdio.toStdString()) {
        std::cout << toProtocolString(obj).toStdString() << std::endl;
    } else {
        lspServErr << "Output of json is not allowed in this mode"
                   << ServerCmdParse::mode().value();
    }
}

void ServerApplication::identifyJsonObject(const QJsonObject &obj)
{
    auto objKeys = obj.keys();
    if (objKeys.contains(K_ID) && objKeys.contains(K_JSON_RPC)
            && objKeys.contains(K_METHOD) && objKeys.contains(K_PARAMS)) {
        int idStr = obj.value(K_ID).toInt();
        QString methodStr = obj.value(K_METHOD).toString();
        QJsonObject params = obj.value(K_PARAMS).toObject();
        jsonrpcMethod(idStr, methodStr, params);
    } else if (objKeys.contains(K_JSON_RPC) && objKeys.contains(K_METHOD)
               && objKeys.contains(K_PARAMS)) {
        QString methodStr = obj.value(K_METHOD).toString();
        QJsonObject params = obj.value(K_PARAMS).toObject();
        jsonrpcNotification(methodStr, params);
    }
}

} //newlsp
