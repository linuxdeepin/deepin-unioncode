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
#include <QStandardPaths>

#define PYLS_PACKAGENAME "python-language-server[all]"
#define PYLS_PROGRAM_MAIN "pyls"

#define JDTLS_PACKAGE_URL "https://download.eclipse.org/jdtls/snapshots/jdt-language-server-1.11.0-202205051421.tar.gz"
#define JDTLS_PACKAGE_NAME "jdt-language-server.tar.gz"
#define JDTLS_CHECKFILE_URL "https://download.eclipse.org/jdtls/snapshots/jdt-language-server-1.11.0-202205051421.tar.gz.sha256"
#define JDTLS_CHECKFILE_NAME "jdt-language-server.tar.gz.sha256"
#define JDTLS_CHECKPROGRAM_NAME "shasum"
#define JDTLS_CHECKPROGRAM_MODE "256"

#define JDTLS_PROGRAM_MAIN "jdtls"
#define JDTLS_PROGRAM_MAIN_MIME "text/x-python3"

BackendChecker::BackendChecker(QWidget *parent)
    : QWidget(parent)
{
    RequestInfo jdtlsInfo;
    jdtlsInfo.setPackageUrl(QUrl(JDTLS_PACKAGE_URL));
    jdtlsInfo.setPackageSaveName(JDTLS_PACKAGE_NAME);
    jdtlsInfo.setCheckFileUrl(QUrl(JDTLS_CHECKFILE_URL));
    jdtlsInfo.setCheckFileSaveName(JDTLS_CHECKFILE_NAME);
    jdtlsInfo.setCheckNumProgram(JDTLS_CHECKPROGRAM_NAME);
    jdtlsInfo.setCheckNumMode(JDTLS_CHECKPROGRAM_MODE);

    Pip3GitInstall pylspInfo;
    pylspInfo.packageName = PYLS_PACKAGENAME;
    pylspInfo.programMain = PYLS_PROGRAM_MAIN;

    requestInfos["Java"] = QVariant::fromValue<RequestInfo>(jdtlsInfo);
    requestInfos["Python"] = QVariant::fromValue<Pip3GitInstall>(pylspInfo);
}

BackendChecker &BackendChecker::instance()
{
    static BackendChecker ins;
    return ins;
}

void BackendChecker::checkLanguageBackend(const QString &languageID)
{
    QDir dir = QDir::home();
    if (!dir.cd(".config")) { dir.mkdir(".config"); }
    if (!dir.cd("languageadapter")) { dir.mkdir("languageadapter"); }
    adapterPath = dir.path();

    // reconfig new lsp server
    if (QFile::exists(dir.path() + QDir::separator() + "languageAdapter.conf")) {
        dir.removeRecursively();
        dir.cdUp();
        if (!dir.cd("languageadapter")) { dir.mkdir("languageadapter"); }
    }

    auto itera = requestInfos.begin();
    while (itera != requestInfos.end()) {
        if (!dir.cd(itera.key())) {
            dir.mkdir(itera.key());
        }
        dir.cdUp();
        itera ++;
    }

    if (requestInfos.keys().contains(languageID)) {
        QVariant infoVar = requestInfos.value(languageID);
        if (infoVar.canConvert<RequestInfo>()) {
            auto info = qvariant_cast<RequestInfo>(infoVar);
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
        } else if (infoVar.canConvert<Pip3GitInstall>()) {
            auto info = qvariant_cast<Pip3GitInstall>(infoVar);
            QString userLocalBinPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                    + QDir::separator() + ".local" + QDir::separator() + "bin";
            if (!QFileInfo(userLocalBinPath + QDir::separator() + PYLS_PROGRAM_MAIN).exists()) {
                auto pip3Dialog = new Pip3Dialog();
                pip3Dialog->install("python-language-server[all]");
                pip3Dialog->exec();
            }
        }
    }
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
    QVariant infoVar = requestInfos.value(languageID);

    if (infoVar.canConvert<RequestInfo>()) {
        RequestInfo info = qvariant_cast<RequestInfo>(infoVar);

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
