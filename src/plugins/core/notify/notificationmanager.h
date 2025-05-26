// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include "constants.h"

#include <QObject>

class Bubble;
class NotificationManagerPrivate;
class NotificationManager : public QObject
{
    Q_OBJECT

public:
    static NotificationManager *instance();

public Q_SLOTS:
    void notify(uint type, const QString &name, const QString &msg, const QStringList &actions);
    void notify(uint type, const QString &name, const QString &msg, const QStringList &actions, NotifyCallback cb);
    void show();
    void hide();
    void toggle();
    void clear();
    void remove(EntityPtr entity);

    void bubbleExpired(Bubble *bubble);
    void bubbleDismissed(Bubble *bubble);
    void bubbleActionInvoked(Bubble *bubble, const QString &actId);

Q_SIGNALS:
    void updated();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit NotificationManager(QObject *parent = nullptr);
    ~NotificationManager();

    NotificationManagerPrivate *const d;
};

#endif   // NOTIFICATIONMANAGER_H
