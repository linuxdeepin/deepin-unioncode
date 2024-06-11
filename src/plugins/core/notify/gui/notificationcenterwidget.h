// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONCENTERWIDGET_H
#define NOTIFICATIONCENTERWIDGET_H

#include "notify/constants.h"

#include <DFloatingWidget>

class NotificationCenterWidgetPrivate;
class NotificationCenterWidget : public DTK_WIDGET_NAMESPACE::DFloatingWidget
{
    Q_OBJECT
public:
    explicit NotificationCenterWidget(QWidget *parent = nullptr);
    ~NotificationCenterWidget();

    void setNotifications(const QList<EntityPtr> &datas);
    void addNotification(EntityPtr data);

public Q_SLOTS:
    void handleClear();
    void handleActionInvoked(EntityPtr ptr, const QString &actId);
    void handleProcessed(EntityPtr ptr);

private:
    NotificationCenterWidgetPrivate *const d;
};

#endif   // NOTIFICATIONCENTERWIDGET_H
