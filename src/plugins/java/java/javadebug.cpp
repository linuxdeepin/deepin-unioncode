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

bool JavaDebug::requestDAPPort(const QString &uuid, const QString &projectPath, QString &retMsg)
{
    QDBusMessage msg = QDBusMessage::createSignal("/path",
                                                  "com.deepin.unioncode.interface",
                                                  "launch_java_dap");
    msg << uuid
        << projectPath
        << d->javaDapPluginConfig.configHomePath
        << d->javaDapPluginConfig.jrePath
        << d->javaDapPluginConfig.jreExecute
        << d->javaDapPluginConfig.launchPackageFile
        << d->javaDapPluginConfig.launchConfigPath
        << d->javaDapPluginConfig.dapPackageFile;

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
    if ("x86_64" == arch) {
        arch = "linux-x64";
    } else if ("aarch64" == arch) {
        arch = "linux-arm64";
    } else {
        retMsg = tr("The computer arch is not supported, can not start debugging.");
        return false;
    }

    QString dapSupportFilePath = support_file::DapSupportConfig::globalPath();
    bool ret = support_file::DapSupportConfig::readFromSupportFile(dapSupportFilePath, d->javaDapPluginConfig);
    if (!ret) {
        retMsg = tr("Read dapconfig.support failed, please check the file and retry.");
        return false;
    }

    d->javaDapPluginConfig.launchPackageUrl += arch;

    return true;
}

void JavaDebug::checkJavaLSPPlugin()
{
    QString configFolder = QDir::homePath() + d->javaDapPluginConfig.configHomePath;
    bool bLaunchJarPath = QFileInfo(configFolder + d->javaDapPluginConfig.launchPackageFile).isFile();
    bool bJrePath = QFileInfo(configFolder + d->javaDapPluginConfig.jreExecute).isFile();
    if (!bLaunchJarPath || !bJrePath) {
        FileOperation::deleteDir(configFolder + d->javaDapPluginConfig.launchPackageName);

        auto decompress = [&](const QString workDir, const QString srcTarget, const QString folder) {
            if (!QFileInfo(workDir).isDir()
                    || !QFileInfo(srcTarget).isFile()
                    || folder.isEmpty()) {
                readyLSPPlugin(false, tr("The zip file is null."));
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

        QString redhatLspFilePath = configFolder + d->javaDapPluginConfig.launchPackageName + ".zip";
        if (QFileInfo(redhatLspFilePath).isFile()) {
            decompress(configFolder, redhatLspFilePath, d->javaDapPluginConfig.launchPackageName);
        } else {
            DownloadUtil *downloadUtil = new DownloadUtil(d->javaDapPluginConfig.launchPackageUrl,
                                                          configFolder,
                                                          d->javaDapPluginConfig.launchPackageName + ".zip");
            connect(downloadUtil, &DownloadUtil::sigProgress, [=](qint64 bytesRead, qint64 totalBytes){
                QString progress = QString(tr("LSP package received: %1 MB, total: %2 MB, percent: %%3"))
                        .arg(bytesRead/1024.00/1024.00).arg(totalBytes/1024.00/1024.00).arg(bytesRead*100.00/totalBytes);
                outProgressMsg(progress);
            });
            connect(downloadUtil, &DownloadUtil::sigFinished, [=](){
                outProgressMsg(tr("Download java lsp plugin finished."));
                decompress(configFolder, redhatLspFilePath, d->javaDapPluginConfig.launchPackageName);
            });

            connect(downloadUtil, &DownloadUtil::sigFailed, [=](){
                readyLSPPlugin(false, tr("Download java lsp plugin failed, please retry!"));
            });

            outProgressMsg(tr("Downloading java lsp plugin, please waiting..."));
            bool ret = downloadUtil->start();
            if (!ret) {
                readyLSPPlugin(false, tr("Download java lsp plugin failed, please retry!"));
            }
        }
    } else {
        readyLSPPlugin(true);
    }
}

void JavaDebug::checkJavaDAPPlugin()
{
    QString configFolder = QDir::homePath() + d->javaDapPluginConfig.configHomePath;
    bool bDapJarPath = QFileInfo(configFolder + d->javaDapPluginConfig.dapPackageFile).isFile();
    if (!bDapJarPath) {
        FileOperation::deleteDir(configFolder + d->javaDapPluginConfig.dapPackageName);

        auto decompress = [&](const QString workDir, const QString srcTarget, const QString folder) {
            if (!QFileInfo(workDir).isDir()
                    || !QFileInfo(srcTarget).isFile()
                    || folder.isEmpty()) {
                readyDAPPlugin(false, tr("The zip file is null."));
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

        QString vsdapFilePath = configFolder + d->javaDapPluginConfig.dapPackageName + ".zip";
        if (QFileInfo(vsdapFilePath).isFile()) {
            decompress(configFolder, vsdapFilePath, d->javaDapPluginConfig.dapPackageName);
        } else {
            DownloadUtil *downloadUtil = new DownloadUtil(d->javaDapPluginConfig.dapPackageUrl,
                                                          configFolder,
                                                          d->javaDapPluginConfig.dapPackageName + ".zip");
            connect(downloadUtil, &DownloadUtil::sigProgress, [=](qint64 bytesRead, qint64 totalBytes){
                QString progress = QString(tr("DAP package received: %1 MB, total: %2 MB, percent: %%3"))
                        .arg(bytesRead/1024.00/1024.00).arg(totalBytes/1024.00/1024.00).arg(bytesRead*100.00/totalBytes);
                outProgressMsg(progress);
            });

            connect(downloadUtil, &DownloadUtil::sigFinished, [=](){
                outProgressMsg(tr("Download java dap plugin finished."));
                decompress(configFolder, vsdapFilePath, "vscjava");
            });

            connect(downloadUtil, &DownloadUtil::sigFailed, [=](){
                readyDAPPlugin(false, tr("Download java dap plugin failed, please retry!"));
            });

            outProgressMsg(tr("Downloading java dap plugin, please waiting..."));
            bool ret = downloadUtil->start();
            if (!ret) {
                readyDAPPlugin(false, tr("Download java dap plugin failed, please retry!"));
            }
        }
    } else {
        readyDAPPlugin(true);
    }
}


void JavaDebug::readyLSPPlugin(bool succeed, const QString &errorMsg)
{
    if (!succeed) {
        debugger.prepareDebugDone({false, errorMsg});
        return;
    }

    checkJavaDAPPlugin();
}

void JavaDebug::readyDAPPlugin(bool succeed, const QString &errorMsg)
{
    if (!succeed) {
        debugger.prepareDebugDone({false, errorMsg});
        return;
    }

    debugger.prepareDebugDone({true, errorMsg});
}

void JavaDebug::outProgressMsg(const QString &msg)
{
    debugger.prepareDebugProgress({msg});
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
    QString javaExec = QDir::homePath() + d->javaDapPluginConfig.configHomePath + d->javaDapPluginConfig.jreExecute;
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

