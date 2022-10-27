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
#include "pythondebug.h"

#include "services/option/optionmanager.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QUuid>
#include <QProcess>

class PythonDebugPrivate
{
    friend class PythonDebug;
};

PythonDebug::PythonDebug(QObject *parent)
    : QObject(parent)
    , d(new PythonDebugPrivate())
{

}

PythonDebug::~PythonDebug()
{
    if (d)
        delete d;
}

bool PythonDebug::prepareDebug(const QString &fileName, QString &retMsg)
{
    if (fileName.isEmpty()) {
        retMsg = tr("There is no opened python file, please open.");
        return false;
    }

    QString pythonTool = OptionManager::getInstance()->getPythonToolPath();
    if (!pythonTool.contains("python3")) {
        retMsg = tr("The python3 is needed, please select it in options dialog or install it.");
        return false;
    }

    QProcess process;
    QStringList options;
    options << "-c" << "pip3 show -- debugpy";
    process.start("/bin/bash", options);
    if (process.waitForReadyRead()) {
        QString output = process.readAllStandardOutput();
        if (output.contains("debugpy"))
            return true;
    }

    retMsg = tr("The debugpy is needed, please use command \"pip3 install debugpy\" install and retry.");
    return false;
}


bool PythonDebug::requestDAPPort(const QString &uuid, const QString &fileName, QString &retMsg)
{
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "launch_python_dap");
    msg << uuid
        << OptionManager::getInstance()->getPythonToolPath()
        << fileName;

    bool ret = QDBusConnection::sessionBus().send(msg);
    if (!ret) {
        retMsg = tr("Request python dap port failed, please retry.");
        return false;
    }

    return true;
}

bool PythonDebug::isLaunchNotAttach()
{
    return false;
}

dap::AttachRequest PythonDebug::attachDAP(int port, const QString &workspace)
{
    dap::AttachRequest attachRequest;
    attachRequest.name = "Python Debug";
    attachRequest.type = "python";
    attachRequest.request = "attach";
    dap::object obj;
    obj["port"] = dap::number(port);
    attachRequest.connect = obj;
    attachRequest.justMyCode = true;
    attachRequest.logToFile = true;
    attachRequest.__configurationTarget = 6;

    dap::array<dap::string> op;
    op.push_back("RedirectOutput");
    op.push_back("UnixClient");
    op.push_back("ShowReturnValue");
    attachRequest.debugOptions = op;
    attachRequest.showReturnValue = true;

    attachRequest.workspaceFolder = workspace.toStdString();
    attachRequest.__sessionId = QUuid::createUuid().toString().toStdString();

    return attachRequest;
}

bool PythonDebug::isRestartDAPManually()
{
    return true;
}

bool PythonDebug::isStopDAPManually()
{
    return true;
}

