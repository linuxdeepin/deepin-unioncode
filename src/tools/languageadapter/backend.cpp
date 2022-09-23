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
#include "backend.h"

#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QMetaEnum>
#include <QDir>
#include <QStandardPaths>

#include <iostream>

struct BackendPrivate
{
    friend class Backend;
    SettingInfo saveInfo;
    QIODevice *backendIns;
};

Backend::Backend(const SettingInfo &info)
    : d (new BackendPrivate)
{
    std::cout << __FUNCTION__ << std::endl;
    d->saveInfo = info;
    if (d->saveInfo.mode == "process") {
        QProcess *process = new QProcess(this);
        if (d->saveInfo.language == "Python") {
            auto procEnv = process->processEnvironment();
            QString userRuntimeBinPath = QDir::homePath() + QDir::separator() + ".local"
                    + QDir::separator() + "bin";
            QString userPythonPkgPath = QDir::homePath() + QDir::separator() + ".local"
                    + QDir::separator() + "lib"
                    + QDir::separator() + "python3.7"
                    + QDir::separator() + "site-packages";
            procEnv.insert("PYTHONPATH=%0", userPythonPkgPath);
            QString PATH_EnvValue = procEnv.value("PATH");
            procEnv.remove("PATH");
            procEnv.insert("PATH", userRuntimeBinPath + ":" + PATH_EnvValue);
            process->setProcessEnvironment(procEnv);
            qInfo() << procEnv.value("PATH");
        }
        QObject::connect(process, &QProcess::errorOccurred,
                         [=](QProcess::ProcessError error)
        {
            std::cout << process->errorString().toStdString()
                      << QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error)
                      << std::endl;
        });
        QObject::connect(process, &QProcess::readyReadStandardError,
                         [=](){
            std::cout << process->readAllStandardError().toStdString()
                      << std::endl;
        });
        QObject::connect(process, &QProcess::aboutToClose,
                         this, &Backend::aboutToClose, Qt::UniqueConnection);
        process->setProgram("bash");
        process->setArguments({"-c", info.program + " " + info.arguments.join(" ")});
        process->setWorkingDirectory(info.workDir);
        process->setReadChannel(QProcess::StandardOutput);
        process->setReadChannelMode(QProcess::SeparateChannels);
        d->backendIns = process;
        process->start();
        if (info.language == "Java") {
            while (!process->waitForReadyRead()) {
                if (process->state() == QProcess::ProcessState::NotRunning) {
                    std::cout << "quit:"
                              << process->program().toStdString() << " "
                              << process->exitCode() << ","
                              << QMetaEnum::fromType<QProcess::ExitStatus>().valueToKey(process->exitStatus())
                              << std::endl;
                    break;
                }
            };
        }
    }
}

Backend::~Backend()
{
    if (d) {
        delete d;
    }
}

void Backend::writeAndWait(const QByteArray &data)
{
    if (!d->backendIns)
        return;
    d->backendIns->write(data);
    d->backendIns->waitForBytesWritten(-1);
}

bool Backend::readAndWait(QVector<QJsonObject> &jsonObjs, QByteArray &source)
{
    int waitCount = 0;
    //    qInfo() << "waitForReadyRead";
    while(!d->backendIns->waitForReadyRead(100)) {
        if ( 3 == waitCount ++) {
            qCritical() << "wait timeOut";
        }
        return false;
    }

    QString contentLengthKey{"ContentLength"};
    QString contentTypeKey{"ContentType"};
    QString charsetKey{"charset"};
    QRegularExpression headRegExp("^Content-Length:\\s?(?<" + contentLengthKey + ">[0-9]+)\\r\\n" +
                                 "(Content-Type:\\s?(?<" + contentTypeKey + ">\\S+);" +
                                 "\\s?charset=(?<" + charsetKey + ">\\S+)\\r\\n)?\\r\\n");

    QByteArray head, jsonSrc;
    while (d->backendIns->bytesAvailable()) {
        head += d->backendIns->read(1);
        auto matchs = headRegExp.match(head);
        if (matchs.hasMatch()) {
            int contentLength = matchs.captured(contentLengthKey).toInt();
            QString contentType = matchs.captured(contentTypeKey); Q_UNUSED(contentType);
            QString charset = matchs.captured(charsetKey); Q_UNUSED(charset);
            while (jsonSrc.size() != contentLength) {
                QByteArray readChar = d->backendIns->read(1);
                if (readChar.isEmpty()) {
                    qInfo() << "readChar wait";
                    d->backendIns->waitForReadyRead(1);
                } else {
                    jsonSrc += readChar;
                }
            }
            QJsonObject jsonObj = QJsonDocument::fromJson(jsonSrc).object();
            qInfo() << "read jsonobject:\n" << jsonObj;
            jsonObjs.append(jsonObj);
            source += head += jsonSrc;
            head.clear();
            jsonSrc.clear();
        }
    }
    return true;
}

SettingInfo Backend::info() const
{
    return d->saveInfo;
}

bool Backend::canRead()
{
    return d->backendIns->bytesAvailable() > 1;
}


