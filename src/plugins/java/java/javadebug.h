// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
