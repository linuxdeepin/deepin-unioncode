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
#include "common/common.h"
#include "jsonrpccallproxy.h"
#include "remotechecker.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonObject>
#include <QDir>

#include <iostream>

// setting from clangd trismit
QProcess *createCxxServ(const newlsp::ProjectKey &key)
{
    lspServOut << __FUNCTION__ << qApp->thread() << QThread::currentThread();
    if (key.language != newlsp::Cxx)
        return nullptr;

    QString projectCacheDir = ".unioncode";
    QString compileDB_Path = QString::fromStdString(key.workspace) + QDir::separator() + projectCacheDir;
    QStringList compileDB_CMD_As;
    compileDB_CMD_As << "-S" << QString::fromStdString(key.workspace);
    compileDB_CMD_As << "-B" << compileDB_Path;
    compileDB_CMD_As << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
    QProcess::execute("/usr/bin/cmake", compileDB_CMD_As);

    QStringList procAs;
    QString clangd = "clangd-13";
    if (ProcessUtil::exists(clangd)) {
        procAs << clangd;
    } else {
        QString clangdFileName = "clangd";
        if (!env::pkg::native::installed()) {
            RemoteChecker::instance().checkLanguageBackend(QString::fromStdString(key.language));
            QString runtimePath = CustomPaths::lspRuntimePath(QString::fromStdString(key.language));
            procAs << runtimePath + QDir::separator() + clangdFileName;
        } else {
            procAs << env::pkg::native::path(clangdFileName);
        }
    }

    procAs << "--log=verbose";
    procAs << QString("--compile-commands-dir=%0").arg(compileDB_Path);
    procAs << "--clang-tidy";
    procAs << "-j=$(nproc)";

    auto proc = new QProcess();
    proc->setProgram("/usr/bin/bash");
    proc->setArguments({"-c", procAs.join(" ")});
    proc->setProcessChannelMode(QProcess::ForwardedOutputChannel);
    QObject::connect(proc, &QProcess::readyReadStandardError, proc, [=]() {
        std::cerr << proc->readAllStandardError().toStdString() << std::endl;
    });
    proc->start();

    JsonRpcCallProxy::ins().setSelect(key);
    return proc;
}

// setting from jdtls trismit
QProcess *createJavaServ(const newlsp::ProjectKey &key)
{
    lspServOut << __FUNCTION__ << qApp->thread() << QThread::currentThread();
    if (key.language != newlsp::Java)
        return nullptr;

    QString dataDir = CustomPaths::projectCachePath(QString::fromStdString(key.workspace));
    QString runtimePath = CustomPaths::lspRuntimePath(QString::fromStdString(key.language));
    bool noRuntimeChilds = QDir(runtimePath).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot).isEmpty();
    if (noRuntimeChilds) {
        if (!env::pkg::native::installed()) {
            RemoteChecker::instance().checkLanguageBackend(QString::fromStdString(key.language));
        } else {
            lspServOut << "unzip install native package..." << noRuntimeChilds;
            QString jdtlsNativePkgPath = env::pkg::native::path(env::package::Category::get()->jdtls);
            ProcessUtil::execute("tar", {"zxvf", jdtlsNativePkgPath, "-C", "."}, runtimePath,
                                 [=](const QByteArray &data){
                lspServOut << QString(data);
            });
        }
    }

    auto proc = new QProcess();
    QString lanuchLine = "/usr/bin/java "
                         "-Declipse.application=org.eclipse.jdt.ls.core.id1 "
                         "-Dosgi.bundles.defaultStartLevel=4 "
                         "-Declipse.product=org.eclipse.jdt.ls.core.product "
                         "-Dlog.level=ALL "
                         "-noverify "
                         "-Xmx1G "
                         "--add-modules=ALL-SYSTEM "
                         "--add-opens java.base/java.util=ALL-UNNAMED "
                         "--add-opens java.base/java.lang=ALL-UNNAMED "
                         "-jar " + runtimePath + "/plugins/org.eclipse.equinox.launcher_1.6.400.v20210924-0641.jar "
                                                 "-configuration " + runtimePath + "/config_linux "
            + QString("-data %0").arg(dataDir);
    proc->setProgram("/usr/bin/bash");
    proc->setArguments({"-c", lanuchLine});
    proc->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    QObject::connect(proc, &QProcess::readyReadStandardOutput, proc, [=](){
        std::cout << proc->readAllStandardOutput().toStdString() << std::endl;
    });
    proc->start();
    QObject::connect(qApp, &QCoreApplication::destroyed, [&](){ proc->kill(); });
    JsonRpcCallProxy::ins().setSelect(key);
    return proc;
}

