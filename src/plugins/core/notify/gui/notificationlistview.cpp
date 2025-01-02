// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationlistview.h"

#include <QScroller>

DWIDGET_USE_NAMESPACE

NotificationListView::NotificationListView(QWidget *parent)
    : DListView(parent)
{
    setIconSize({ 16, 16 });
    setSpacing(5);
}

NotificationListView::~NotificationListView()
{
}
