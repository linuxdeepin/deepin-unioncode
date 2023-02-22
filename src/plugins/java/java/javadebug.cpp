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
#include "javadebug.h"


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

    if (!checkConfigFile(retMsg))
        return false;

    checkJavaLSPPlugin();

    return true;
}

bool JavaDebug::requestDAPPort(const QString &uuid, const QString &kit,
                               const QString &projectPath,
                               QString &retMsg)
{
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "launch_java_dap");
    QString projectCachePath = CustomPaths::projectCachePath(projectPath);
    msg << uuid
        << kit
        << projectPath
        << d->javaDapPluginConfig.configHomePath
        << d->javaDapPluginConfig.jrePath
        << d->javaDapPluginConfig.jreExecute
        << d->javaDapPluginConfig.launchPackageFile
        << d->javaDapPluginConfig.launchConfigPath
        << d->javaDapPluginConfig.dapPackageFile
        << projectCachePath;

    bool ret = QDBusConnection::sessionBus().send(msg);
    if (!ret) {
        retMsg = tr("Request java dap port failed, please retry.");
        return false;
    }

    return true;
}

bool JavaDebug::checkConfigFile(QString &retMsg)
{
    QString arch = ProcessUtil::localPlatform();
    QString dapSupportFilePath = support_file::DapSupportConfig::globalPath();

    d->javaDapPluginConfig.configHomePath = CustomPaths::user(CustomPaths::Configures) + QDir::separator();
    bool ret = support_file::DapSupportConfig::readFromSupportFile(dapSupportFilePath, arch, d->javaDapPluginConfig, d->javaDapPluginConfig.configHomePath);
    if (!ret) {
        retMsg = tr("Read dapconfig.support failed, please check the file and retry.");
        return false;
    }

    if (d->javaDapPluginConfig.launchPackageName.isEmpty()
            || d->javaDapPluginConfig.dapPackageName.isEmpty()) {
        retMsg = tr("The computer arch is not supported, can not start debugging.");
        return false;
    }

    d->dapPackagePath = env::pkg::native::path() + QDir::separator();

    auto copyPackage = [&](const QString &fileName) {
        if (!QFileInfo(d->javaDapPluginConfig.configHomePath + fileName).isFile()) {
            QFile::copy(d->dapPackagePath + fileName, d->javaDapPluginConfig.configHomePath + fileName);
        }
    };

    copyPackage(d->javaDapPluginConfig.launchPackageName + ".vsix");
    copyPackage(d->javaDapPluginConfig.dapPackageName + ".vsix");

    return true;
}

void JavaDebug::checkJavaLSPPlugin()
{
    QString configFolder = d->javaDapPluginConfig.configHomePath;
    bool bLaunchJarPath = QFileInfo(d->javaDapPluginConfig.launchPackageFile).isFile();
    bool bJrePath = QFileInfo(d->javaDapPluginConfig.jreExecute).isFile();
    if (!bLaunchJarPath || !bJrePath) {
        FileOperation::deleteDir(d->javaDapPluginConfig.launchPackagePath);

        auto decompress = [&](const QString workDir, const QString srcTarget, const QString folder) {
            if (!QFileInfo(workDir).isDir()
                    || !QFileInfo(srcTarget).isFile()
                    || folder.isEmpty()) {
                readyLSPPlugin(false, tr("The lsp package file is null."));
                return;
            }

            QProcess process;
            connect(&process, &QProcess::readyReadStandardError, [&]() {
                QString msg = "Decompress " + srcTarget + " error: " + process.readAllStandardError();
                outProgressMsg(msg);
            });
            connect(&process, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
                    [&](int exitcode, QProcess::ExitStatus exitStatus) {
                if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
                    readyLSPPlugin(true);
                } else {
                    QFile::remove(srcTarget);
                    readyLSPPlugin(false, tr("Decompress lsp package process exited unnormally."));
                }
            });
            process.setWorkingDirectory(workDir);
            process.start("unzip", QStringList{srcTarget, "-d", folder});
            process.waitForFinished();
        };

        QString redhatLspFilePath = d->javaDapPluginConfig.launchPackagePath + ".vsix";
        if (QFileInfo(redhatLspFilePath).isFile()) {
            decompress(configFolder, redhatLspFilePath, d->javaDapPluginConfig.launchPackageName);
        } else {
            readyLSPPlugin(false, tr("The lsp package is not exist."));
        }
    } else {
        readyLSPPlugin(true);
    }
}

void JavaDebug::checkJavaDAPPlugin()
{
    QString configFolder = d->javaDapPluginConfig.configHomePath;
    bool bDapJarPath = QFileInfo(d->javaDapPluginConfig.dapPackageFile).isFile();
    if (!bDapJarPath) {
        FileOperation::deleteDir(configFolder + d->javaDapPluginConfig.dapPackageName);

        auto decompress = [&](const QString workDir, const QString srcTarget, const QString folder) {
            if (!QFileInfo(workDir).isDir()
                    || !QFileInfo(srcTarget).isFile()
                    || folder.isEmpty()) {
                readyDAPPlugin(false, tr("The dap package file is null."));
                return;
            }

            QProcess process;
            connect(&process, &QProcess::readyReadStandardError, [&]() {
                QString msg = "Decompress " + srcTarget + " error: " + process.readAllStandardError();
                outProgressMsg(msg);
            });
            connect(&process, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
                    [&](int exitcode, QProcess::ExitStatus exitStatus) {
                if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
                    readyDAPPlugin(true);
                } else {
                    QFile::remove(srcTarget);
                    readyDAPPlugin(false, tr("Decompress dap package process exited unnormally."));
                }
            });
            process.setWorkingDirectory(workDir);
            process.start("unzip", QStringList{srcTarget, "-d", folder});
            process.waitForFinished();
        };

        QString vsdapFilePath = configFolder + d->javaDapPluginConfig.dapPackageName + ".vsix";
        if (QFileInfo(vsdapFilePath).isFile()) {
            decompress(configFolder, vsdapFilePath, d->javaDapPluginConfig.dapPackageName);
        } else {
            readyDAPPlugin(false, tr("The dap package is not exist."));
        }
    } else {
        readyDAPPlugin(true);
    }
}

void JavaDebug::readyLSPPlugin(bool succeed, const QString &errorMsg)
{
    if (!succeed) {
        debugger.prepareDebugDone(false, errorMsg);
        return;
    }

    checkJavaDAPPlugin();
}

void JavaDebug::readyDAPPlugin(bool succeed, const QString &errorMsg)
{
    if (!succeed) {
        debugger.prepareDebugDone(false, errorMsg);
        return;
    }

    debugger.prepareDebugDone(true, errorMsg);
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

