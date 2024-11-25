// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONSERVICE_H
#define SESSIONSERVICE_H

#include "services/services_global.h"
#include <framework/framework.h>

namespace dpfservice {
class SERVICE_EXPORT SessionService final : public dpf::PluginService, dpf::AutoServiceRegister<SessionService>
{
    Q_OBJECT
    Q_DISABLE_COPY(SessionService)
public:
    static QString name()
    {
        return "org.deepin.service.SessionService";
    }

    explicit SessionService(QObject *parent = nullptr)
        : dpf::PluginService(parent)
    {
    }

    DPF_INTERFACE(QString, currentSession);
    DPF_INTERFACE(QString, lastSession);
    DPF_INTERFACE(QStringList, sessionList);
    DPF_INTERFACE(QDateTime, sessionDateTime, const QString &session);
    DPF_INTERFACE(QDateTime, lastActiveTime, const QString &session);

    DPF_INTERFACE(bool, createSession, const QString &session);
    DPF_INTERFACE(bool, removeSession, const QString &session);
    DPF_INTERFACE(void, removeSessions, const QStringList &sessions);
    DPF_INTERFACE(bool, renameSession, const QString &oldName, const QString &newName);
    DPF_INTERFACE(bool, cloneSession, const QString &select, const QString &clone);
    DPF_INTERFACE(void, showSessionManager);

    DPF_INTERFACE(void, setValue, const QString &key, const QVariant &value);
    DPF_INTERFACE(QVariant, value, const QString &key);

    DPF_INTERFACE(bool, loadSession, const QString &session);
    DPF_INTERFACE(bool, saveSession);
    DPF_INTERFACE(bool, isDefaultSession, const QString &session);
    DPF_INTERFACE(bool, isSessionLoading);
    DPF_INTERFACE(bool, isDefaultVirgin);

    DPF_INTERFACE(QString, sessionFile, const QString &session);
};
}

#endif   // SESSIONSERVICE_H
