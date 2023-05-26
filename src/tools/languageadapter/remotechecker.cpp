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
#include "remotechecker.h"
#include "common/common.h"

#include <QDirIterator>
#include <QMimeDatabase>
#include <QDebug>

using FO = FileOperation;

void RemoteChecker::doCheckClangd(const QString &language)
{
    if (checkClangdFlag)
        return;

    checkClangdFlag = true;

    QString user = "deepin";
    QString origin = "clangd-archive";
    QString branch = "dev";
    QString rawPrefix = "https://raw.githubusercontent.com";
    QString platformSupportFileName = "platform.support";
    QString clangdSha256FileName = "clangd.sha256";
    QString clangdFileName = "clangd";

    QUrl remotePlatformSupportUrl(rawPrefix + "/" + user + "/" + origin + "/"
                                  + branch + "/" + platformSupportFileName);
    QString currentPlatform = ProcessUtil::localPlatform();
    // get is support platform
    QStringList platformSupports;
    for (auto one : getRemoteFile(remotePlatformSupportUrl).split("\n")) {
        if (!one.isEmpty()) {
            platformSupports.append(one);
        }
    }
    if (!platformSupports.contains(currentPlatform)) {
        lspServErr << "get remote platform support error"
                   << ", remote:" << platformSupports
                   << ", local:" << currentPlatform;
        return;
    }

    QUrl remoteClangdShasum256Url(rawPrefix + "/" + user + "/" + origin + "/" + branch
                                  + "/" + currentPlatform + "/" + clangdSha256FileName);
    // local lsp shasum256 file
    QString localClangdShasum256Path = CustomPaths::lspRuntimePath(language) + QDir::separator() + clangdSha256FileName;
    if (!FO::exists(localClangdShasum256Path)) { // not local shasum256 file
        // save remote shasum256
        saveRemoteFile(remoteClangdShasum256Url, localClangdShasum256Path);
    } else { // exist shasum256 file
        QString remoteClangdShasum256Data = getRemoteFile(remoteClangdShasum256Url);
        if (!remoteClangdShasum256Data.isEmpty()) {
            if (FO::readAll(localClangdShasum256Path) != remoteClangdShasum256Data) {
                // save remote shasum256
                saveRemoteFile(remoteClangdShasum256Url, localClangdShasum256Path);
            }
        }
    }

    // local lsp cxx backend program
    QString localClangdPath = CustomPaths::lspRuntimePath(language) + QDir::separator() + clangdFileName;
    bool localClangdKeep = true;
    if (FO::exists(localClangdPath)) {
        if (FO::exists(localClangdShasum256Path)) {
            localClangdKeep = checkShasum(localClangdPath, FO::readAll(localClangdShasum256Path), "256");
        } else { // not exists clangd program
            localClangdKeep = false;
        }
    } else {
        localClangdKeep = false;
    }

    if (!localClangdKeep) {
        FO::doRemove(localClangdPath);
        QString downloadPrefix = "https://github.com";
        QUrl remoteClangdUrl(downloadPrefix + "/" + user + "/" + origin
                             + "/raw/dev/" + currentPlatform + "/" + clangdFileName);
        QStringList args = { remoteClangdUrl.toEncoded(), "-O", clangdFileName };
        WGetDialog dialog;
        dialog.setWorkingDirectory(CustomPaths::lspRuntimePath(language));
        dialog.setWgetArguments(args);
        dialog.exec();
        QFile::Permissions permission = QFile::Permission::ReadUser
                | QFile::Permission::WriteUser
                | QFile::Permission::ExeUser;
        QFile(localClangdPath).setPermissions(permission);
    }
}

