// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pythondebug.h"

#include "project/properties/configutil.h"
#include "services/option/optionmanager.h"
#include "common/util/custompaths.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QUuid>
#include <QProcess>

class PythonDebugPrivate
{
    friend class PythonDebug;
    QString interpreterPath;
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

    d->interpreterPath = config::ConfigUtil::instance()->getConfigureParamPointer()->pythonVersion.path;
    if (d->interpreterPath.isEmpty()) //project has not set interpreter to config. use default interpreter
        d->interpreterPath = OptionManager::getInstance()->getPythonToolPath();
    if (!d->interpreterPath.contains("python3")) {
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


bool PythonDebug::requestDAPPort(const QString &uuid, const QString &kit,
                                 const QString &projectPath,
                                 const QString &fileName,
                                 QString &retMsg)
{
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "launch_python_dap");
    QString projectCachePath = CustomPaths::projectCachePath(projectPath);
    msg << uuid
        << kit
        << d->interpreterPath
        << fileName
        << projectPath
        << projectCachePath;

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

