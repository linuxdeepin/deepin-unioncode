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
#include "wgetdialog.h"

#include "common/common.h"

#include <QDirIterator>
#include <QMimeDatabase>

#define JDTLS_PACKAGE_URL "https://download.eclipse.org/jdtls/snapshots/jdt-language-server-1.10.0-202203040350.tar.gz"
#define JDTLS_PACKAGE_NAME "jdt-language-server.tar.gz"
#define JDTLS_CHECKFILE_URL "https://download.eclipse.org/jdtls/snapshots/jdt-language-server-1.10.0-202203040350.tar.gz.sha256"
#define JDTLS_CHECKFILE_NAME "jdt-language-server.tar.gz.sha256"
#define JDTLS_CHECKPROGRAM_NAME "shasum"
#define JDTLS_CHECKPROGRAM_MODE "256"

#define JDTLS_PROGRAM_MAIN "jdtls"
#define JDTLS_PROGRAM_MAIN_MIME "text/x-python3"

#define JDTLS_PROGRAM_REMAIN "jdtls_run.sh"
#define JDTLS_PROGRAM_REMAIN_MIME "application/x-shellscript"

BackendChecker::BackendChecker(QWidget *parent)
    : QWidget(parent)
{
    RequestInfo info;
    info.setPackageUrl(QUrl(JDTLS_PACKAGE_URL));
    info.setPackageSaveName(JDTLS_PACKAGE_NAME);
    info.setCheckFileUrl(QUrl(JDTLS_CHECKFILE_URL));
    info.setCheckFileSaveName(JDTLS_CHECKFILE_NAME);
    info.setCheckNumProgram(JDTLS_CHECKPROGRAM_NAME);
    info.setCheckNumMode(JDTLS_CHECKPROGRAM_MODE);

    requestInfos["Java"] = info;

    QDir dir = QDir::home();
    if (!dir.cd(".config")) { dir.mkdir(".config"); }
    if (!dir.cd("languageadapter")) { dir.mkdir("languageadapter"); }
    adapterPath = dir.path();

    auto itera = requestInfos.begin();
    while (itera != requestInfos.end()) {
        if (!dir.cd(itera.key())) {
            dir.mkdir(itera.key());
        }
        dir.cdUp();
        itera ++;
    }

    for (auto languageID : requestInfos.keys()) {
        auto info = requestInfos.value(languageID);
        if (!existRunMain(languageID)) { // install
            if (!checkCachePackage(languageID)) { // Sha256 check
                QStringList args = { info.getPackageUrl().toEncoded(),
                                     "-O",
                                     info.getPackageSaveName() };
                auto dialog =  new WGetDialog;
                dialog->setWorkDirectory(adapterPath);
                dialog->setWgetArguments(args);
                dialog->exec();
            }

            auto processDialog = new ProcessDialog();
            processDialog->setWorkDirectory(adapterPath);
            processDialog->setProgram("tar");
            processDialog->setArguments({"zxvf", info.getPackageSaveName(), "-C", languageID});
            processDialog->exec();
        }
        if (!existShellRemain(languageID)) {
            createShellRemain(languageID);
        }
    };
}

bool BackendChecker::existShellRemain(const QString &languageID)
{
    if (languageID == "Java") {
        QDir dir(adapterPath + QDir::separator() + languageID);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files | QDir::Dirs);
        QDirIterator itera(dir, QDirIterator::Subdirectories);
        QMimeDatabase mimeDB;
        while (itera.hasNext()) {
            itera.next();
            QString mimeName = mimeDB.mimeTypeForFile(itera.fileInfo()).name();
            if (JDTLS_PROGRAM_REMAIN_MIME == mimeName
                    && JDTLS_PROGRAM_REMAIN == itera.fileName()) {
                return true;
            }
        }
    }
    return false;
}

