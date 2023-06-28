// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEDEBUG_H
#define CMAKEDEBUG_H

#include "dap/protocol.h"
#include <QObject>

class CMakeDebugPrivate;
class CMakeDebug : public QObject
{
    Q_OBJECT
public:
    explicit CMakeDebug(QObject *parent = nullptr);
    ~CMakeDebug();

    bool requestDAPPort(const QString &uuid, const QString &kit,
                        const QString &targetPath, const QStringList &arguments,
                        QString &retMsg);
    bool isLaunchNotAttach();
    bool prepareDebug(QString &retMsg);
    dap::LaunchRequest launchDAP(const QString &targetPath, const QStringList &argments);

signals:

private slots:

private:
    CMakeDebugPrivate *const d;
};

#endif // CMAKEDEBUG_H
