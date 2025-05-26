// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONITEMWIDGET_H
#define NOTIFICATIONITEMWIDGET_H

#include "notify/constants.h"

#include <QWidget>

class NotificationItemWidgetPrivate;
class NotificationItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NotificationItemWidget(QWidget *parent = nullptr, EntityPtr entity = nullptr);
    ~NotificationItemWidget();

    EntityPtr entity() const;
    void setEntity(EntityPtr entity);

Q_SIGNALS:
    void dismissed();
    void processed(EntityPtr ptr);
    void actionInvoked(const QString &actId);

private:
    NotificationItemWidgetPrivate *const d;
};

#endif   // NOTIFICATIONITEMWIDGET_H
