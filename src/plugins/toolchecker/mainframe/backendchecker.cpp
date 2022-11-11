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
#include "backendchecker.h"
#include "pip3dialog.h"
#include "wgetdialog.h"

#include "common/common.h"

#include <QDirIterator>
#include <QMimeDatabase>
#include <QDebug>

using FO = FileOperation;

namespace  {
bool checkClangdFlag = false;
bool checkJdtlsFlag = false;
bool checkPylsFlag = false;
}

QString BackendChecker::localPlatform()
{
    // get location platform
    QString platform = "";
    bool platfromQueRes = ProcessUtil::execute("arch", {}, [&](const QByteArray &data){
        platform = QString(data).replace("\n","");
    });
    if (!platfromQueRes)
        qCritical() << "usr command arch failed, please check tool program arch";
    else if (platform.isEmpty())
        qCritical() << "query local platform failed, not support \"arch\" command?";
    return platform;
}

void BackendChecker::doCheckClangd(const QString &language)
{
    if (checkClangdFlag)
        return;

    if (!checkClangdFlag)
        checkClangdFlag = true;

    QString user = "FunningC0217";
    QString origin = "clangd-archive";
    QString branch = "dev";
    QString rawPrefix = "https://raw.githubusercontent.com";
    QString platformSupportFileName = "platform.support";
    QString clangdSha256FileName = "clangd.sha256";
    QString clangdFileName = "clangd";

    QUrl remotePlatformSupportUrl(rawPrefix + "/" + user + "/" + origin + "/"
                                  + branch + "/" + platformSupportFileName);
    QString currentPlatform = localPlatform();
    // get is support platform
    QStringList platformSupports;
    for (auto one : getRemoteFile(remotePlatformSupportUrl).split("\n")) {
        if (!one.isEmpty()) {
            platformSupports.append(one);
        }
    }
    if (!platformSupports.contains(currentPlatform)) {
        qCritical() << "get remote platform support error"
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
        dialog.setWorkDirectory(CustomPaths::lspRuntimePath(language));
        dialog.setWgetArguments(args);
        dialog.exec();
        QFile::Permissions permission = QFile::Permission::ReadUser
                | QFile::Permission::WriteUser
                | QFile::Permission::ExeUser;
        QFile(localClangdPath).setPermissions(permission);
    }
}

void BackendChecker::doCheckJdtls(const QString &language)
{
    if (checkJdtlsFlag)
        return;

    if (!checkJdtlsFlag)
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
        dialog.setWorkDirectory(CustomPaths::lspRuntimePath(language));
        dialog.setWgetArguments(args);
        dialog.exec();

        ProcessDialog processDialog;
        processDialog.setWorkDirectory(CustomPaths::lspRuntimePath(language));
        processDialog.setProgram("tar");
        processDialog.setArguments({"zxvf", localJdtlsPath, "-C", "."});
        processDialog.exec();
    }
}

void BackendChecker::doCheckPyls(const QString &language)
{
    Q_UNUSED(language)

    if (checkPylsFlag)
        return;

    if (!checkPylsFlag)
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

BackendChecker::BackendChecker(QWidget *parent)
    : QWidget(parent)
{

}

BackendChecker &BackendChecker::instance()
{
    static BackendChecker ins;
    return ins;
}

void BackendChecker::checkLanguageBackend(const QString &language)
{
    if (language == "C/C++") {
        doCheckClangd(language);
    } else if (language == "Java") {
        doCheckJdtls(language);
    } else if (language == "Python") {
        doCheckPyls(language);
    } else {
    }
}

bool BackendChecker::checkShasum(const QString &filePath, const QString &src_code, const QString &mode)
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

QString BackendChecker::getRemoteFile(const QUrl &url)
{
    QString ret;
    while (ret.isEmpty()) {
        QProcess curlProc;
        curlProc.setProgram("curl");
        curlProc.setArguments({url.toEncoded()});
        curlProc.start();
        qInfo() << curlProc.program() << curlProc.arguments();
        curlProc.waitForFinished(1500);

        if (curlProc.exitCode() == 0)
            ret = curlProc.readAll();
    }

    if (ret.endsWith("\n"))
        ret.remove(ret.size() - 1, 1);
    return ret;
}

bool BackendChecker::saveRemoteFile(const QUrl &url, const QString &saveFilePath)
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

