// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjadebug.h"

#include "services/option/optionmanager.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QUuid>

class NinjaDebugPrivate
{
    friend class NinjaDebug;
};

NinjaDebug::NinjaDebug(QObject *parent)
    : QObject(parent)
    , d(new NinjaDebugPrivate())
{

}

NinjaDebug::~NinjaDebug()
{
    if (d)
        delete d;
}

bool NinjaDebug::requestDAPPort(const QString &uuid, const QString &kit,
                                const QString &targetPath, const QStringList &arguments,
                                QString &retMsg)
{
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "getDebugPort");

    msg << uuid
        << kit
        << targetPath
        << arguments;
    bool ret = QDBusConnection::sessionBus().send(msg);
    if (!ret) {
        retMsg = tr("Request cxx dap port failed, please retry.");
        return false;
    }

    return true;
}

bool NinjaDebug::isLaunchNotAttach()
{
    return false;
}

dap::LaunchRequest NinjaDebug::launchDAP(const QString &targetPath, const QStringList &argments)
{
    dap::LaunchRequest request;
    request.name = "(gdb) Launch";
    request.type = "cppdbg";
    request.request = "launch";
    request.program = targetPath.toStdString();
    request.stopAtEntry = false;
    dap::array<dap::string> arrayArg;
    foreach (QString arg, argments) {
        arrayArg.push_back(arg.toStdString());
    }
    request.args = arrayArg;
    request.externalConsole = false;
    request.MIMode = "gdb";
    request.__sessionId = QUuid::createUuid().toString().toStdString();

    return request;
}