bool BackendChecker::createShellRemain(const QString &languageID)
{
    if (languageID == "Java") {
        QString jdtlsRemainPath = CustomPaths::global(CustomPaths::Scripts)
                + QDir::separator() +JDTLS_PROGRAM_REMAIN;
        QString languageChildPath = adapterPath + QDir::separator() + languageID;
        if (!QFileInfo(jdtlsRemainPath).exists()) {
            ContextDialog::ok(QDialog::tr("Failed, global Java env lsp "
                                          "remain shell file not exists"));
            qCritical() << "jdtlsRemainPath need jdtls_run.sh file from script path";
            abort();
        }
        ProcessUtil::execute("cp", {jdtlsRemainPath, languageChildPath}, [](auto data){
            qInfo() << data;
        });
        return true;
    }
    return false;
}

bool BackendChecker::existRunMain(const QString &languageID)
{
    if (languageID == "Java") {
        QDir dir(adapterPath + QDir::separator() + languageID);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files | QDir::Dirs);
        QDirIterator itera(dir, QDirIterator::Subdirectories);
        QMimeDatabase mimeDB;
        while (itera.hasNext()) {
            itera.next();
            QString mimeName = mimeDB.mimeTypeForFile(itera.fileInfo()).name();
            if (JDTLS_PROGRAM_MAIN_MIME == mimeName
                    && JDTLS_PROGRAM_MAIN == itera.fileName()) {
                return true;
            }
        }
    }
    return false;
}

bool BackendChecker::checkCachePackage(const QString &languageID)
{
    auto info = requestInfos.value(languageID);

    QDir adapterDir(adapterPath);
    if (!adapterDir.exists(info.getPackageSaveName()))
        return false;

    if (adapterDir.exists(info.getCheckFileSaveName())) {
        adapterDir.remove(info.getCheckFileSaveName());
    }

    // current cache package path
    QString localPackageName = info.getPackageSaveName();

    // download sha256
    QStringList args = { info.getCheckFileUrl().toEncoded(),
                         "-O",
                         info.getCheckFileSaveName() };

    QProcess wgetCheckFileProcess;
    wgetCheckFileProcess.setWorkingDirectory(adapterPath);
    wgetCheckFileProcess.setProgram("wget");
    wgetCheckFileProcess.setArguments({ info.getCheckFileUrl().toEncoded(),
                                        "-O",
                                        info.getCheckFileSaveName()});
    wgetCheckFileProcess.start();
    wgetCheckFileProcess.waitForFinished();

    QProcess checkProcess;
    checkProcess.setWorkingDirectory(adapterPath);
    checkProcess.setProgram(info.getCheckNumProgram());
    checkProcess.setArguments({"-a", info.getCheckNumMode(), info.getPackageSaveName()});
    checkProcess.start();
    checkProcess.waitForFinished();

    QString output = checkProcess.readAll();
    QStringList result = output.split(" ");
    if (result.size() >= 2) {
        output = result.first();
    }

    QFile checkFile(adapterDir.filePath(info.getCheckFileSaveName()));
    if (checkFile.open(QFile::OpenModeFlag::ReadOnly)) {
        QString readOut = checkFile.readAll();
        if (readOut == output)
            return true;
    }

    return false;
}

QString RequestInfo::getPackageSaveName() const
{
    return packageSaveName;
}

QUrl RequestInfo::getCheckFileUrl() const
{
    return checkFileUrl;
}

QString RequestInfo::getCheckFileSaveName() const
{
    return checkFileSaveName;
}

QString RequestInfo::getCheckNumProgram() const
{
    return checkNumProgram;
}

QString RequestInfo::getCheckNumMode() const
{
    return checkNumMode;
}

void RequestInfo::setPackageUrl(const QUrl &value)
{
    packageUrl = value;
}

void RequestInfo::setPackageSaveName(const QString &value)
{
    packageSaveName = value;
}

void RequestInfo::setCheckFileUrl(const QUrl &value)
{
    checkFileUrl = value;
}

void RequestInfo::setCheckFileSaveName(const QString &value)
{
    checkFileSaveName = value;
}

void RequestInfo::setCheckNumProgram(const QString &value)
{
    checkNumProgram = value;
}

void RequestInfo::setCheckNumMode(const QString &value)
{
    checkNumMode = value;
}

QUrl RequestInfo::getPackageUrl() const
{
    return packageUrl;
}
