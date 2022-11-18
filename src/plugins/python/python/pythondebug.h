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
#ifndef PYTHONDEBUG_H
#define PYTHONDEBUG_H

#include "dap/protocol.h"
#include <QObject>

class PythonDebugPrivate;
class PythonDebug : public QObject
{
    Q_OBJECT
public:
    explicit PythonDebug(QObject *parent = nullptr);
    ~PythonDebug();

    bool prepareDebug(const QString &fileName, QString &retMsg);
    bool requestDAPPort(const QString &uuid, const QString &kit,
                        const QString &projectPath,
                        const QString &fileName,
                        QString &retMsg);
    bool isLaunchNotAttach();
    dap::AttachRequest attachDAP(int port, const QString &workspace);
    bool isRestartDAPManually();
    bool isStopDAPManually();

signals:

private slots:

private:
    PythonDebugPrivate *const d;
};

#endif // PYTHONDEBUG_H
