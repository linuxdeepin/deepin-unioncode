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
#ifndef PYTHONDEBUGGER_H
#define PYTHONDEBUGGER_H


#include <QObject>

class PythonDebuggerPrivate;
class PythonDebugger : public QObject
{
    Q_OBJECT
public:
    explicit PythonDebugger(QObject *parent = nullptr);
    virtual ~PythonDebugger() override;

signals:
    void sigSendToClient(const QString &uuid, int port, const QString &kit);

public slots:
    void slotReceiveClientInfo(const QString &uuid,
                               const QString &kit,
                               const QString &pythonExecute,
                               const QString &fileName);

private:
    void registerLaunchDAPConnect();
    void initialize(const QString &pythonExecute,
                    const QString &fileName);

    PythonDebuggerPrivate *const d;
};

#endif // PYTHONDEBUGGER_H
