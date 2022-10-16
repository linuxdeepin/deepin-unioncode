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
#include "envseacher.h"
#include "jsonrpccallproxy.h"

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

    auto proc = new QProcess();
#ifdef __linux__
    QString compileDB_Path = QString::fromStdString(key.workspace) + QDir::separator() + ".unioncode";
    QStringList compileDB_CMD_As;
    compileDB_CMD_As << "-S" << QString::fromStdString(key.workspace);
    compileDB_CMD_As << "-B" << compileDB_Path;
    compileDB_CMD_As << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
    QProcess::execute("/usr/bin/cmake", compileDB_CMD_As);

    QStringList procAs;
    procAs << "/usr/bin/unioncode-clangd";
    procAs << "--log=verbose";
    procAs << QString("--compile-commands-dir=%0").arg(compileDB_Path);

    proc->setProgram("/usr/bin/bash");
    proc->setArguments({"-c", procAs.join(" ")});
#endif
    proc->setProcessChannelMode(QProcess::ForwardedOutputChannel);
    QObject::connect(proc, &QProcess::readyReadStandardError,
                     proc, [=]() {
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

    auto proc = new QProcess();
#ifdef __linux__
    QString dataDir = QString::fromStdString(key.workspace) + QDir::separator()
            + ".unioncode" + QDir::separator()
            + QString::fromStdString(key.language);
    QString programAs = "/usr/bin/java "
                        "-Declipse.application=org.eclipse.jdt.ls.core.id1 "
                        "-Dosgi.bundles.defaultStartLevel=4 "
                        "-Declipse.product=org.eclipse.jdt.ls.core.product "
                        "-Dlog.level=ALL "
                        "-noverify "
                        "-Xmx1G "
                        "--add-modules=ALL-SYSTEM "
                        "--add-opens java.base/java.util=ALL-UNNAMED "
                        "--add-opens java.base/java.lang=ALL-UNNAMED "
                        "-jar ${HOME}/.config/languageadapter/Java/plugins/org.eclipse.equinox.launcher_1.6.400.v20210924-0641.jar "
                        "-configuration ${HOME}/.config/languageadapter/Java/config_linux "
            + QString("-data %0").arg(dataDir);
    proc->setProgram("/usr/bin/bash");
    proc->setArguments({"-c", programAs});
    proc->start();
#endif
    proc->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    QObject::connect(proc, &QProcess::readyReadStandardOutput,
                     proc, [=](){
        std::cout << proc->readAllStandardOutput().toStdString() << std::endl;
    });
    proc->start();

    JsonRpcCallProxy::ins().setSelect(key);
    return proc;
}

// setting from pyls trismit
QProcess *createPythonServ(const newlsp::ProjectKey &key)
{
    lspServOut << __FUNCTION__ << qApp->thread() << QThread::currentThread();
    if (key.language != newlsp::Python)
        return nullptr;

    auto proc = new QProcess();
#ifdef __linux__
    proc->setProgram("/usr/bin/bash");
    proc->setArguments({"-c","pyls -v"});
    auto procEnv = proc->processEnvironment();
    QVariantHash envs = EnvSeacher::python3();
    if (envs.keys().contains(K_UserEnv)) {
        QVariant userEnvVar = envs[K_UserEnv];
        if (userEnvVar.canConvert<UserEnv>()) {
            UserEnv userEnv = qvariant_cast<UserEnv>(userEnvVar);
            if (userEnv.binsPath) {
                QString PATH_EnvValue = procEnv.value("PATH");
                QString userRuntimeBinPath = userEnv.binsPath.value();
                procEnv.remove("PATH");
                procEnv.insert("PATH", userRuntimeBinPath + ":" + PATH_EnvValue);
            }
            if (userEnv.pkgsPath) {
                QString userPythonPkgPath = userEnv.pkgsPath.value();
                procEnv.insert("PYTHONPATH", userPythonPkgPath);
            }
        }
    }
    proc->setProcessEnvironment(procEnv);
#endif
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
            JsonRpcCallProxy::ins().save(projectKey, proc);
            lspServOut << "selected ProjectKey{language:" << projectKey.language
                       <<  ", workspace:" << projectKey.workspace
                        << "}";
        }
    }

    if (!proc)
        lspServOut << "selected error ProjectKey{language:" << projectKey.language
                   << ",workspace:" << projectKey.workspace
                   << "}";
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
                     &JsonRpcCallProxy::ins(), &JsonRpcCallProxy::methodFilter);

    QObject::connect(&lspServ, &newlsp::ServerApplication::jsonrpcNotification,
                     &JsonRpcCallProxy::ins(), &JsonRpcCallProxy::notificationFilter);
    lspServ.start();
    lspServOut << "created ServerApplication";

    return a.exec();
}
