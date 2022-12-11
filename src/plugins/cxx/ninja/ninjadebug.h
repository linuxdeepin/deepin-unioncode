/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#ifndef NINJADEBUG_H
#define NINJADEBUG_H

#include "dap/protocol.h"
#include <QObject>

class NinjaDebugPrivate;
class NinjaDebug : public QObject
{
    Q_OBJECT
public:
    explicit NinjaDebug(QObject *parent = nullptr);
    ~NinjaDebug();

    bool requestDAPPort(const QString &uuid, const QString &kit,
                        const QString &targetPath, const QStringList &arguments,
                        QString &retMsg);
    bool isLaunchNotAttach();
    dap::LaunchRequest launchDAP(const QString &targetPath, const QStringList &argments);

signals:

private slots:

private:
    NinjaDebugPrivate *const d;
};

#endif // NINJADEBUG_H
