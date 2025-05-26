// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONENTITY_H
#define NOTIFICATIONENTITY_H

#include <QObject>
#include <QDateTime>

#include <memory>
#include <functional>

using NotifyCallback = std::function<void(const QString &actId)>;

class NotificationEntityPrivate;
class NotificationEntity : public QObject
{
    Q_OBJECT
public:
    enum NotificationType {
        Information,
        Warning,
        Error
    };

    explicit NotificationEntity(NotificationType type, const QString &name = QString(),
                                const QString &msg = QString(), const QStringList &actions = QStringList(),
                                NotifyCallback cb = nullptr, QObject *parent = nullptr);
    ~NotificationEntity();

    QString name() const;
    void setName(const QString &name);

    QString message() const;
    void setMessage(const QString &msg);

    QStringList actions() const;
    void setActions(const QStringList &actions);

    NotificationType type() const;
    void setType(NotificationType type);

    NotifyCallback callback() const;
    void setCallback(NotifyCallback cb);

private:
    NotificationEntityPrivate *const d;
};

Q_DECLARE_METATYPE(std::shared_ptr<NotificationEntity>);

#endif   // NOTIFICATIONENTITY_H
