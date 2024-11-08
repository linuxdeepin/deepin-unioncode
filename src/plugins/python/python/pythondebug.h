// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    bool requestDAPPort(const QString &ppid, const QString &kit,
                        const QString &projectPath,
                        const QString &fileName,
                        QString &retMsg);
    bool isLaunchNotAttach();
    dap::AttachRequest attachDAP(int port, const QString &workspace);
    bool isRestartDAPManually();
    bool isStopDAPManually();

private Q_SLOTS:
    void notifyMessage(const QString &msg);
    void notifyToInstall();

private:
    PythonDebugPrivate *const d;
};

#endif // PYTHONDEBUG_H