// setting from pyls trismit
QProcess *createPythonServ(const newlsp::ProjectKey &key)
{
    lspServOut << __FUNCTION__ << qApp->thread() << QThread::currentThread();
    if (key.language != newlsp::Python)
        return nullptr;

    //    if (!env::pkg::native::installed()) {
    //    RemoteChecker::instance().checkLanguageBackend(QString::fromStdString(key.language));
    //    }

    QString jdtlsNativePkgPath = env::pkg::native::path(env::package::Category::get()->jdtls);
    ProcessUtil::execute("pip3", {"install", "python-language-server[all]"}, [=](const QByteArray &data){
        lspServOut << QString(data);
    });

    auto proc = new QProcess();
    proc->setProgram("/usr/bin/bash");
    proc->setArguments({"-c","pyls -v"});
    env::lang::Version pyVer;
    pyVer.major = 3;
    auto python3Env = env::lang::get(env::lang::Category::User, env::lang::Python, pyVer);
    proc->setProcessEnvironment(python3Env);
    proc->setProcessChannelMode(QProcess::ForwardedOutputChannel);
    QObject::connect(proc, &QProcess::readyReadStandardError,
                     proc, [=]() {
        std::cerr << proc->readAllStandardError().toStdString() << std::endl;
    });
    proc->start();

    JsonRpcCallProxy::ins().setSelect(key);

    return proc;
}

void selectLspServer(const QJsonObject &params)
{
    QString language = params.value(QString::fromStdString(newlsp::language)).toString();
    QString workspace = params.value(QString::fromStdString(newlsp::workspace)).toString();
    newlsp::ProjectKey projectKey {language.toStdString(), workspace.toStdString()};
    JsonRpcCallProxy::ins().setSelect(projectKey);
    QProcess *proc = JsonRpcCallProxy::ins().value(projectKey);

    if (!proc) {
        proc = JsonRpcCallProxy::ins().createLspServ(projectKey);
        if (proc) {
            lspServOut << "save backend process";
            JsonRpcCallProxy::ins().save(projectKey, proc);
            lspServOut << "selected ProjectKey{language:" << projectKey.language
                       <<  ", workspace:" << projectKey.workspace << "}";
            JsonRpcCallProxy::ins().setSelect(projectKey);
        }
    }

    if (!proc)
        lspServOut << "selected error ProjectKey{language:" << projectKey.language
                   << ",workspace:" << projectKey.workspace << "}";
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    JsonRpcCallProxy::ins().bindCreateProc(newlsp::Cxx, createCxxServ);
    JsonRpcCallProxy::ins().bindCreateProc(newlsp::Java, createJavaServ);
    JsonRpcCallProxy::ins().bindCreateProc(newlsp::Python, createPythonServ);
    JsonRpcCallProxy::ins().bindFilter("selectLspServer", selectLspServer);

    newlsp::ServerApplication lspServ(a);
    QObject::connect(&lspServ, &newlsp::ServerApplication::jsonrpcMethod,
                     &JsonRpcCallProxy::ins(), &JsonRpcCallProxy::methodFilter,
                     Qt::QueuedConnection);

    QObject::connect(&lspServ, &newlsp::ServerApplication::jsonrpcNotification,
                     &JsonRpcCallProxy::ins(), &JsonRpcCallProxy::notificationFilter,
                     Qt::QueuedConnection);
    lspServ.start();
    lspServOut << "created ServerApplication";

    return a.exec();
}
