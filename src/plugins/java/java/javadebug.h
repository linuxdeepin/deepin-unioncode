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
#ifndef JAVADEBUG_H
#define JAVADEBUG_H

#include "dap/protocol.h"
#include <QObject>

class JavaDebugPrivate;
class JavaDebug : public QObject
{
    Q_OBJECT
public:
    explicit JavaDebug(QObject *parent = nullptr);
    ~JavaDebug();

    bool isAnsyPrepareDebug();
    bool prepareDebug(const QString &projectPath,
                      QString &retMsg);
    bool requestDAPPort(const QString &uuid, const QString &kit,
                        const QString &projectPath,
                        QString &retMsg);
    bool isLaunchNotAttach();
    dap::LaunchRequest launchDAP(const QString &workspace,
                                 const QString &mainClass,
                                 const QString &projectName,
                                 const QStringList &classPaths);
    bool isRestartDAPManually();
    bool isStopDAPManually();

signals:
    void sigProgressMsg(const QString &msg);

private slots:

private:
    bool checkConfigFile(QString &retMsg);
    void outProgressMsg(const QString &msg);

private:
    JavaDebugPrivate *const d;
};

#endif // JAVADEBUG_H
