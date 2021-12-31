/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef APPBUS_H
#define APPBUS_H

#include "framework/framework_global.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class AppBusPrivate;
class AppBus : public QObject
{
    Q_OBJECT
    friend class AppBusPrivate;
    AppBusPrivate *const d;
public:
    explicit AppBus(QObject *parent = nullptr);
    virtual ~AppBus();
    QStringList onlineServer();
    QString mimeServer();
    bool isMimeServer(const QString& serverName);
Q_SIGNALS:
    void newCreateAppBus(const QString& serverName);
};

DPF_END_NAMESPACE

#endif // APPBUS_H
