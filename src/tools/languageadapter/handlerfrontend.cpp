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
#include "handlerfrontend.h"
#include "handlerbackend.h"
#include "protocol.h"
#include "route.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

HandlerFrontend::HandlerFrontend()
{

}

void HandlerFrontend::filterData(const QByteArray &array)
{
    static QByteArray cache;
    if (array.startsWith("Content-Length:")) {
        cache = array;
        int index = cache.indexOf("\r\n\r\n");
        if (index == -1) {
            return;
        } else {
            QByteArray head = cache.mid(0, index);
            QList<QByteArray> headList = head.split(':');
            int readCount = headList[1].toInt();
            QByteArray data = cache.mid(head.size() + 4, readCount);
            if (data.size() == readCount) {
                request(data);
                cache.remove(0, index);
                cache.remove(0, readCount + 4);
            } else if (data.size() < readCount) {
                return;
            } else {
                qCritical() << "process data error";
                abort();
            }
            if (!cache.isEmpty()) {
                filterData(cache);
            }
        }
    } else {
        cache += array;
        filterData(cache);
    }
}

void HandlerFrontend::request(const QByteArray &array)
{
    QJsonParseError err;
    auto jsonObj = QJsonDocument::fromJson(array, &err).object();
    if (jsonObj.isEmpty()) {
        qInfo() << err.error;
        return;
    }

    qInfo() << "\nRequest -> " << qPrintable(array);

    if (initRequest(jsonObj))
        return;

//    if (exitRequest(jsonObj))
//        return;
}

bool HandlerFrontend::initRequest(const QJsonObject &obj)
{
    bool result = false;
    if (obj.value("method").toString() == "initialize") {
        QJsonObject paramsObj = obj.value("params").toObject();
        QString language = paramsObj.value("language").toString();
        QString workspaceFolder;
        QJsonArray array = paramsObj.value("workspaceFolders").toArray();
        if (array.size() >= 1) {
            workspaceFolder = array[0].toObject().value("uri").toString();
        }
        Route::Head head {workspaceFolder, language};
        Route::instance()->saveFrontend(head, this);
        auto backend = Route::instance()->createBackend(head);
        Route::instance()->saveBackend(head, backend);
        QObject::connect(backend, &HandlerBackend::nowReadedAll, this, &HandlerFrontend::nowToWrite, Qt::UniqueConnection);
        QObject::connect(this, &HandlerFrontend::nowReadedAll, backend, &HandlerBackend::nowToWrite, Qt::UniqueConnection);
        //手动发送初始化
        backend->nowToWrite(QJsonDocument(obj).toJson());
        result = true;
    }
    return result;
}
