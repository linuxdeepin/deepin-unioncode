// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakedebug.h"
#include "services/project/projectservice.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QUuid>

using namespace dpfservice;
class CMakeDebugPrivate
{
    friend class CMakeDebug;
};

CMakeDebug::CMakeDebug(QObject *parent)
    : QObject(parent)
    , d(new CMakeDebugPrivate())
{

}

CMakeDebug::~CMakeDebug()
{
    if (d)
        delete d;
}

bool CMakeDebug::prepareDebug(QString &retMsg)
{
    auto prjInfo = dpfGetService(ProjectService)->getActiveProjectInfo();
    QString debuggerTool = prjInfo.debugProgram();
    if (!debuggerTool.contains("gdb")) {
        retMsg = tr("The gdb is required, please install it in console with \"sudo apt install gdb\", "
                    "and then restart the tool, reselect the CMake Debugger in Options Dialog...");
        return false;
    }

    return true;
}

bool CMakeDebug::requestDAPPort(const QString &uuid, const QString &kit,
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

bool CMakeDebug::isLaunchNotAttach()
{
    return false;
}

dap::LaunchRequest CMakeDebug::launchDAP(const QString &targetPath, const QStringList &argments)
{
    auto prjService = dpfGetService(ProjectService);
    dap::array<dap::string> env;
    if (prjService) {
        ProjectInfo prjInfo = prjService->getActiveProjectInfo();
        for (QString envItem : prjInfo.runEnvironment())
            env.push_back(envItem.toStdString());
    }

    dap::LaunchRequest request;
    request.name = "(gdb) Launch";
    request.type = "cppdbg";
    request.request = "launch";
    request.program = targetPath.toStdString();
    request.stopAtEntry = false;
    request.environment = env;
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

