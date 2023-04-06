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

