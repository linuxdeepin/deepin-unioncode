// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pythondebug.h"
#include "utils/utils.h"
#include "installer/pipinstaller.h"

#include "project/properties/configutil.h"
#include "services/option/optionmanager.h"
#include "services/window/windowservice.h"
#include "common/util/custompaths.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QUuid>
#include <QProcess>

using namespace dpfservice;

constexpr char kCancelActId[] { "cancel" };
constexpr char kInstallActId[] { "install_default" };

class PythonDebugPrivate
{
    friend class PythonDebug;
    QString interpreterPath;
};

PythonDebug::PythonDebug(QObject *parent)
    : QObject(parent), d(new PythonDebugPrivate())
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
        metaObject()->invokeMethod(this, "notifyMessage",
                                   Qt::QueuedConnection,
                                   Q_ARG(QString, tr("There is no opened python file, please open.")));
        return false;
    }

    d->interpreterPath = config::ConfigUtil::instance()->getConfigureParamPointer()->pythonVersion.path;
    if (d->interpreterPath.isEmpty()) //project has not set interpreter to config. use default interpreter
        d->interpreterPath = OptionManager::getInstance()->getPythonToolPath();
    if (d->interpreterPath.isEmpty()) {
        metaObject()->invokeMethod(this, "notifyMessage",
                                   Qt::QueuedConnection,
                                   Q_ARG(QString, tr("An interpreter is necessary. Please select it in options dialog or install it.")));
        return false;
    }

    PIPInstaller installer;
    if (!installer.checkInstalled(d->interpreterPath, "debugpy")) {
        metaObject()->invokeMethod(this, &PythonDebug::notifyToInstall);
        return false;
    }

    return true;
}

bool PythonDebug::requestDAPPort(const QString &ppid, const QString &kit,
                                 const QString &projectPath,
                                 const QString &fileName,
                                 QString &retMsg)
{
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "launch_python_dap");
    QString projectCachePath = CustomPaths::projectCachePath(projectPath);
    msg << ppid
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

void PythonDebug::notifyMessage(const QString &msg)
{
    auto winSrv = dpfGetService(WindowService);
    if (!winSrv)
        return;

    winSrv->notify(2, "Python", msg, {});
}

void PythonDebug::notifyToInstall()
{
    auto winSrv = dpfGetService(WindowService);
    if (!winSrv)
        return;

    auto handleInstall = [this](const QString &id) {
        if (id == kInstallActId) {
            PIPInstaller installer;
            InstallInfo info { "", "python", { "debugpy" } };
            installer.install(d->interpreterPath, info);
        }
    };

    QStringList acts { kCancelActId, tr("Cancel"),
                       kInstallActId, tr("Install") };
    winSrv->notifyWithCallback(2, "Python",
                               tr("You need the corresponding version of the debugger. Please install it and try again."),
                               acts, handleInstall);
}
