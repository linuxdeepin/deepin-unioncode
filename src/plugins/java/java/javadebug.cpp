// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "javadebug.h"

#include "services/project/projectservice.h"
#include "common/supportfile/dapconfig.h"
#include "common/util/downloadutil.h"
#include "common/util/fileoperation.h"
#include "common/util/eventdefinitions.h"
#include "common/util/processutil.h"
#include "common/util/custompaths.h"
#include "common/util/environment.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QUuid>
#include <QDir>
#include <QProcess>
#include <QDebug>

using namespace dpfservice;
class JavaDebugPrivate
{
    friend class JavaDebug;

    support_file::JavaDapPluginConfig javaDapPluginConfig;
    QString dapPackagePath;
};

JavaDebug::JavaDebug(QObject *parent)
    : QObject(parent)
    , d(new JavaDebugPrivate())
{

}

JavaDebug::~JavaDebug()
{
    if(d)
        delete d;
}

bool JavaDebug::isAnsyPrepareDebug()
{
    return true;
}

bool JavaDebug::prepareDebug(const QString &projectPath, QString &retMsg)
{
    if (!QFileInfo(projectPath).exists()) {
        retMsg = tr("The project is not exist, please check the files and retry.");
        return false;
    }
    debugger.prepareDebugDone(true, retMsg);

    return true;
}

bool JavaDebug::requestDAPPort(const QString &uuid, const QString &kit,
                               const QString &projectPath,
                               QString &retMsg)
{
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "launch_java_dap");


    auto activeProjectInfo = dpfGetService(ProjectService)->getActiveProjectInfo();


    QString projectCachePath = CustomPaths::projectCachePath(projectPath);
    msg << uuid
        << kit
        << projectPath
        << CustomPaths::user(CustomPaths::Configures) + QDir::separator()
        << activeProjectInfo.property("jrePath").toString()
        << activeProjectInfo.property("jreExecute").toString()
        << activeProjectInfo.property("launchPackageFile").toString()
        << activeProjectInfo.property("launchConfigPath").toString()
        << activeProjectInfo.property("dapPackageFile").toString()
        << projectCachePath;

    bool ret = QDBusConnection::sessionBus().send(msg);
    if (!ret) {
        retMsg = tr("Request java dap port failed, please retry.");
        return false;
    }

    return true;
}

void JavaDebug::outProgressMsg(const QString &msg)
{
    debugger.prepareDebugProgress(msg);
}

bool JavaDebug::isLaunchNotAttach()
{
    return true;
}

dap::LaunchRequest JavaDebug::launchDAP(const QString &workspace,
                                        const QString &mainClass,
                                        const QString &projectName,
                                        const QStringList &classPaths)
{
    dap::LaunchRequest request;
    request.name = "Java Debug";
    request.type = "java";
    request.request = "launch";
    request.cwd = workspace.toStdString();
    request.console = "integratedTerminal";
    request.internalConsoleOptions = "neverOpen";
    request.mainClass = mainClass.toStdString();
    request.projectName = projectName.toStdString();
    dap::array<dap::string> dapClassPaths;
    foreach (auto classpath, classPaths) {
        dapClassPaths.push_back(classpath.toStdString());
    }
    request.classPaths = dapClassPaths;
    QString javaExec = d->javaDapPluginConfig.jreExecute;
    request.javaExec = javaExec.toStdString();
    request.shortenCommandLine = "none";
    request.__sessionId = QUuid::createUuid().toString().toStdString();

    return request;
}

bool JavaDebug::isRestartDAPManually()
{
    return true;
}

bool JavaDebug::isStopDAPManually()
{
    return true;
}

