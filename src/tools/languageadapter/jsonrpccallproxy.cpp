// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsonrpccallproxy.h"
#include <QDebug>

JsonRpcCallProxy::JsonRpcCallProxy(QObject *parent)
    : Route(parent)
{
    lspServOut << __FUNCTION__ << this;
}

JsonRpcCallProxy &JsonRpcCallProxy::ins()
{
    if (qApp->thread() != QThread::currentThread()) {
        qCritical() << "can create from main thread";
        qApp->quit();
    }
    static JsonRpcCallProxy proxy;
    return proxy;
}

bool JsonRpcCallProxy::bindCreateProc(const std::string &language, const ProcCreatFunc &func)
{
    if (createFuncs.value(language.c_str()))
        return false;
    createFuncs.insert(language.c_str(), func);
    return true;
}

QProcess *JsonRpcCallProxy::createLspServ(const newlsp::ProjectKey &key)
{
    QString langQStr(key.language.c_str());
    QString workQStr(key.language.c_str());
    auto createFunc = createFuncs.value(langQStr);
    if (createFunc)
        return createFunc(key);
    return nullptr;
}

void JsonRpcCallProxy::methodFilter(int id, const QString &method, const QJsonObject &params)
{
    lspServOut << __FUNCTION__ << qApp->thread() << QThread::currentThread();
    lspServOut << "method -->" << method.toStdString();
    if (redMethods.keys().contains(method)) {
        lspServOut << "Redirect filtering:" << method.toStdString();
        QJsonObject result;
        if (redMethods.value(method)(params, result)) {
            QJsonObject protocol;
            protocol[newlsp::K_JSON_RPC] = "2.0";
            protocol[newlsp::K_ID] = id;
            protocol[newlsp::K_RESULT] = result;
        } else {
            lspServErr << std::string("Redirect filtering: ") + "error called logic";
            abort();
        }
    }
    QProcess *proc = newlsp::Route::value(select);
    std::string language = JsonRpcCallProxy::ins().select.language;
    if (proc) {
        QString writeData = newlsp::ServerApplication::toProtocolString(id, method, params);
        lspServOut << "Origin" <<  JsonRpcCallProxy::ins().select.language
                   << "Server:"<< method.toStdString()
                   << "\nwriteData:" << writeData.toStdString();
        proc->write(writeData.toUtf8());
        proc->waitForBytesWritten();
        return;
    } else {
        lspServErr << "can't found lsp" << language << "server to send" << method.toStdString();
    }
}

void JsonRpcCallProxy::notificationFilter(const QString &method, const QJsonObject &params)
{
    lspServOut << __FUNCTION__ << qApp->thread() << QThread::currentThread();
    lspServOut << "notification -->" << method.toStdString();
    if (redNotifications.keys().contains(method)) {
        lspServOut << "Redirect filtering:" << method.toStdString();
        redNotifications.value(method)(params);
        return;
    }
    QProcess *proc = newlsp::Route::value(select);
    std::string language = JsonRpcCallProxy::ins().select.language;
    if (proc) {
        QString writeData = newlsp::ServerApplication::toProtocolString(method, params);
        lspServOut << "Origin" <<  language
                   << "Server:" << method.toStdString()
                   << "\nwriteData:" << writeData.toStdString();
        proc->write(writeData.toUtf8());
        proc->waitForBytesWritten();
        return;
    } else {
        lspServErr << "can't found lsp" << language << "server to send" << method.toStdString();
    }
}

void JsonRpcCallProxy::bindFilter(const QString &methodName, const MethodFilterFunc &func)
{
    redMethods.insert(methodName, func);
}

void JsonRpcCallProxy::bindFilter(const QString &methodName, const NotificationFilterFunc &func)
{
    redNotifications.insert(methodName, func);
}
