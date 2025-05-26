// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>

class SessionManagerPrivate;
class SessionManager : public QObject
{
    Q_OBJECT
public:
    static SessionManager *instance();

    QString currentSession();
    QString lastSession();
    QStringList sessionList();
    QDateTime sessionDateTime(const QString &session);
    QDateTime lastActiveTime(const QString &session);

    bool createSession(const QString &session);
    bool removeSession(const QString &session);
    void removeSessions(const QStringList &sessions);
    bool renameSession(const QString &oldName, const QString &newName);
    bool cloneSession(const QString &select, const QString &clone);
    void showSessionManager();

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key);

    bool loadSession(const QString &session);
    void saveSession();
    bool isDefaultSession(const QString &session);
    bool isSessionLoading();
    bool isDefaultVirgin();
    bool isAutoLoadLastSession();

    void markSessionFileDirty();
    QString sessionFile(const QString &session);

Q_SIGNALS:
    void readyToSaveSession();
    void sessionLoaded(const QString &session);
    void sessionStatusChanged();

    void sessionCreated(const QString &session);
    void sessionRenamed(const QString &oldName, const QString &newName);
    void sessionRemoved(const QString &session);

private:
    SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    SessionManagerPrivate *const d;
};

#endif   // SESSIONMANAGER_H
