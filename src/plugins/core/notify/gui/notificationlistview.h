// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONLISTVIEW_H
#define NOTIFICATIONLISTVIEW_H

#include "notify/constants.h"

#include <DListView>

class NotificationListView : public DTK_WIDGET_NAMESPACE::DListView
{
    Q_OBJECT
public:
    explicit NotificationListView(QWidget *parent = nullptr);
    ~NotificationListView();

Q_SIGNALS:
    void processed(EntityPtr ptr);
    void actionInvoked(EntityPtr ptr, const QString &actId);
};

#endif   // NOTIFICATIONLISTVIEW_H