void RemoteChecker::doCheckJdtls(const QString &language)
{
    if (checkJdtlsFlag)
        return;

    checkJdtlsFlag = true;

    QUrl remoteJdtlsUrl("https://download.eclipse.org/jdtls/snapshots/jdt-language-server-1.11.0-202205051421.tar.gz");
    QString localJdtlsPath = CustomPaths::lspRuntimePath(language) + QDir::separator() + "jdt-language-server.tar.gz";
    QUrl remoteJdtlsShasum256Url("https://download.eclipse.org/jdtls/snapshots/jdt-language-server-1.11.0-202205051421.tar.gz.sha256");
    QString localJdtlsShasum256Path = CustomPaths::lspRuntimePath(language) + QDir::separator() + "jdt-language-server.tar.gz.sha256";

    // local lsp shasum256 file
    if (!FO::exists(localJdtlsShasum256Path)) { // not local shasum256 file
        // save remote shasum256
        saveRemoteFile(remoteJdtlsShasum256Url, localJdtlsShasum256Path);
    } else { // exist shasum256 file
        QString remoteClangdShasum256Data = getRemoteFile(remoteJdtlsShasum256Url);
        if (!remoteClangdShasum256Data.isEmpty()) {
            if (FO::readAll(localJdtlsShasum256Path) != remoteClangdShasum256Data) {
                // save remote shasum256
                saveRemoteFile(remoteJdtlsShasum256Url, localJdtlsShasum256Path);
            }
        }
    }

    // local lsp cxx backend program
    bool localKeep = true;
    if (FO::exists(localJdtlsPath)) {
        if (FO::exists(localJdtlsShasum256Path)) {
            localKeep = checkShasum(localJdtlsPath, FO::readAll(localJdtlsShasum256Path), "256");
        } else { // not exists clangd program
            localKeep = false;
        }
    } else {
        localKeep = false;
    }

    if (!localKeep) {
        FO::doRemove(localJdtlsPath);

        QStringList args = { remoteJdtlsUrl.toEncoded(), "-O", localJdtlsPath };
        WGetDialog dialog;
        dialog.setWorkingDirectory(CustomPaths::lspRuntimePath(language));
        dialog.setWgetArguments(args);
        dialog.exec();

        ProcessDialog processDialog;
        processDialog.setWorkingDirectory(CustomPaths::lspRuntimePath(language));
        processDialog.setProgram("tar");
        processDialog.setArguments({"zxvf", localJdtlsPath, "-C", "."});
        processDialog.exec();
    }
}

void RemoteChecker::doCheckPyls(const QString &language)
{
    Q_UNUSED(language)

    if (checkPylsFlag)
        return;

    checkPylsFlag = true;

    // virtualenv not support, use user env
    QString pip3PackageName {"python-language-server[all]"};
    QString executeProgram {"pyls"};

    QString userLocalBinPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            + QDir::separator() + ".local" + QDir::separator() + "bin";
    if (!QFileInfo(userLocalBinPath + QDir::separator() + executeProgram).exists()) {
        Pip3Dialog pip3Dialog;
        pip3Dialog.install("python-language-server[all]");
        pip3Dialog.exec();
    }
}

void RemoteChecker::checkJSServer(const QString &checkPath)
{
    if (checkJSServerFlag)
        return;

    checkJSServerFlag = true;

    QString workingDirectory = checkPath;
    QString nodePath = workingDirectory + "/node_modules/node/bin/node";
    if (QFileInfo::exists(nodePath))
        return;

    QProcess process;
    process.setWorkingDirectory(workingDirectory);

    auto runCommand = [&](const QString &program, const QStringList &args)
    {
        process.start(program, args);
        process.waitForFinished();
        QString error = process.readAllStandardError();
        if (!error.isEmpty())
            qCritical() << QString("run %1 error:").arg(program) <<  error;
    };

    runCommand("npm", {"install", "n"});
    runCommand("npm", {"install", "typescript", "typescript-language-server"});

    process.setWorkingDirectory(workingDirectory + "/node_modules/.bin");
    process.setEnvironment({QString("N_PREFIX=%1/node_modules/node").arg(workingDirectory)});
    runCommand("./n", {"stable"});
}

RemoteChecker::RemoteChecker()
{

}

RemoteChecker &RemoteChecker::instance()
{
    static RemoteChecker ins;
    return ins;
}

void RemoteChecker::checkLanguageBackend(const QString &language)
{
    if (language == "C/C++") {
        doCheckClangd(language);
    } else if (language == "Java") {
        doCheckJdtls(language);
    } else if (language == "Python") {
        doCheckPyls(language);
    }
}

bool RemoteChecker::checkShasum(const QString &filePath, const QString &src_code, const QString &mode)
{
    QProcess checkProcess;
    checkProcess.setProgram("shasum");
    checkProcess.setArguments({"-a", mode, filePath});
    checkProcess.start();
    checkProcess.waitForFinished();

    QString output = checkProcess.readAll();
    QStringList result = output.split(" ");
    if (result.size() >= 2) {
        output = result.first();
    }
    return src_code == output;
}

QString RemoteChecker::getRemoteFile(const QUrl &url)
{
    QString ret;
    while (ret.isEmpty()) {
        QProcess curlProc;
        curlProc.setProgram("curl");
        curlProc.setArguments({url.toEncoded()});
        curlProc.start();
        lspServOut << curlProc.program().toStdString();
        for (auto args : curlProc.arguments()) {
            lspServOut << "," << args.toStdString();
        }
        curlProc.waitForFinished(1500);

        if (curlProc.exitCode() == 0)
            ret = curlProc.readAll();
    }

    if (ret.endsWith("\n"))
        ret.remove(ret.size() - 1, 1);
    return ret;
}

bool RemoteChecker::saveRemoteFile(const QUrl &url, const QString &saveFilePath)
{
    QString data = getRemoteFile(url);
    if (data.isEmpty()) {
        return false;
    }

    QFile file(saveFilePath);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }
    file.write(data.toLatin1());
    file.close();
    return true;
}

