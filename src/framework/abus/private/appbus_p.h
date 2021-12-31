/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef APPBUS_P_H
#define APPBUS_P_H

#include "framework/abus/appbus.h"
#include "framework/service/pluginservice.h"

#include "framework/framework_global.h"

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QCoreApplication>

DPF_BEGIN_NAMESPACE

class AppBusPrivate : public QObject
{
    Q_OBJECT
    friend class AppBus;
    AppBus *const q;

public:
    explicit AppBusPrivate(AppBus * dd);
    void importService(PluginService* service);
    void exportService(PluginService* service);

private Q_SLOTS:
    void procNewConnection();

private:
    QLocalServer server;

    QString tryPingString;
    QString appServerName;
    QHash<QString, QLocalSocket*> onlineServers;

    QStringList scanfUseBusApp();

    bool tryPing(const QString &serverName);
    bool isTryPing(const QByteArray &array);
    bool isKeepAlive(const QByteArray &array);
    bool isMethodCall(const QByteArray &array);
};

DPF_END_NAMESPACE

#endif // APPBUS_P_H
