// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processutil.h"

#include <QProcess>
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include <QUrl>

bool ProcessUtil::execute(const QString &program,
                          const QStringList &arguments,
                          ProcessUtil::ReadCallBack func)
{
    return ProcessUtil::execute(program, arguments, "", func);
}

bool ProcessUtil::execute(const QString &program,
                          const QStringList &arguments,
                          const QString &workdir,
                          ProcessUtil::ReadCallBack func)
{
    return execute(program, arguments, workdir, QProcess().processEnvironment(), func);
}

bool ProcessUtil::execute(const QString &program,
                          const QStringList &arguments,
                          const QString &workdir,
                          const QProcessEnvironment &env,
                          ProcessUtil::ReadCallBack func)
{
    bool ret = true;
    QProcess process;
    process.setWorkingDirectory(workdir);
    process.setProgram(program);
    process.setArguments(arguments);
    process.setProcessEnvironment(env);
    process.connect(&process, QOverload<int, QProcess::ExitStatus >::of(&QProcess::finished),
                    [&](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitCode != 0)
            ret = false;
        qInfo() << program << arguments.join(" ")
                << "exitCode: " << exitCode
                << "exitStatus: " << exitStatus;
    });
    process.connect(&process, &QProcess::errorOccurred,
                    [&](QProcess::ProcessError error){
        ret = false;
        qCritical() << program << arguments.join(" ")
                    << "error: " << error
                    << "errorString: " << process.errorString();
    });

    process.start();
    process.waitForFinished();

    if (func) {
        QByteArray array = process.readAll();
        func(array.trimmed());
    }

    return ret;
}

QString ProcessUtil::execute(const QStringList &commands, bool cascade)
{
    auto executeCascade = [&](QString command, QString arg)->QString{
        QString ret;
        execute("bash", QStringList() << "-c" << (command + " " + arg), [&](const QByteArray &output){
            ret = output;
        });
        return ret;
    };

    QString ret;
    foreach (QString command, commands) {
        if (cascade) {
            ret = executeCascade(command, ret);
        } else{
            executeCascade(command, {});
        }
    }
    return ret;
}

bool ProcessUtil::exists(const QString &name)
{
    bool ret = false;
#ifdef linux
    auto outCallback = [&ret, &name](const QByteArray &array) {
        QList<QByteArray> rmSearch = array.split(' ');
        foreach (QByteArray rmProcess, rmSearch) {
            QFileInfo info(rmProcess.trimmed());
            if (info.exists() && info.fileName() == name && info.isExecutable()) {
                if (!ret) {
                    ret = true;
                    break;
                }
            } else {
                ret = false;
            }
        }
    };
    ProcessUtil::execute("whereis", {name}, outCallback);
#else
#endif
    return ret;
}

QString ProcessUtil::version(const QString &name)
{
    QString retOut;
#ifdef linux
    auto outCallback = [&retOut](const QByteArray &array) {
        retOut = QString ::fromLatin1(array);
    };
    ProcessUtil::execute(name, {"-version"}, outCallback);
#else
#endif
    return retOut;
}

bool ProcessUtil::hasGio()
{
    return exists("gio");
}

bool ProcessUtil::moveToTrash(const QString &filePath)
{
#ifdef linux
    if (!hasGio())
        return false;
    return ProcessUtil::execute("gio", {"trash", filePath});
#else

#endif
}

bool ProcessUtil::recoverFromTrash(const QString &filePath)
{
#ifdef linux
    if (!hasGio() || filePath.isEmpty())
        return false;

    QDirIterator itera(QDir::homePath() + QDir::separator() + ".local/share/Trash/files");
    while (itera.hasNext()){
        itera.next();
        QFileInfo info(filePath);
        if(info.suffix() == itera.fileInfo().suffix()
                && info.baseName() == itera.fileInfo().baseName()) {
            QByteArray readArray;
            auto readCB = [&](const QByteArray array){readArray = array;};
            QString trashFileUri = QString("trash:///%0").arg(info.fileName());
            QStringList queryArgs;
            queryArgs << "info";
            queryArgs << trashFileUri;
            queryArgs << "| grep trash::orig-path";
            bool execResult = ProcessUtil::execute("gio", queryArgs, readCB);
            if (!execResult && readArray.isEmpty()) {
                qCritical() << "Unknown Error";
                abort();
            }
            readArray = readArray.replace(" ", "");
            auto list = readArray.split('\n');
            auto itera = list.rbegin();
            while (itera != list.rend()) {
                if (itera->startsWith("trash::orig-path:")) {
                    readArray = *itera;
                    break;
                }
                itera ++;
            }
            if (!readArray.startsWith("trash::orig-path:")) {
                qCritical() << "Error from: " <<QString::fromUtf8(readArray);
                abort();
            }
            readArray = readArray.replace("trash::orig-path:", "");
            if (readArray == filePath) {
                QStringList args;
                args << "move";
                args << trashFileUri;
                args << QUrl::fromLocalFile(filePath).toString();
                return ProcessUtil::execute("gio", args);
            }
        }
    }

#else

#endif
    return false;
}

bool ProcessUtil::portOverhead(unsigned int port)
{
    bool ret = true;
#ifdef linux
    auto outCallback = [&ret](const QByteArray &array) {
        qInfo() << qPrintable(array);
        if (array.isEmpty()) {
            ret = false;
        }
    };
    ProcessUtil::execute("/bin/bash", {"-c", "ss -ntlp|grep " + QString::number(port)}, outCallback);
#else
#endif
    return ret;
}

QString ProcessUtil::localPlatform()
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

