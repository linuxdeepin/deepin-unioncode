// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
