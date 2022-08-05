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

struct BackendPrivate
{
    friend class Backend;
    SettingInfo saveInfo;
    QIODevice *backendIns;
};

Backend::Backend(const SettingInfo &info)
    : d (new BackendPrivate)
{
    d->saveInfo = info;
    if (d->saveInfo.mode == "process") {
        QProcess *process = new QProcess(this);
        QObject::connect(process, &QProcess::errorOccurred,
                         [=](QProcess::ProcessError error){
            qCritical() << process->errorString() << error;
        });
        QObject::connect(process, &QProcess::readyReadStandardError,
                         [=](){
            qCritical() << process->readAllStandardError();
        });
        QObject::connect(process, &QProcess::aboutToClose,
                         this, &Backend::aboutToClose,
                         Qt::UniqueConnection);
        process->setProgram(info.program);
        process->setArguments(info.arguments);
        process->setWorkingDirectory(info.workDir);
        process->setReadChannel(QProcess::StandardOutput);
        process->setReadChannelMode(QProcess::SeparateChannels);
        d->backendIns = process;
        process->start();
        process->waitForStarted();
        if (info.language == "Java") {
            while (!process->waitForReadyRead());
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
    qInfo() << "waitForReadyRead";
    while(d->backendIns->waitForReadyRead(100)) {
        if ( 3 == waitCount ++) {
            qCritical() << "wait timeOut";
        }
        return false;
    }

    QRegularExpression regExp("^Content-Length:\\s+(\\d+)\\r\\n\\r\\n");
    QByteArray head, jsonSrc;
    while (d->backendIns->bytesAvailable()) {
        head += d->backendIns->read(1);
        auto matchs = regExp.match(head);
        if (matchs.hasMatch()) {
            int jsonObjSize = matchs.captured(1).toInt();
            while (jsonSrc.size() != jsonObjSize) {
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


