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
#include "jsonrpccallproxy.h"

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
